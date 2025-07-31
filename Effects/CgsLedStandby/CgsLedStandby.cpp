#include "CgsLedStandby.hpp"

REGISTER_EFFECT(CgsLedStandby);

CgsLedStandby::~CgsLedStandby() { }
CgsLedStandby::CgsLedStandby(QWidget* parent) : CgsLedEffect(parent) {
    SetSpeed(100);
}

RGBColor CgsLedStandby::getColor(unsigned int x, unsigned int, unsigned int, unsigned int, float t) {
    float offset = std::clamp(std::sinf((t * 1000.0f + x * 100.0f) / 1000.0f) * 0.15f, 0.0f, 1.0f);
    return rgb(offset, std::clamp(1.0f + offset, 0.0f, 1.0f), offset);
}
