#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <QWidget>
#include "ui_Sequence.h"
#include "RGBEffect.h"
#include "EffectRegisterer.h"

namespace Ui {
class Sequence;
}

class Sequence : public RGBEffect
{
    Q_OBJECT

public:
    explicit Sequence(QWidget *parent = nullptr);
    ~Sequence();

    EFFECT_REGISTERER(ClassName(), UI_Name(), CAT_ADVANCED, [](){return new Sequence;});

    static std::string const ClassName() {return "Sequence";}
    static std::string const UI_Name() { return QT_TR_NOOP("Sequence"); }

    void StepEffect(std::vector<ControllerZone*>) override;
    void LoadCustomSettings(json) override;
    json SaveCustomSettings() override;

private:
    Ui::Sequence *ui;

    void SetDynamicStrings();

    double progress = 0.f;

private slots:
    void changeEvent(QEvent *event) override;
};

#endif // SEQUENCE_H
