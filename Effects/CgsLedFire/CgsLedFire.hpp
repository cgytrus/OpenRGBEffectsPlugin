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

private:
    siv::BasicPerlinNoise<float> m_noise;
};
