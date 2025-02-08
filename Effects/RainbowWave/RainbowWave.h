#ifndef RAINBOWWAVE_H
#define RAINBOWWAVE_H

#include "RGBEffect.h"
#include "EffectRegisterer.h"

class RainbowWave: public RGBEffect
{
    Q_OBJECT

public:
    RainbowWave();
    ~RainbowWave() {};

    EFFECT_REGISTERER(ClassName(), UI_Name(), CAT_RAINBOW, [](){return new RainbowWave;});

    static std::string const ClassName() { return "RainbowWave"; }
    static std::string const UI_Name() { return QT_TR_NOOP("Rainbow Wave"); }

    void StepEffect(std::vector<ControllerZone*>) override;

private:
    float Progress = 0;
};

#endif // RAINBOWWAVE_H
