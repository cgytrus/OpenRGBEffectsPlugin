#include "AmbientObs.hpp"
#include <QDebug>
#include "ColorUtils.h"
#include "OpenRGBEffectSettings.h"
#include "global_obs.hpp"
#include "gamma8.hpp"

REGISTER_EFFECT(AmbientObs);

AmbientObs::AmbientObs(QWidget *parent) : RGBEffect(parent), m_ui(new Ui::AmbientObs) {
    m_ui->setupUi(this);

    SetDynamicStrings();
    EffectDetails.EffectClassName = ClassName();
    EffectDetails.HasCustomSettings = true;
    EffectDetails.SupportsRandom = false;

    m_ui->crop->setCheckState(m_crop ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    m_ui->gammaCorrection->setCheckState(m_gammaCorrection ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);

    m_ui->cropFrame->hide();
}

AmbientObs::~AmbientObs() {
    obs::stopVideo(this);
    delete m_ui;
}

void AmbientObs::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        m_ui->retranslateUi(this);
        SetDynamicStrings();
    }
}

void AmbientObs::SetDynamicStrings() {
    EffectDetails.EffectName = tr(UI_Name().c_str()).toStdString();
    EffectDetails.EffectDescription = tr("Takes a portion of the screen and reflect it to your devices").toStdString();

    m_ui->mode->clear();
    m_ui->mode->addItems({
        tr("Copy"),
        tr("Average")
    });
}

void AmbientObs::EffectState(const bool state) {
    EffectEnabled = state;
    if (state) {
        qDebug() << "[AmbientObs] Start capturer";
        obs::startVideo(this, [&](QImage img) {
            m_imageLock.lock();
            m_image = m_crop ?
                img.copy(QRect(m_cropLeft, m_cropTop, m_cropWidth, m_cropHeight)) :
                img.copy({ {}, obs::getOutputSize() });
            m_imageLock.unlock();
        });
    }
    else {
        obs::stopVideo(this);
    }
}

void AmbientObs::StepEffect(std::vector<ControllerZone*> controller_zones) {
    if (controller_zones.empty())
        return;

    if (m_crop && (m_cropWidth == 0 || m_cropHeight == 0)) {
        for (ControllerZone* controller_zone : controller_zones) {
            controller_zone->SetAllZoneLEDs(0, Brightness, Temperature, Tint);
        }
        return;
    }

    if (m_image.isNull()) {
        return;
    }

    m_imageLock.lock();

    switch (m_mode) {
        case AmbientObsMode::Average: {
            QImage scaled = m_image.scaled(1, 1, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            auto color = ColorUtils::fromQColor(scaled.pixelColor(0, 0));
            if (m_gammaCorrection)
                color = applyGamma(color);
            for (ControllerZone* controller_zone : controller_zones) {
                controller_zone->SetAllZoneLEDs(color, Brightness, Temperature, Tint);
            }
            break;
        }

        case AmbientObsMode::Copy: {
            for (ControllerZone* controller_zone : controller_zones) {
                bool reverse = controller_zone->reverse;
                if (controller_zone->type() == ZONE_TYPE_SINGLE || controller_zone->type() == ZONE_TYPE_LINEAR) {
                    unsigned int width = controller_zone->leds_count();
                    unsigned int height = 1;
                    QImage scaled = m_image.scaled(width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                    for (unsigned int i = 0; i < width; i++) {
                        auto color = ColorUtils::fromQColor(scaled.pixelColor(reverse ? width - i - 1 : i, 0));
                        if (m_gammaCorrection)
                            color = applyGamma(color);
                        controller_zone->SetLED(i, color, Brightness, Temperature, Tint);
                    }
                }
                else if (controller_zone->type() == ZONE_TYPE_MATRIX) {
                    unsigned int width = controller_zone->matrix_map_width();
                    unsigned int height = controller_zone->matrix_map_height();
                    QImage scaled = m_image.scaled(width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                    unsigned int* map = controller_zone->map();
                    for (unsigned int y = 0; y < height; y++) {
                        for (unsigned int x = 0; x < width; x++) {
                            unsigned int i = map[y * width + x];
                            auto color = ColorUtils::fromQColor(scaled.pixelColor(reverse ? width - x - 1 : x, y));
                            if (m_gammaCorrection)
                                color = applyGamma(color);
                            controller_zone->SetLED(i, color, Brightness, Temperature, Tint);
                        }
                    }
                }
            }
            break;
        }
    }

    m_imageLock.unlock();
}

void AmbientObs::LoadCustomSettings(json settings) {
    if (settings.contains("mode"))
        m_ui->mode->setCurrentIndex(settings["mode"]);
    if (settings.contains("gammaCorrection"))
        m_ui->gammaCorrection->setChecked(settings["gammaCorrection"]);
    if (settings.contains("crop"))
        m_ui->crop->setChecked(settings["crop"]);
    if (settings.contains("cropLeft"))
        m_ui->cropLeft->setValue(settings["cropLeft"]);
    if (settings.contains("cropTop"))
        m_ui->cropTop->setValue(settings["cropTop"]);
    if (settings.contains("cropWidth"))
        m_ui->cropWidth->setValue(settings["cropWidth"]);
    if (settings.contains("cropHeight"))
        m_ui->cropHeight->setValue(settings["cropHeight"]);
}

json AmbientObs::SaveCustomSettings() {
    json settings;

    settings["mode"] = m_mode;
    settings["gammaCorrection"] = m_gammaCorrection;
    settings["crop"] = m_crop;
    settings["cropLeft"] = m_cropLeft;
    settings["cropTop"] = m_cropTop;
    settings["cropWidth"] = m_cropWidth;
    settings["cropHeight"] = m_cropHeight;

    return settings;
}

void AmbientObs::on_mode_currentIndexChanged(int value) {
    m_mode = static_cast<AmbientObsMode>(value);
}

void AmbientObs::on_crop_stateChanged(int value) {
    m_crop = value;
    m_ui->cropFrame->setVisible(value);
}

void AmbientObs::on_gammaCorrection_stateChanged(int value) {
    m_gammaCorrection = value;
}

void AmbientObs::on_cropLeft_valueChanged(int value) {
    m_cropLeft = value;
}

void AmbientObs::on_cropTop_valueChanged(int value) {
    m_cropTop = value;
}

void AmbientObs::on_cropWidth_valueChanged(int value) {
    m_cropWidth = value;
}

void AmbientObs::on_cropHeight_valueChanged(int value) {
    m_cropHeight = value;
}
