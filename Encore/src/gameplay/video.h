#ifndef VIDEO_STREAM_PLAYER_H
#define VIDEO_STREAM_PLAYER_H

#include "raylib.h"
#include <filesystem>
#include <string>
#include <cmath>
#include <algorithm>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <queue>
#include <condition_variable>

#include "settings.h"

// Check if FFmpeg is available
#ifdef __has_include
  #if __has_include(<libavcodec/avcodec.h>)
    #define FFMPEG_AVAILABLE 1
  #else
    #define FFMPEG_AVAILABLE 0
  #endif
#else
  #define FFMPEG_AVAILABLE 1  // Assume available if __has_include not supported
#endif

#if FFMPEG_AVAILABLE
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}
#endif

#if FFMPEG_AVAILABLE
class VideoStream {
public:
    VideoStream() = default;
    ~VideoStream() {
        Unload();
    }

    bool Load(const std::filesystem::path& videoPath) {
        if (isLoaded) {
            TraceLog(LOG_WARNING, "VIDEO: A video is already loaded. Call Unload() first.");
            return true;
        }

        const std::string pathStr = videoPath.string();

        if (!std::filesystem::exists(videoPath)) {
            TraceLog(LOG_INFO, "VIDEO: No video file found at: %s", pathStr.c_str());
            return false;
        }

        if (avformat_open_input(&pFormatCtx, pathStr.c_str(), NULL, NULL) != 0) {
            TraceLog(LOG_ERROR, "FFMPEG: Couldn't open video file: %s", pathStr.c_str());
            return false;
        }

        if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
            TraceLog(LOG_ERROR, "FFMPEG: Couldn't find stream information.");
            avformat_close_input(&pFormatCtx);
            return false;
        }

        videoStreamIndex = -1;
        for (unsigned int i = 0; i < pFormatCtx->nb_streams; i++) {
            if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                videoStreamIndex = i;
                break;
            }
        }
        if (videoStreamIndex == -1) {
            TraceLog(LOG_ERROR, "FFMPEG: Didn't find a video stream.");
            avformat_close_input(&pFormatCtx);
            return false;
        }

        AVCodecParameters* pCodecPar = pFormatCtx->streams[videoStreamIndex]->codecpar;
        const AVCodec* pCodec = avcodec_find_decoder(pCodecPar->codec_id);
        if (pCodec == NULL) {
            TraceLog(LOG_ERROR, "FFMPEG: Unsupported codec!");
            avformat_close_input(&pFormatCtx);
            return false;
        }

        pCodecCtx = avcodec_alloc_context3(pCodec);
        if (avcodec_parameters_to_context(pCodecCtx, pCodecPar) < 0) {
            TraceLog(LOG_ERROR, "FFMPEG: Couldn't copy codec context.");
            avcodec_free_context(&pCodecCtx);
            avformat_close_input(&pFormatCtx);
            return false;
        }

        if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
            TraceLog(LOG_ERROR, "FFMPEG: Could not open codec.");
            avcodec_free_context(&pCodecCtx);
            avformat_close_input(&pFormatCtx);
            return false;
        }

        width = pCodecCtx->width;
        height = pCodecCtx->height;
        AVRational time_base = pFormatCtx->streams[videoStreamIndex]->avg_frame_rate;
        fps = (time_base.num > 0 && time_base.den > 0) ? static_cast<double>(time_base.num) / time_base.den : 30.0;

        display_width = width;
        display_height = height;
        if (height > 500) {
            float aspect = (float)width / (float)height;
            display_height = 500;
            display_width = (int)(display_height * aspect);
        }

        sws_ctx = sws_getContext(width, height, pCodecCtx->pix_fmt, display_width, display_height, AV_PIX_FMT_RGBA, SWS_BILINEAR, NULL, NULL, NULL);
        if (sws_ctx == NULL) {
            TraceLog(LOG_ERROR, "FFMPEG: Could not initialize the conversion context.");
            Unload();
            return false;
        }

        Image blankImage = GenImageColor(display_width, display_height, BLANK);
        displayTexture = LoadTextureFromImage(blankImage);
        UnloadImage(blankImage);
        SetTextureFilter(displayTexture, TEXTURE_FILTER_BILINEAR);

        isLoaded = true;
        stopDecoder = false;
        decoderThread = std::thread(&VideoStream::DecodeLoop, this);

        TraceLog(LOG_INFO, "VIDEO: Stream loaded successfully (%dx%d @ %f fps), downscaled to %dx%d", width, height, fps, display_width, display_height);
        return true;
    }

    void Unload() {
        if (!isLoaded) return;

        isPlaying = false;
        stopDecoder = true;
        frameQueueCond.notify_all();
        if (decoderThread.joinable()) {
            decoderThread.join();
        }

        if(sws_ctx) sws_freeContext(sws_ctx);
        if(pCodecCtx) avcodec_free_context(&pCodecCtx);
        if(pFormatCtx) avformat_close_input(&pFormatCtx);

        std::lock_guard<std::mutex> lock(queueMutex);
        while(!frameQueue.empty()){
            av_frame_free(&frameQueue.front());
            frameQueue.pop();
        }

        if (displayTexture.id > 0) UnloadTexture(displayTexture);

        pFormatCtx = nullptr; pCodecCtx = nullptr; sws_ctx = nullptr;
        displayTexture = { 0 };
        isLoaded = false;
    }

    void Update() {
        if (!isLoaded) return;
        
        if (delayedStart) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - playStartTime).count();
            if (elapsed >= startDelayMs) {
                isPlaying = true;
                delayedStart = false;
            } else {
                return;
            }
        }
        
        if (!isPlaying) return;

        frameTimer += GetFrameTime();
        double frameDuration = (fps > 0) ? (1.0 / fps) : 0.033;

        if (frameTimer >= frameDuration) {
            frameTimer -= frameDuration;

            std::unique_lock<std::mutex> lock(queueMutex);
            if (!frameQueue.empty()) {
                AVFrame* frame = frameQueue.front();
                frameQueue.pop();
                lock.unlock();
                frameQueueCond.notify_one();

                uint8_t* frameBuffer[4] = { nullptr };
                int linesize[4] = { 0 };
                av_image_alloc(frameBuffer, linesize, display_width, display_height, AV_PIX_FMT_RGBA, 1);
                sws_scale(sws_ctx, (uint8_t const* const*)frame->data, frame->linesize, 0, height, frameBuffer, linesize);
                UpdateTexture(displayTexture, frameBuffer[0]);
                av_freep(&frameBuffer[0]);
                av_frame_free(&frame);
            }
        }
    }

    void Draw(int posX = 0, int posY = 0, Color tint = WHITE) {
        if (isLoaded && displayTexture.id > 0) {
            float screenWidth = (float)GetScreenWidth();
            float screenHeight = (float)GetScreenHeight();
            float screenAspect = screenWidth / screenHeight;
            float videoAspect = (float)display_width / (float)display_height;

            Rectangle sourceRec = { 0.0f, 0.0f, (float)display_width, (float)display_height };
            Rectangle destRec;

            if (screenAspect > videoAspect) {
                float scale = screenWidth / (float)display_width;
                float scaledHeight = (float)display_height * scale;
                destRec = { 0, (screenHeight - scaledHeight) / 2.0f, screenWidth, scaledHeight };
            } else {
                float scale = screenHeight / (float)display_height;
                float scaledWidth = (float)display_width * scale;
                destRec = { (screenWidth - scaledWidth) / 2.0f, 0, scaledWidth, screenHeight };
            }

            DrawTexturePro(displayTexture, sourceRec, destRec, {0, 0}, 0.0f, tint);
        }
    }

    void Play() { 
        if (isLoaded) {
            isPlaying = true;
            startDelayMs = 0.0;
            playStartTime = std::chrono::steady_clock::now();
        }
    }
    void PlayWithDelay(double delayMs) {
        if (isLoaded) {
            startDelayMs = delayMs;
            playStartTime = std::chrono::steady_clock::now();
            isPlaying = false;
            delayedStart = true;
        }
    }
    void Pause() { 
        if (isLoaded) {
            isPlaying = false;
            delayedStart = false;
        }
    }
    void Resume() { 
        if (isLoaded) {
            if (startDelayMs > 0.0) {
                PlayWithDelay(startDelayMs);
            } else {
                Play();
            }
        }
    }
    void Stop() {
        if (isLoaded) {
            isPlaying = false;
            delayedStart = false;
            std::lock_guard<std::mutex> lock(seekMutex);
            seekRequest = true;
            seekToTime = 0.0;
            frameQueueCond.notify_all();
        }
    }

    bool IsLoaded() const { return isLoaded; }

private:
    void DecodeLoop() {
        AVPacket* packet = av_packet_alloc();
        const size_t maxQueueSize = 30;

        while (!stopDecoder) {
            {
                std::lock_guard<std::mutex> lock(seekMutex);
                if (seekRequest) {
                    // Convert time to timestamp
                    int64_t timestamp = (int64_t)(seekToTime * AV_TIME_BASE);
                    if (pFormatCtx->streams[videoStreamIndex]->time_base.den != 0) {
                        timestamp = av_rescale_q(timestamp, AV_TIME_BASE_Q, pFormatCtx->streams[videoStreamIndex]->time_base);
                    }
                    
                    av_seek_frame(pFormatCtx, videoStreamIndex, timestamp, AVSEEK_FLAG_BACKWARD);
                    avcodec_flush_buffers(pCodecCtx);

                    std::lock_guard<std::mutex> queueLock(queueMutex);
                    while(!frameQueue.empty()){
                        av_frame_free(&frameQueue.front());
                        frameQueue.pop();
                    }
                    seekRequest = false;
                    frameQueueCond.notify_all();
                }
            }

            if (!isPlaying) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            {
                std::unique_lock<std::mutex> lock(queueMutex);
                frameQueueCond.wait(lock, [this, maxQueueSize] { return frameQueue.size() < maxQueueSize || stopDecoder; });
            }
            if (stopDecoder) break;

            if (av_read_frame(pFormatCtx, packet) >= 0) {
                if (packet->stream_index == videoStreamIndex) {
                    if (avcodec_send_packet(pCodecCtx, packet) == 0) {
                        while (true) {
                            AVFrame* decoded_frame = av_frame_alloc();
                            int ret = avcodec_receive_frame(pCodecCtx, decoded_frame);
                            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                                av_frame_free(&decoded_frame);
                                break;
                            } else if (ret < 0) {
                                TraceLog(LOG_ERROR, "FFMPEG: Error during decoding.");
                                av_frame_free(&decoded_frame);
                                break;
                            }

                            std::lock_guard<std::mutex> lock(queueMutex);
                            frameQueue.push(decoded_frame);
                        }
                    }
                }
                av_packet_unref(packet);
            } else {
                std::lock_guard<std::mutex> lock(seekMutex);
                seekRequest = true;
            }
        }
        av_packet_free(&packet);
    }

    AVFormatContext *pFormatCtx = nullptr;
    AVCodecContext  *pCodecCtx = nullptr;
    SwsContext      *sws_ctx = nullptr;
    int             videoStreamIndex = -1;

    Texture2D displayTexture = { 0 };

    std::thread decoderThread;
    std::queue<AVFrame*> frameQueue;
    std::mutex queueMutex;
    std::condition_variable frameQueueCond;
    std::mutex seekMutex;
    std::atomic<bool> seekRequest{false};
    std::atomic<bool> stopDecoder{false};
    double seekToTime = 0.0;
    double startDelayMs = 0.0;
    std::chrono::steady_clock::time_point playStartTime;
    std::atomic<bool> delayedStart{false};

    int width = 0, height = 0;
    int display_width = 0, display_height = 0;
    double fps = 30.0;
    double frameTimer = 0.0;
    std::atomic<bool> isLoaded{false};
    std::atomic<bool> isPlaying{false};
};

#else
class VideoStream {
public:
    VideoStream() = default;
    ~VideoStream() = default;
    
    bool Load(const std::filesystem::path& videoPath) { 
        TraceLog(LOG_INFO, "VIDEO: FFmpeg not available, video playback disabled");
        return false; 
    }
    void Unload() {}
    void Update() {}
    void Draw(int posX = 0, int posY = 0, Color tint = WHITE) {}
    void Play() {}
    void PlayWithDelay(double delayMs) {}
    void Pause() {}
    void Resume() {}
    void Stop() {}
    bool IsLoaded() const { return false; }
};
#endif

#endif // VIDEO_H