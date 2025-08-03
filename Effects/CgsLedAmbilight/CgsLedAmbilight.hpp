#pragma once

#include "ui_CgsLedAmbilight.h"
#include "CgsLedEffect.hpp"
#include "global_obs.hpp"

namespace Ui {
    class CgsLedAmbilight;
}

enum class AmbilightMode { Copy, Average };

class CgsLedAmbilight : public CgsLedEffect {
    Q_OBJECT
    CGS_LED_EFFECT(CgsLedAmbilight, "Ambilight")

protected:
    virtual void onShouldUpdateUi() override;

    virtual void load(json settings) override;
    virtual json save() override;

    virtual void start() override;
    virtual void stop() override;

    void draw(std::vector<ControllerZone*>) override;

private:
    AmbilightMode m_mode = AmbilightMode::Copy;
    QRectF m_crop = { 0.0f, 0.0f, 1.0f, 1.0f };

    QImage m_image;
    std::mutex m_imageLock;
};
