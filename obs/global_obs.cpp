#include "global_obs.hpp"
#define NOMINMAX
#include <Windows.h>
#include <unordered_map>
#include <mutex>
#include <QApplication>

#ifdef __APPLE__
#define INPUT_AUDIO_SOURCE "coreaudio_input_capture"
#define OUTPUT_AUDIO_SOURCE "coreaudio_output_capture"
#define APPLICATION_AUDIO_SOURCE ""
#elif _WIN32
#define INPUT_AUDIO_SOURCE "wasapi_input_capture"
#define OUTPUT_AUDIO_SOURCE "wasapi_output_capture"
#define APPLICATION_AUDIO_SOURCE "wasapi_process_output_capture"
#else
#define INPUT_AUDIO_SOURCE "pulse_input_capture"
#define OUTPUT_AUDIO_SOURCE "pulse_output_capture"
#define APPLICATION_AUDIO_SOURCE ""
#endif

namespace obs {
    namespace {
        std::unordered_map<void*, std::function<void(QImage)>> s_video;
        std::unordered_map<void*, std::function<void(float)>> s_audioMono;
        std::unordered_map<void*, std::function<void(float, float)>> s_audioStereo;
        obs_video_info s_ovi;
        obs_source* s_videoSource = nullptr;
        obs_source* s_audioSource = nullptr;

        void logObsEnum(const char* name, bool(*f)(size_t, const char**)) {
            const char* temp;
            printf("%s:\n", name);
            for (size_t i = 0; f(i, &temp); i++)
                printf("  %s\n", temp);
        }

        void rawVideoCallback(void*, video_data* frame) {
            QImage img{ frame->data[0], static_cast<int>(s_ovi.output_width), static_cast<int>(s_ovi.output_height), QImage::Format::Format_ARGB32 };
            for (const auto& [_, cb] : s_video) {
                cb(img);
            }
        }

        void rawMonoAudioCallback(void*, size_t, audio_data* data) {
            float* samples = reinterpret_cast<float*>(data->data[0]);
            for (size_t i = 0; i < data->frames; i++) {
                for (const auto& [_, cb] : s_audioMono) {
                    cb(samples[i]);
                }
            }
        }

        void rawStereoAudioCallback(void*, size_t, audio_data* data) {
            float* samples[2] = {
                reinterpret_cast<float*>(data->data[0]),
                reinterpret_cast<float*>(data->data[1])
            };
            for (size_t i = 0; i < data->frames; i++) {
                for (const auto& [_, cb] : s_audioStereo) {
                    cb(samples[0][i], samples[1][i]);
                }
            }
        }

        bool resetVideo(uint32_t fps, uint32_t width, uint32_t height) {
            if (fps == 0 || width == 0 || height == 0)
                return false;

            printf("[OpenRGBEffectsPlugin] resetVideo %ix%i @ %i\n", width, height, fps);

            if (!s_video.empty())
                obs_remove_raw_video_callback(&rawVideoCallback, nullptr);
            if (s_videoSource)
                obs_set_output_source(0, nullptr);

            width += (0b100 - (width - (width & 0xfffffffc))) % 0b100;
            height += (0b10 - (height - (height & 0xfffffffe))) % 0b10;

            obs_video_info ovi;
            ovi.graphics_module = "libobs-d3d11";
            ovi.fps_num = fps;
            ovi.fps_den = 1;
            ovi.base_width = width;
            ovi.base_height = height;
            ovi.output_width = width;
            ovi.output_height = height;
            ovi.output_format = video_format::VIDEO_FORMAT_RGBA;
            ovi.adapter = 0;
            ovi.gpu_conversion = true;
            ovi.colorspace = video_colorspace::VIDEO_CS_DEFAULT;
            ovi.range = video_range_type::VIDEO_RANGE_DEFAULT;
            ovi.scale_type = obs_scale_type::OBS_SCALE_BILINEAR;
            auto res = obs_reset_video(&ovi);
            if (res != 0) {
                printf("[OpenRGBEffectsPlugin] obs_reset_video %i\n", res);
                return false;
            }

            obs_get_video_info(&ovi);
            s_ovi = ovi;

            if (s_videoSource)
                obs_set_output_source(0, s_videoSource);
            if (!s_video.empty())
                obs_add_raw_video_callback(nullptr, &rawVideoCallback, nullptr);

            return true;
        }

        void shouldUpdateSize() {
            if (!s_videoSource)
                return;
            if (s_ovi.base_width >= obs_source_get_width(s_videoSource) &&
                s_ovi.base_height >= obs_source_get_height(s_videoSource))
                return;
            resetVideo(s_ovi.fps_num, obs_source_get_width(s_videoSource), obs_source_get_height(s_videoSource));
        }

        void videoPropertiesUpdate(obs_source_t*, obs_data_t*) {
            shouldUpdateSize();
        }

        void setVideoSourceInternal(obs_source* source) {
            if (s_videoSource)
                obs_source_release(s_videoSource);
            s_videoSource = source;
            shouldUpdateSize();
            obs_set_output_source(0, s_videoSource);
        }

        void setAudioSourceInternal(obs_source* source) {
            if (s_audioSource)
                obs_source_release(s_audioSource);
            s_audioSource = source;
            obs_set_output_source(1, s_audioSource);
        }
    }

    void init(const char* locale, std::filesystem::path moduleConfigPath) {
        printf("[OpenRGBEffectsPlugin] init\n");

        if (!obs_startup(locale, moduleConfigPath.string().c_str(), nullptr)) {
            printf("[OpenRGBEffectsPlugin] !obs_startup\n");
            return;
        }

        obs_add_module_path("obs-plugins/64bit", "obs-plugins/%module%");
        obs_add_data_path("data/libobs/");

        resetVideo(60, 32, 32);

        obs_audio_info oai;
        oai.samples_per_sec = 48000;
        oai.speakers = speaker_layout::SPEAKERS_STEREO;
        if (!obs_reset_audio(&oai))
            printf("[OpenRGBEffectsPlugin] !obs_reset_audio\n");

        obs_enter_graphics();
        obs_load_all_modules();
        obs_post_load_modules();
        obs_log_loaded_modules();
        obs_leave_graphics();

        logObsEnum("obs_enum_source_types", &obs_enum_source_types);
        logObsEnum("obs_enum_input_types", &obs_enum_input_types);
        logObsEnum("obs_enum_filter_types", &obs_enum_filter_types);
    }

    void deinit() {
        printf("[OpenRGBEffectsPlugin] deinit\n");
        setVideoSourceInternal(nullptr);
        setAudioSourceInternal(nullptr);
        obs_shutdown();
    }

    void startVideo(void* self, std::function<void(QImage)> callback) {
        if (s_video.find(self) != s_video.end())
            return;

        shouldUpdateSize();

        if (s_video.empty())
            obs_add_raw_video_callback(nullptr, &rawVideoCallback, nullptr);

        s_video[self] = callback;
    }

    void stopVideo(void* self) {
        if (s_video.find(self) == s_video.end())
            return;

        s_video.erase(self);

        if (s_video.empty())
            obs_remove_raw_video_callback(&rawVideoCallback, nullptr);
    }

    void startMonoAudio(void* self, std::function<void(float)> callback) {
        if (s_audioMono.find(self) != s_audioMono.end())
            return;

        if (s_audioMono.empty()) {
            audio_convert_info aci;
            aci.samples_per_sec = 48000;
            aci.format = audio_format::AUDIO_FORMAT_FLOAT;
            aci.speakers = speaker_layout::SPEAKERS_MONO;
            aci.allow_clipping = true;
            obs_add_raw_audio_callback(0, &aci, &rawMonoAudioCallback, nullptr);
        }

        s_audioMono[self] = callback;
    }

    void stopMonoAudio(void* self) {
        if (s_audioMono.find(self) == s_audioMono.end())
            return;

        s_audioMono.erase(self);

        if (s_audioMono.empty())
            obs_remove_raw_audio_callback(0, &rawMonoAudioCallback, nullptr);
    }

    void startStereoAudio(void* self, std::function<void(float, float)> callback) {
        if (s_audioStereo.find(self) != s_audioStereo.end())
            return;

        if (s_audioStereo.empty()) {
            audio_convert_info aci;
            aci.samples_per_sec = 48000;
            aci.format = audio_format::AUDIO_FORMAT_FLOAT;
            aci.speakers = speaker_layout::SPEAKERS_STEREO;
            aci.allow_clipping = true;
            obs_add_raw_audio_callback(0, &aci, &rawMonoAudioCallback, nullptr);
        }

        s_audioStereo[self] = callback;
    }

    void stopStereoAudio(void* self) {
        if (s_audioStereo.find(self) == s_audioStereo.end())
            return;

        s_audioStereo.erase(self);

        if (s_audioStereo.empty())
            obs_remove_raw_audio_callback(0, &rawStereoAudioCallback, nullptr);
    }

    QSize getOutputSize() {
        if (!s_videoSource)
            return { 0, 0 };
        return QSize(obs_source_get_width(s_videoSource), obs_source_get_height(s_videoSource));
    }

    uint64_t getFramerate() {
        return s_ovi.fps_num;
    }

    void setFramerate(uint64_t fps) {
        resetVideo(fps, s_ovi.base_width, s_ovi.base_height);
    }

    VideoSourceType getVideoSource() {
        if (!s_videoSource)
            return VideoSourceType::Monitor;
        std::string id = obs_source_get_id(s_videoSource);
        if (id == "monitor_capture")
            return VideoSourceType::Monitor;
        if (id == "window_capture")
            return VideoSourceType::Window;
        if (id == "game_capture")
            return VideoSourceType::Game;
        return VideoSourceType::Monitor;
    }

    void setVideoSource(VideoSourceType type) {
        const char* id = "";
        switch (type) {
            case VideoSourceType::Monitor:
                id = "monitor_capture";
                break;
            case VideoSourceType::Window:
                id = "window_capture";
                break;
            case VideoSourceType::Game:
                id = "game_capture";
                break;
            default:
                return;
        }
        setVideoSourceInternal(nullptr);
        setVideoSourceInternal(obs_source_create(id, "video", nullptr, nullptr));
    }

    AudioSourceType getAudioSource() {
        if (!s_audioSource)
            return AudioSourceType::Output;
        std::string id = obs_source_get_id(s_audioSource);
        if (id == INPUT_AUDIO_SOURCE)
            return AudioSourceType::Input;
        if (id == OUTPUT_AUDIO_SOURCE)
            return AudioSourceType::Output;
        if (id == APPLICATION_AUDIO_SOURCE)
            return AudioSourceType::Application;
        return AudioSourceType::Output;
    }

    void setAudioSource(AudioSourceType type) {
        const char* id = "";
        switch (type) {
            case AudioSourceType::Input:
                id = INPUT_AUDIO_SOURCE;
                break;
            case AudioSourceType::Output:
                id = OUTPUT_AUDIO_SOURCE;
                break;
            case AudioSourceType::Application:
                id = APPLICATION_AUDIO_SOURCE;
                break;
            default:
                return;
        }
        if (strlen(id) == 0)
            return;
        setAudioSourceInternal(nullptr);
        setAudioSourceInternal(obs_source_create(id, "audio", nullptr, nullptr));
    }

    OBSPropertiesView* createVideoPropertiesView() {
        if (!s_videoSource)
            return nullptr;
        return new OBSPropertiesView(
            obs_source_get_settings(s_videoSource),
            s_videoSource,
            reinterpret_cast<PropertiesReloadCallback>(&obs_source_properties),
            reinterpret_cast<PropertiesUpdateCallback>(&videoPropertiesUpdate),
            reinterpret_cast<PropertiesVisualUpdateCb>(&obs_source_update)
        );
    }

    OBSPropertiesView* createAudioPropertiesView() {
        if (!s_audioSource)
            return nullptr;
        return new OBSPropertiesView(
            obs_source_get_settings(s_audioSource),
            s_audioSource,
            reinterpret_cast<PropertiesReloadCallback>(&obs_source_properties),
            nullptr,
            reinterpret_cast<PropertiesVisualUpdateCb>(&obs_source_update)
        );
    }

    std::string saveVideoSource() {
        if (!s_videoSource)
            return "";
        OBSDataAutoRelease data = obs_save_source(s_videoSource);
        return obs_data_get_json(data);
    }

    void loadVideoSource(std::string str) {
        if (str == "")
            return;
        OBSDataAutoRelease data = obs_data_create_from_json(str.c_str());
        setVideoSourceInternal(nullptr);
        setVideoSourceInternal(obs_load_source(data));
    }

    std::string saveAudioSource() {
        if (!s_audioSource)
            return "";
        OBSDataAutoRelease data = obs_save_source(s_audioSource);
        return obs_data_get_json(data);
    }

    void loadAudioSource(std::string str) {
        if (str == "")
            return;
        OBSDataAutoRelease data = obs_data_create_from_json(str.c_str());
        setAudioSourceInternal(nullptr);
        setAudioSourceInternal(obs_load_source(data));
    }
}
