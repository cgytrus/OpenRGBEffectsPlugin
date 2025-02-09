#ifndef MOSAIC_H
#define MOSAIC_H

#include <QWidget>
#include "ui_Mosaic.h"
#include "RGBEffect.h"
#include "EffectRegisterer.h"
#include "hsv.h"

namespace Ui {
class Mosaic;
}

struct Tile {
    hsv_t hsv;
    float brightness = 0.f;
    float decrease_speed_mult = 1.f;
};

class Mosaic : public RGBEffect
{
    Q_OBJECT

public:
    explicit Mosaic(QWidget *parent = nullptr);
    ~Mosaic();

    EFFECT_REGISTERER(ClassName(), UI_Name(), CAT_SIMPLE, [](){return new Mosaic;});

    static std::string const ClassName() {return "Mosaic";}
    static std::string const UI_Name() { return QT_TR_NOOP("Mosaic"); }

    void StepEffect(std::vector<ControllerZone*>) override;
    void OnControllerZonesListChanged(std::vector<ControllerZone*>) override;
    void LoadCustomSettings(json) override;
    json SaveCustomSettings() override;

private slots:
    void changeEvent(QEvent *event) override;
    void on_rarity_valueChanged(int);
private:
    Ui::Mosaic *ui;

    void SetDynamicStrings();

    void ResetMosaic(std::vector<ControllerZone*>);
    void UpdateTiles(unsigned int);
    std::vector<std::vector<Tile>> tiles;
    int rarity = 10;
};

#endif // MOSAIC_H
