#pragma once

#include <QWidget>
#include "RGBEffect.h"
#include "EffectRegisterer.h"
#include "hsv.h"
#include <memory>
#include "gamma8.hpp"

static inline constexpr RGBColor rgb(float r, float g, float b) {
    return qRgb(static_cast<int>(r * 255.0f), static_cast<int>(g * 255.0f), static_cast<int>(b * 255.0f));
}

static inline RGBColor hsv(float h, float s, float v) {
    hsv_t x {
        static_cast<unsigned int>(std::fmodf(h, 360.0f)),
        static_cast<unsigned char>(s * 255.0f),
        static_cast<unsigned char>(v * 255.0f)
    };
    return hsv2rgb(&x);
}

struct music_colors_t {
    float hueSpeed = 5.0f;
    float hueOffset = 0.0f;
    float rightHueOffset = 30.0f;
    float hueRange = 120.0f;
    float saturation = 0.7f;
    bool gammaCorrection = true;

    inline RGBColor get(unsigned int i, unsigned int length, float time, float h, float v) {
        float x = static_cast<float>(i) / length;
        auto c = hsv(time * hueSpeed + hueOffset + x * rightHueOffset + h * hueRange, saturation, v);
        return gammaCorrection ? applyGamma(c) : c;
    }
};

inline void to_json(json& j, const music_colors_t& s) {
    j["hueSpeed"] = s.hueSpeed;
    j["hueOffset"] = s.hueOffset;
    j["rightHueOffset"] = s.rightHueOffset;
    j["hueRange"] = s.hueRange;
    j["saturation"] = s.saturation;
    j["gammaCorrection"] = s.gammaCorrection;
}

inline void from_json(const json& j, music_colors_t& s) {
    if (j.contains("hueSpeed")) s.hueSpeed = j["hueSpeed"];
    if (j.contains("hueOffset")) s.hueOffset = j["hueOffset"];
    if (j.contains("rightHueOffset")) s.rightHueOffset = j["rightHueOffset"];
    if (j.contains("hueRange")) s.hueRange = j["hueRange"];
    if (j.contains("saturation")) s.saturation = j["saturation"];
    if (j.contains("gammaCorrection")) s.gammaCorrection = j["gammaCorrection"];
}

#define CGS_LED_EFFECT(className, name) \
public: \
    explicit className(QWidget* parent = nullptr); \
    ~className(); \
    EFFECT_REGISTERER(#className, "CG's LED " name, "CG's LED", [](){ return new className; }); \
protected: \
    RGBColor getColor(unsigned int x, unsigned int y, unsigned int width, unsigned int height, float t) override; \
private: \
    std::unique_ptr<Ui::className> m_ui = ([&]() { \
        auto ui = std::make_unique<Ui::className>(); \
        ui->setupUi(this); \
        EffectDetails.EffectClassName = #className; \
        EffectDetails.EffectName = "CG's LED " name; \
        EffectDetails.EffectDescription = ""; \
        EffectDetails.SupportsRandom = false; \
        EffectDetails.MaxSpeed = 200; \
        EffectDetails.MinSpeed = 1; \
        return std::move(ui); \
    })();

class CgsLedEffect : public RGBEffect {
    Q_OBJECT

public:
    explicit CgsLedEffect(QWidget* parent = nullptr) : RGBEffect(parent) { };
    virtual ~CgsLedEffect() { };

    virtual void EffectState(bool state) {
        EffectEnabled = state;
        if (state) {
            this->start();
        }
        else {
            this->stop();
            m_time = 0.0f;
        }
    }

    virtual void StepEffect(std::vector<ControllerZone*> zones) {
        for (const auto& zone : zones) {
            if (zone->type() == ZONE_TYPE_SINGLE || zone->type() == ZONE_TYPE_LINEAR) {
                auto width = zone->leds_count();
                for (unsigned int x = 0; x < width; x++) {
                    zone->SetLED(x, getColor(x, 0, width, 1, m_time), Brightness, Temperature, Tint);
                }
            }
            else if (zone->type() == ZONE_TYPE_MATRIX) {
                auto width = zone->matrix_map_width();
                auto height = zone->matrix_map_height();
                for (unsigned int y = 0; y < height; y++) {
                    for (unsigned int x = 0; x < width; x++) {
                        auto i = zone->map()[(y * width) + x];
                        zone->SetLED(i, getColor(x, y, width, height, m_time), Brightness, Temperature, Tint);
                    }
                }
            }
        }
        m_time += Speed / 100.0f / FPS;
    }

    void LoadCustomSettings(json settings) { load(settings); }
    json SaveCustomSettings() { return save(); }

protected:
    float getTime() { return m_time; }

    virtual void load(json) { }
    virtual json save() { return { }; }

    virtual void start() { }
    virtual void stop() { }

    virtual RGBColor getColor(unsigned int, unsigned int, unsigned int, unsigned int, float) { return 0; }

private:
    float m_time = 0.0f;
};
