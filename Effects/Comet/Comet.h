#ifndef COMET_H
#define COMET_H

#include <QWidget>
#include "ui_Comet.h"
#include "RGBEffect.h"
#include "EffectRegisterer.h"
#include "hsv.h"

namespace Ui {
class Comet;
}

class Comet : public RGBEffect
{
    Q_OBJECT

public:
    explicit Comet(QWidget *parent = nullptr);
    ~Comet();

    EFFECT_REGISTERER(ClassName(), UI_Name(), CAT_SIMPLE, [](){return new Comet;});

    static std::string const ClassName() {return "Comet";}
    static std::string const UI_Name() { return QT_TR_NOOP("Comet"); }

    void StepEffect(std::vector<ControllerZone*>) override;

private:
    Ui::Comet *ui;

    void SetDynamicStrings();

    double time = 0;
    double progress = 0;

    RGBColor GetColor(unsigned int, unsigned int);

    hsv_t tmp;

private slots:
    void changeEvent(QEvent *event) override;
};

#endif // COMET_H
