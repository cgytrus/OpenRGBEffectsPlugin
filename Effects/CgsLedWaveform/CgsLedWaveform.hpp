#pragma once

#include "ui_CgsLedWaveform.h"
#include "CgsLedEffect.hpp"

namespace Ui {
    class CgsLedWaveform;
}

enum class HueOffsetMode {
    Signed,
    Absolute,
    Centered
};

class CgsLedWaveform : public CgsLedEffect {
    Q_OBJECT
    CGS_LED_EFFECT(CgsLedWaveform, "Waveform")

public:
    virtual void StepEffect(std::vector<ControllerZone*> zones) override;

protected:
    virtual void load(json settings) override;
    virtual json save() override;

    virtual void start() override;
    virtual void stop() override;

private:
    music_colors_t m_colors = {};
    HueOffsetMode m_hueOffsetMode = HueOffsetMode::Signed;
    float m_displaySeconds = 0.15f;

    size_t getDisplayCount() { return 48000 * m_displaySeconds; }

    std::vector<float> m_samples;

    size_t m_bufferHead = 0;
    size_t m_displayTail = 0;

    std::mutex m_samplesLock;
};
