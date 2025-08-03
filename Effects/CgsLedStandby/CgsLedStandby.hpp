#pragma once

#include "ui_CgsLedStandby.h"
#include "CgsLedEffect.hpp"

namespace Ui {
    class CgsLedStandby;
}

class CgsLedStandby : public CgsLedEffect {
    Q_OBJECT
    CGS_LED_EFFECT(CgsLedStandby, "Standby")

protected:
    virtual RGBColor draw(unsigned int x, unsigned int y, unsigned int width, unsigned int height, float t) override;
};
