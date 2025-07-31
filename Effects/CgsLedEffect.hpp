#pragma once

#include <QWidget>
#include <QCheckBox>
#include "RGBEffect.h"
#include "EffectRegisterer.h"
#include "hsv.h"
#include <memory>
#include "gamma8.hpp"
#include "LivePreviewController.h"

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

    inline RGBColor get(unsigned int i, unsigned int length, float time, float h, float v) {
        float x = static_cast<float>(i) / length;
        return hsv(time * hueSpeed + hueOffset + x * rightHueOffset + h * hueRange, saturation, v);
    }
};

inline void to_json(json& j, const music_colors_t& s) {
    j["hueSpeed"] = s.hueSpeed;
    j["hueOffset"] = s.hueOffset;
    j["rightHueOffset"] = s.rightHueOffset;
    j["hueRange"] = s.hueRange;
    j["saturation"] = s.saturation;
}

inline void from_json(const json& j, music_colors_t& s) {
    if (j.contains("hueSpeed")) s.hueSpeed = j["hueSpeed"];
    if (j.contains("hueOffset")) s.hueOffset = j["hueOffset"];
    if (j.contains("rightHueOffset")) s.rightHueOffset = j["rightHueOffset"];
    if (j.contains("hueRange")) s.hueRange = j["hueRange"];
    if (j.contains("saturation")) s.saturation = j["saturation"];
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
        auto* gammaCorrection = new QCheckBox("Gamma Correction"); \
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed); \
        sizePolicy.setHorizontalStretch(0); \
        sizePolicy.setVerticalStretch(0); \
        sizePolicy.setHeightForWidth(gammaCorrection->sizePolicy().hasHeightForWidth()); \
        gammaCorrection->setSizePolicy(sizePolicy); \
        gammaCorrection->setCheckState(this->getGammaCorrection() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked); \
        this->connect(gammaCorrection, static_cast<void(QCheckBox::*)(int)>(&QCheckBox::stateChanged), [&](int value) { \
            this->setGammaCorrection(value == Qt::CheckState::Checked); \
        }); \
        ui->gridLayout->addWidget(gammaCorrection, 0, 0, 1, 1); \
        EffectDetails.EffectClassName = #className; \
        EffectDetails.EffectName = "CG's LED " name; \
        EffectDetails.EffectDescription = ""; \
        EffectDetails.SupportsRandom = false; \
        EffectDetails.MaxSpeed = 200; \
        EffectDetails.MinSpeed = 1; \
        EffectDetails.HasCustomSettings = true; \
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
            bool isPreview = dynamic_cast<LivePreviewController*>(zone->controller);
            if (zone->type() == ZONE_TYPE_SINGLE || zone->type() == ZONE_TYPE_LINEAR) {
                auto width = zone->leds_count();
                for (unsigned int x = 0; x < width; x++) {
                    auto color = getColor(x, 0, width, 1, m_time);
                    if (!isPreview && m_gammaCorrection)
                        color = applyGamma(color);
                    zone->SetLED(x, color, Brightness, Temperature, Tint);
                }
            }
            else if (zone->type() == ZONE_TYPE_MATRIX) {
                auto width = zone->matrix_map_width();
                auto height = zone->matrix_map_height();
                for (unsigned int y = 0; y < height; y++) {
                    for (unsigned int x = 0; x < width; x++) {
                        auto i = zone->map()[(y * width) + x];
                        auto color = getColor(x, y, width, height, m_time);
                        if (!isPreview && m_gammaCorrection)
                            color = applyGamma(color);
                        zone->SetLED(i, color, Brightness, Temperature, Tint);
                    }
                }
            }
        }
        m_time += Speed / 100.0f / FPS;
    }

    void LoadCustomSettings(json settings) {
        if (settings.contains("gammaCorrection"))
            m_gammaCorrection = settings["gammaCorrection"];
        load(settings);
    }
    json SaveCustomSettings() {
        json settings = save();
        settings["gammaCorrection"] = m_gammaCorrection;
        return settings;
    }

protected:
    float getTime() { return m_time; }

    bool getGammaCorrection() { return m_gammaCorrection; }
    void setGammaCorrection(bool x) { m_gammaCorrection = x; }

    virtual void load(json) { }
    virtual json save() { return { }; }

    virtual void start() { }
    virtual void stop() { }

    virtual RGBColor getColor(unsigned int, unsigned int, unsigned int, unsigned int, float) { return 0; }

private:
    float m_time = 0.0f;
    bool m_gammaCorrection = false;
};
