#include "CgsLedFire.hpp"

REGISTER_EFFECT(CgsLedFire);

CgsLedFire::~CgsLedFire() { }
CgsLedFire::CgsLedFire(QWidget* parent) : CgsLedEffect(parent) {
    EffectDetails.HasCustomSettings = false;
    SetSpeed(80);
}

RGBColor CgsLedFire::getColor(unsigned int x, unsigned int y, unsigned int, unsigned int, float t) {
    float valueNoise = m_noise.noise3D_01(x * 0.25f, y * 0.25f, t);
    float hueNoise = m_noise.noise3D_01(x * 0.1f, t * 0.5f, y * 0.1f);
    return hsv(hueNoise * 60.0f, 1.0f, valueNoise);
}
