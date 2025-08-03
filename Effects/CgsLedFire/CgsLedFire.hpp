#pragma once

#include "ui_CgsLedFire.h"
#include "CgsLedEffect.hpp"
#include "PerlinNoise.hpp"

namespace Ui {
    class CgsLedFire;
}

class CgsLedFire : public CgsLedEffect {
    Q_OBJECT
    CGS_LED_EFFECT(CgsLedFire, "Fire")

protected:
    virtual RGBColor draw(unsigned int x, unsigned int y, unsigned int width, unsigned int height, float t) override;

private:
    siv::BasicPerlinNoise<float> m_noise;
};
