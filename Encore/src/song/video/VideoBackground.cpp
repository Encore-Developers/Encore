#include "VideoBackground.h"

#include "tracy/Tracy.hpp"
#include "util/enclog.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/frame.h>
#include <libswscale/swscale.h>
}

void VideoBackground::OpenFile() {
    ZoneScoped;
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
    ReadAndDecodeFrame();
}

Texture2D *VideoBackground::GetTexture(double time) {
    if (!active) {
        return nullptr;
    }
    if (time > decodedTime - 3 && bufferCount < 10 - frameQueue.size()) {
        workers.SubmitTask([this]{ReadAndDecodeFrame();});
        bufferCount += 1;
    }
    if (!frameQueueMutex.try_lock()) {
        if (currentTexture.id == 0) {
            return nullptr;
        } else {
            return &currentTexture;
        }
    }
    if (frameQueue.empty()) {
        frameQueueMutex.unlock();
        if (currentTexture.id == 0) {
            return nullptr;
        } else {
            return &currentTexture;
        }
    }
    auto nextFrame = frameQueue.front();
    if (currentTexture.id == 0 || (time >= PtsToAudioTime(nextFrame->frame->pts, nextFrame->frame->time_base) && (nextFrame->frame->width != currentTexture.width || nextFrame->frame->height != currentTexture.height))) {
        if (currentTexture.id != 0) {
            UnloadTexture(currentTexture);
        }
        currentTexture = LoadTextureFromImage(nextFrame->GetRaylibImage());
        frameQueue.pop_front();
    } else if (time >= PtsToAudioTime(nextFrame->frame->pts, nextFrame->frame->time_base)) {
        UpdateTexture(currentTexture, nextFrame->frame->data[0]);
        frameQueue.pop_front();
    }
    frameQueueMutex.unlock();
    return &currentTexture;
}
VideoBackground::~VideoBackground() {
    if (fmtCtx) {
        avformat_close_input(&fmtCtx);
    }
    if (codecCtx) {
        avcodec_free_context(&codecCtx);
    }
}


void VideoBackground::ReadAndDecodeFrame() {
    ZoneScoped
    if (fmtCtx && codecCtx) {
        ManagedFrame managedFrame = av_frame_alloc();
        auto frame = managedFrame.frame;

        int recvResult = -1;
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
            recvResult = avcodec_receive_frame_flags(codecCtx, frame, AV_CODEC_RECEIVE_FRAME_FLAG_SYNCHRONOUS);
            if (recvResult < 0) {
                Encore::Log::Error("Failed to receive frame: {}", recvResult);
            }
        }
        SwsContext* swsCtx = sws_getContext(frame->width, frame->height, codecCtx->pix_fmt, frame->width, frame->height, AV_PIX_FMT_RGB24, 0, nullptr, nullptr, nullptr);
        AVFrame* outFrame = av_frame_alloc();
        auto scaleResult = sws_scale_frame(swsCtx, outFrame, frame);
        outFrame->pts = frame->pts;
        outFrame->time_base = fmtCtx->streams[streamIndex]->time_base;
        decodedTime = PtsToAudioTime(frame->pts, frame->time_base);
        frameQueueMutex.lock();
        frameQueue.push_back(std::make_shared<ManagedFrame>(outFrame));
        frameQueueMutex.unlock();
        active = true;

        bufferCount -= 1;
        sws_free_context(&swsCtx);
    }
}
double VideoBackground::PtsToAudioTime(int64_t pts, AVRational timeBase) {
    double timeBaseFloat = timeBase.num / (double)timeBase.den;
    return pts * timeBaseFloat;
}