#pragma once

#include "obs.hpp"
#include <functional>
#include <QImage>
#include "properties-view.hpp"
#include <filesystem>

namespace obs {
    enum class VideoSourceType {
        None,
        Monitor,
        Window,
        Game
    };

    enum class AudioSourceType {
        None,
        Input,
        Output,
        Application
    };

    void init(const char* locale, std::filesystem::path moduleConfigPath);
    void deinit();

    void startVideo(void* self, std::function<void(QImage)> callback);
    void stopVideo(void* self);

    void startMonoAudio(void* self, std::function<void(float)> callback);
    void stopMonoAudio(void* self);

    void startStereoAudio(void* self, std::function<void(float, float)> callback);
    void stopStereoAudio(void* self);

    QSize getOutputSize();

    uint64_t getFramerate();
    void setFramerate(uint64_t fps);

    VideoSourceType getVideoSource();
    void setVideoSource(VideoSourceType type);

    AudioSourceType getAudioSource();
    void setAudioSource(AudioSourceType type);

    OBSPropertiesView* createVideoPropertiesView();
    OBSPropertiesView* createAudioPropertiesView();

    std::string saveVideoSource();
    void loadVideoSource(std::string str);

    std::string saveAudioSource();
    void loadAudioSource(std::string str);
};
