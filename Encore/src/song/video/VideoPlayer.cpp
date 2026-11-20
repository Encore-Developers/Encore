#include "VideoPlayer.h"

#include "external/glad.h"
#include "tracy/Tracy.hpp"
#include "tracy/TracyOpenGL.hpp"
#include "util/enclog.h"
#include <cassert>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/frame.h>
#include <libswscale/swscale.h>
}

std::deque<std::function<void()>> VideoPlayer::cleanupTasks;
std::mutex VideoPlayer::cleanupTasksMutex;

void VideoPlayer::OpenFile() {
    ZoneScoped;
#ifndef NO_VIDEO
    auto result = avformat_open_input(&fmtCtx, reinterpret_cast<const char *>(videoPath.generic_u8string().c_str()), nullptr, nullptr);
    if (result != 0) {
        Encore::Log::Error("Failed to open video file: {}", result);
        return;
    }
    auto streamnum = av_find_best_stream(fmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &codec, 0);
    if (streamnum < 0) {
        Encore::Log::Error("Failed to open decoder for video file: {}", streamnum);
        return;
    }
    codecCtx = avcodec_alloc_context3(codec);
    if (!codecCtx) {
        Encore::Log::Error("Failed to allocate video codec context");
        return;
    }
    if (avcodec_parameters_to_context(codecCtx, fmtCtx->streams[streamnum]->codecpar) < 0) {
        Encore::Log::Error("Failed to copy codec parameters");
        return;
    }
    codecCtx->pix_fmt = AV_PIX_FMT_RGB8;

    if (avcodec_open2(codecCtx, codec, nullptr) < 0) {
        Encore::Log::Error("Failed to start codec");
        return;
    }
    streamIndex = streamnum;
    bufferCount += 1;
    if (initialSeekTime > 0) {
        av_seek_frame(fmtCtx, streamIndex, initialSeekTime / ((double)fmtCtx->streams[streamIndex]->time_base.num/(double)fmtCtx->streams[streamIndex]->time_base.den), 0);
    }


    ReadAndDecodeFrame();
#endif
}

AsyncTexture::AsyncTexture(int width, int height) : textureId(0), width(width), height(height) {
    glGenTextures(1, &textureId);
    static size_t number = 0;
    glObjectLabel(GL_TEXTURE, textureId, -1, std::format("AsyncTexture #{}", number).c_str());
    glGenBuffers(1, &pbo);
    glObjectLabel(GL_BUFFER, pbo, -1, std::format("AsyncTexture PBO #{}", number).c_str());
    number++;
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
    //glBufferData(GL_PIXEL_UNPACK_BUFFER, width*height*3, nullptr, GL_STREAM_DRAW);
    glBufferStorage(GL_PIXEL_UNPACK_BUFFER, width*height*3, nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    auto error = glGetError();
    mappedUploadBuffer = glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, width*height*3, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    error = glGetError();
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}
AsyncTexture::~AsyncTexture() {
    auto textureToClean = textureId;
    auto pboToClean = pbo;
    VideoPlayer::AddCleanupTask([textureToClean, pboToClean]() {
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboToClean);
        glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

        glDeleteTextures(1, &textureToClean);
    });
}
void AsyncTexture::UploadToPBO(const std::shared_ptr<ManagedFrame> &frame) {
    // set state on the main thread first
    ZoneScoped
    assert(uploadState == UPLOADING);
    memcpy(mappedUploadBuffer, frame->frame->data[0], frame->frame->width*frame->frame->height*3);
    uploadState = PREFINALIZED;
}
Texture2D AsyncTexture::GetRaylibTexture() {
    if (uploadState != DONE) {
        return {0, 0, 0, 0, 0};
    }
    return {textureId, (int)width, (int)height, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8};
}
void AsyncTexture::Poll() {
    ZoneScoped
    if (uploadState == PREFINALIZED) {
        TracyGpuZone("Texture Update")
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
        glBindTexture(GL_TEXTURE_2D, textureId);
        if (textureInitialized) {
            ZoneScopedN("SubImage")
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        } else {
            ZoneScopedN("MainImage")
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
            auto error = glGetError();
            textureInitialized = true;
        }
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glBindBuffer(GL_TEXTURE_2D, 0);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        {
            ZoneScopedN("Atomic Write")
            uploadState = DONE;
        }
    }
}
std::shared_ptr<AsyncTexture> VideoPlayer::GetTexture(double time) {
    ZoneScoped
    windowHeight = GetRenderHeight();
#ifndef NO_VIDEO
    if (!active) {
        return nullptr;
    }
    if (time > decodedTime - 3 && bufferCount < 10 - frameQueue.size()) {
        auto ptr = selfPtr.lock();
        decodeWorker.SubmitTask([ptr]{ptr->ReadAndDecodeFrame();});
        bufferCount += 1;
    }
    if (!frameQueueMutex.try_lock()) {
        return currentTexture;
    }
    if (frameQueue.empty()) {
        frameQueueMutex.unlock();
        return currentTexture;
    }
    auto nextFrame = frameQueue.front();
    //
    if (textures.empty() || textures[0]->width != nextFrame->frame->width || textures[0]->height != nextFrame->frame->height) {
        textures.clear();
        for (int i = 0; i < 3; i++) {
            textures.push_back(std::make_shared<AsyncTexture>(nextFrame->frame->width, nextFrame->frame->height));
        }
        nextTextureIndex = 0;
    }
    if (!nextFrame->texture) {
        nextFrame->texture = textures[nextTextureIndex];
        nextTextureIndex = (nextTextureIndex + 1) % textures.size();
    }
    if (nextFrame->texture->currentFrame != nextFrame->frameIndex && nextFrame->texture->uploadState != AsyncTexture::UPLOADING && nextFrame->texture->uploadState != AsyncTexture::PREFINALIZED) {
        nextFrame->texture->currentFrame = nextFrame->frameIndex;
        QueueFrameUpload(nextFrame, nextFrame->texture);
    }
    nextFrame->texture->Poll();
    if (time >= PtsToAudioTime(nextFrame->frame->pts, nextFrame->frame->time_base) || nextFrame->frameIndex <= 0) {
        if (nextFrame->texture->uploadState == AsyncTexture::DONE) {
            currentTexture = nextFrame->texture;
            frameQueue.pop_front();
        }
    }
    frameQueueMutex.unlock();
    return currentTexture;
#else
    return nullptr;
#endif
}
VideoPlayer::~VideoPlayer() {
#ifndef NO_VIDEO
    if (fmtCtx) {
        avformat_close_input(&fmtCtx);
    }
    if (codecCtx) {
        avcodec_free_context(&codecCtx);
    }
#endif
}

void VideoPlayer::QueueFrameUpload(const std::shared_ptr<ManagedFrame>& frame, const std::shared_ptr<AsyncTexture>& texture) {
    ZoneScoped
    assert(texture->uploadState == AsyncTexture::IDLE || texture->uploadState == AsyncTexture::DONE);
    texture->uploadState = AsyncTexture::UPLOADING;
    auto ptr = selfPtr.lock();
    uploadWorkers.SubmitTask([ptr, frame, texture](){texture->UploadToPBO(frame);});
}

void VideoPlayer::ReadAndDecodeFrame() {
    ZoneScoped
#ifndef NO_VIDEO
    if (fmtCtx && codecCtx) {
        auto managedFrame = std::make_shared<ManagedFrame>(av_frame_alloc());
        auto frame = managedFrame->frame;

        int recvResult = -1;
        {
            ZoneScopedN("Send Frame to Decoder")
            while (recvResult < 0) {
                AVPacket* packet = av_packet_alloc();
                auto result = av_read_frame(fmtCtx, packet);
                if (result != 0) {
                    Encore::Log::Error("Failed to read packet: {}", result);
                    return;
                }
                if (packet->stream_index != streamIndex) {
                    av_packet_free(&packet);
                    continue;
                }

                // TODO EOF handling
                auto sendResult = avcodec_send_packet(codecCtx, packet);
                if (sendResult < 0) {
                    Encore::Log::Error("Failed to send packet: {}", sendResult);
                    av_packet_free(&packet);
                    return;
                }
                av_packet_free(&packet);
                {
                    ZoneScopedN("Receive Frame")
                    recvResult = avcodec_receive_frame_flags(codecCtx, frame, AV_CODEC_RECEIVE_FRAME_FLAG_SYNCHRONOUS);
                    if (recvResult < 0) {
                        Encore::Log::Error("Failed to receive frame: {}", recvResult);
                    }
                }

            }
        }
        {
            auto ptr = selfPtr.lock();
            uploadWorkers.SubmitTask([this, ptr, managedFrame]() {
                if (ptr) {
                    ptr->ScaleAndSubmitFrame(managedFrame);
                } else {
                    ScaleAndSubmitFrame(managedFrame);
                }
            });
        }

    }
#endif
}
void VideoPlayer::ScaleAndSubmitFrame(const std::shared_ptr<ManagedFrame> &managedFrame) {
    ZoneScopedN("Color conversion + scaling")
    auto frame = managedFrame->frame;
    auto targetHeight = windowHeight.load();
    auto destW = frame->width;
    auto destH = frame->height;
    if (targetHeight != -1) {
        if (frame->height > targetHeight) {
            auto scaleMult = targetHeight / (double)frame->height;
            destW *= scaleMult;
            destH *= scaleMult;
        }
    }
    SwsContext* swsCtx = sws_getContext(frame->width, frame->height, codecCtx->pix_fmt, destW, destH, AV_PIX_FMT_RGB24, 0, nullptr, nullptr, nullptr);
    AVFrame* outFrame = av_frame_alloc();
    outFrame->format = AV_PIX_FMT_RGB24;
    outFrame->width = destW;
    outFrame->height = destH;

    int size = destW * destH * 3;
    auto buf = (uint8_t*)av_malloc(size);
    outFrame->buf[0] = av_buffer_create(buf, size, av_buffer_default_free, NULL, 0);
    outFrame->data[0] = buf;
    outFrame->linesize[0] = destW * 3;

    auto scaleResult = sws_scale_frame(swsCtx, outFrame, frame);
    outFrame->pts = frame->pts;
    outFrame->time_base = fmtCtx->streams[streamIndex]->time_base;
    decodedTime = PtsToAudioTime(frame->pts, frame->time_base);
    {
        ZoneScopedN("Enqueue frame")
        frameQueueMutex.lock();
        frameQueue.push_back(std::make_shared<ManagedFrame>(outFrame, frameCount++));
        frameQueueMutex.unlock();
    }
    active = true;

    bufferCount -= 1;
    sws_free_context(&swsCtx);
}
double VideoPlayer::PtsToAudioTime(int64_t pts, AVRational timeBase) {
    double timeBaseFloat = timeBase.num / (double)timeBase.den;
    return pts * timeBaseFloat;
}
