#pragma once

#include <QWidget>
#include <QMouseEvent>
#include "ui_AmbientObs.h"
#include "RGBEffect.h"
#include "EffectRegisterer.h"
#include "global_obs.hpp"

namespace Ui {
    class AmbientObs;
}

enum class AmbientObsMode { Copy, Average };

class AmbientObs : public RGBEffect {
    Q_OBJECT

public:
    explicit AmbientObs(QWidget *parent = nullptr);
    ~AmbientObs();

    EFFECT_REGISTERER(ClassName(), UI_Name(), CAT_SPECIAL, [](){ return new AmbientObs; });

    static std::string const ClassName() { return "AmbientObs"; }
    static std::string const UI_Name() { return QT_TR_NOOP("Ambient OBS"); }

    void StepEffect(std::vector<ControllerZone*>) override;
    void LoadCustomSettings(json) override;
    json SaveCustomSettings() override;
    void EffectState(bool) override;

private slots:
    void changeEvent(QEvent *event) override;
    void on_mode_currentIndexChanged(int);
    void on_crop_stateChanged(int);
    void on_cropLeft_valueChanged(int);
    void on_cropTop_valueChanged(int);
    void on_cropWidth_valueChanged(int);
    void on_cropHeight_valueChanged(int);

private:
    Ui::AmbientObs* m_ui;

    void UpdateSelection();
    void SetDynamicStrings();

    AmbientObsMode m_mode = AmbientObsMode::Copy;
    bool m_crop = false;
    unsigned int m_cropLeft = 0;
    unsigned int m_cropTop = 0;
    unsigned int m_cropWidth = 1;
    unsigned int m_cropHeight = 1;

    QImage m_image;
    std::mutex m_imageLock;
};
