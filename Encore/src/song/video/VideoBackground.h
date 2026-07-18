#pragma once
#include "raylib.h"
#include "util/threadpool.h"

#include <filesystem>
#include <utility>
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

struct ManagedFrame {
    AVFrame* frame;

    ManagedFrame(AVFrame* frame) : frame(frame) {}

    Image GetRaylibImage() const {
        Image out;
        out.width = frame->width;
        out.height = frame->height;
        out.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8;
        out.mipmaps = 1;
        out.data = frame->data[0];
        return out;
    }

    ~ManagedFrame() {
        av_frame_free(&frame);
    }
};

class VideoBackground {
public:
    bool active = false;
    std::filesystem::path videoPath;
    ThreadPool workers = 1;
    std::deque<std::shared_ptr<ManagedFrame>> frameQueue;
    std::mutex frameQueueMutex;
    std::atomic<double> decodedTime = 0;
    std::atomic_uint64_t bufferCount = 0;
    Texture2D currentTexture;

    unsigned int pbo = 0;
    void* mappedUploadBuffer = nullptr;

    enum UploadState {
        IDLE,
        UPLOADING,
        DONE
    };

    std::atomic<UploadState> uploadState = IDLE;

    VideoBackground(std::filesystem::path videoPath) : videoPath(std::move(videoPath)) {
        currentTexture.id = 0;
        currentTexture.width = 0;
        currentTexture.height = 0;
        OpenFile();
    }

    // Get the texture for the current frame.
    Texture2D* GetTexture(double time);

    ~VideoBackground();
    void QueueFrameUpload(const std::shared_ptr<ManagedFrame> &frame);
    void UploadToPBO(const std::shared_ptr<ManagedFrame>& frame);

protected:
    AVFormatContext *fmtCtx = nullptr;
    const AVCodec *codec = nullptr;
    AVCodecContext* codecCtx = nullptr;

    int streamIndex = -1;

    void OpenFile();
    void ReadAndDecodeFrame();
    static double PtsToAudioTime(int64_t pts, AVRational timeBase);
};
