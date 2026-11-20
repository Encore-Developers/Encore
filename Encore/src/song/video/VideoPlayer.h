#pragma once
#include "raylib.h"
#include "util/threadpool.h"

#include <filesystem>
#include <utility>
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

struct AsyncTexture;

struct ManagedFrame {
    AVFrame* frame;
    std::shared_ptr<AsyncTexture> texture;
    uint64_t frameIndex;

    ManagedFrame(AVFrame* frame, uint64_t index = 0) : frame(frame), frameIndex(index) {}

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

struct AsyncTexture {
    unsigned int textureId;
    unsigned int width;
    unsigned int height;
    uint64_t currentFrame = UINT64_MAX;
    bool textureInitialized = false;
    enum UploadState {
        IDLE,
        UPLOADING,
        PREFINALIZED,
        DONE
    };

    std::atomic<UploadState> uploadState = IDLE;
    unsigned int pbo = 0;
    void* mappedUploadBuffer = nullptr;

    // Only call on main thread!
    AsyncTexture(int width, int height);
    void Poll();

    ~AsyncTexture();
    void UploadToPBO(const std::shared_ptr<ManagedFrame>& frame);

    Texture2D GetRaylibTexture();
};

class VideoPlayer {
public:
    bool active = false;
    std::filesystem::path videoPath;
    ThreadPool decodeWorker = 1;
    ThreadPool uploadWorkers = 4;
    std::deque<std::shared_ptr<ManagedFrame>> frameQueue;
    std::mutex frameQueueMutex;
    std::atomic<double> decodedTime = 0;
    std::atomic_uint64_t bufferCount = 0;
    std::shared_ptr<AsyncTexture> currentTexture;
    std::atomic_uint64_t frameCount = 0;
    double initialSeekTime = 0;
    // Upgraded to a full pointer when other threads are doing other thread things
    std::weak_ptr<VideoPlayer> selfPtr;

    std::vector<std::shared_ptr<AsyncTexture>> textures;
    size_t nextTextureIndex = 0;

    std::atomic_int32_t windowHeight = -1;


    VideoPlayer(std::filesystem::path videoPath, double initialSeekTime) : videoPath(std::move(videoPath)), initialSeekTime(initialSeekTime) {
        decodeWorker.Detach();
        uploadWorkers.Detach();
        OpenFile();
    }

    /// Because OpenGL is a dumbfuck that hates being threaded we have to always clean up its resources on the main thread.
    static std::deque<std::function<void()>> cleanupTasks;
    static std::mutex cleanupTasksMutex;

    static void AddCleanupTask(const std::function<void()>& task) {
        std::lock_guard lock(cleanupTasksMutex);
        cleanupTasks.push_back(task);
    }

    /// Only call on the main thread!!
    static void PollCleanup() {
        std::lock_guard lock(cleanupTasksMutex);
        while (!cleanupTasks.empty()) {
            cleanupTasks.back()();
            cleanupTasks.pop_back();
        }
    }

    // Get the texture for the current frame.
    std::shared_ptr<AsyncTexture> GetTexture(double time);

    ~VideoPlayer();
    void QueueFrameUpload(const std::shared_ptr<ManagedFrame> &frame, const std::shared_ptr<AsyncTexture>& texture);

protected:
    AVFormatContext *fmtCtx = nullptr;
    const AVCodec *codec = nullptr;
    AVCodecContext* codecCtx = nullptr;

    int streamIndex = -1;

    void OpenFile();
    void ReadAndDecodeFrame();
    void ScaleAndSubmitFrame(const std::shared_ptr<ManagedFrame>& managedFrame);
    static double PtsToAudioTime(int64_t pts, AVRational timeBase);
};
