#pragma once

#include "ui_CgsLedWaveform.h"
#include "CgsLedEffect.hpp"

namespace Ui {
    class CgsLedWaveform;
}

struct sample_t {
    float sum;
    int count;
    float time;

    float getValue() { return sum / count; }
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
    float m_bufferSeconds = 1.0f;
    float m_displaySeconds = 0.15f;
    int m_avgCount = 87;

    size_t getBufferSize() { return 48000 * m_bufferSeconds; }
    size_t getDisplayCount() { return 48000 * m_displaySeconds / m_avgCount; }

    std::vector<sample_t> m_samples;
    uint64_t m_totalSamples;

    size_t m_displayTail;
    size_t m_showDisplayTail;

    std::mutex m_samplesLock;
};
