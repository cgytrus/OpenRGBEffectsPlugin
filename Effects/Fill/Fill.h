#ifndef FILL_H
#define FILL_H

#include <QWidget>
#include "ui_Fill.h"
#include "RGBEffect.h"
#include "EffectRegisterer.h"

namespace Ui {
class Fill;
}

class Fill : public RGBEffect
{
    Q_OBJECT

public:
    explicit Fill(QWidget *parent = nullptr);
    ~Fill();

    EFFECT_REGISTERER(ClassName(), UI_Name(), CAT_SIMPLE, [](){return new Fill;});

    static std::string const ClassName() {return "Fill";}
    static std::string const UI_Name() { return QT_TR_NOOP("Fill"); }

    void StepEffect(std::vector<ControllerZone*>) override;

private:
    Ui::Fill *ui;

    void SetDynamicStrings();

    double time = 0;
    double progress = 0;
    double old_progress = 0;
    RGBColor random;

    RGBColor GetColor(unsigned int, unsigned int);

private slots:
    void changeEvent(QEvent *event) override;
};

#endif // FILL_H
