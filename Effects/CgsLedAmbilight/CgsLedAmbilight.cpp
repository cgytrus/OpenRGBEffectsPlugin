#include "CgsLedAmbilight.hpp"
#include <QDebug>
#include "ColorUtils.h"
#include "global_obs.hpp"
#include "gamma8.hpp"
#include "LivePreviewController.h"

REGISTER_EFFECT(CgsLedAmbilight);

CgsLedAmbilight::CgsLedAmbilight(QWidget* parent) : CgsLedEffect(parent) {
    m_ui->mode->addItems({
        "Copy",
        "Average"
    });
    this->connect(m_ui->mode, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [&](int value) {
        m_mode = static_cast<AmbilightMode>(value);
    });

    m_ui->cropLeft->setRange(0.0, 1.0);
    m_ui->cropLeft->setSingleStep(0.0001);
    this->connect(m_ui->cropLeft, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [&](double value) {
        m_crop.setLeft(static_cast<float>(value));
    });

    m_ui->cropRight->setRange(0.0, 1.0);
    m_ui->cropRight->setSingleStep(0.0001);
    this->connect(m_ui->cropRight, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [&](double value) {
        m_crop.setRight(static_cast<float>(value));
    });

    m_ui->cropTop->setRange(0.0, 1.0);
    m_ui->cropTop->setSingleStep(0.0001);
    this->connect(m_ui->cropTop, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [&](double value) {
        m_crop.setTop(static_cast<float>(value));
    });

    m_ui->cropBottom->setRange(0.0, 1.0);
    m_ui->cropBottom->setSingleStep(0.0001);
    this->connect(m_ui->cropBottom, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [&](double value) {
        m_crop.setBottom(static_cast<float>(value));
    });

    this->onShouldUpdateUi();
}

CgsLedAmbilight::~CgsLedAmbilight() {
    this->stop();
}

void CgsLedAmbilight::onShouldUpdateUi() {
    m_ui->mode->setCurrentIndex(static_cast<int>(m_mode));
    m_ui->cropLeft->setValue(static_cast<double>(m_crop.left()));
    m_ui->cropRight->setValue(static_cast<double>(m_crop.right()));
    m_ui->cropTop->setValue(static_cast<double>(m_crop.top()));
    m_ui->cropBottom->setValue(static_cast<double>(m_crop.bottom()));
}

void CgsLedAmbilight::load(json settings) {
    if (settings.contains("mode"))
        m_mode = settings["mode"];
    if (settings.contains("cropLeft"))
        m_crop.setLeft(settings["cropLeft"]);
    if (settings.contains("cropTop"))
        m_crop.setTop(settings["cropTop"]);
    if (settings.contains("cropWidth"))
        m_crop.setWidth(settings["cropWidth"]);
    if (settings.contains("cropHeight"))
        m_crop.setHeight(settings["cropHeight"]);
}

json CgsLedAmbilight::save() {
    json settings;

    settings["mode"] = m_mode;
    settings["cropLeft"] = m_crop.left();
    settings["cropTop"] = m_crop.top();
    settings["cropWidth"] = m_crop.width();
    settings["cropHeight"] = m_crop.height();

    return settings;
}

void CgsLedAmbilight::start() {
    qDebug() << "[CgsLedAmbilight] Start capturer";
    obs::startVideo(this, [&](QImage img) {
        auto size = obs::getOutputSize();
        auto image = img.copy(QRectF{
            size.width() * m_crop.left(),
            size.height() * m_crop.top(),
            std::max(size.width() * m_crop.width(), 1.0),
            std::max(size.height() * m_crop.height(), 1.0)
        }.toRect());
        m_imageLock.lock();
        m_image = image;
        m_imageLock.unlock();
    });
}

void CgsLedAmbilight::stop() {
    obs::stopVideo(this);
}

void CgsLedAmbilight::draw(std::vector<ControllerZone*> zones) {
    if (zones.empty())
        return;

    m_imageLock.lock();
    QImage image = m_image;
    m_imageLock.unlock();

    if (image.isNull())
        return;

    switch (m_mode) {
        case AmbilightMode::Average: {
            QImage scaled = image.scaled(1, 1, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            for (const auto& zone : zones) {
                bool isPreview = this->zoneIsPreview(zone);
                auto color = ColorUtils::fromQColor(scaled.pixelColor(0, 0));
                color = this->gamma(color, isPreview);
                zone->SetAllZoneLEDs(color, Brightness, Temperature, Tint);
            }
            break;
        }
        case AmbilightMode::Copy: {
            for (const auto& zone : zones) {
                bool isPreview = this->zoneIsPreview(zone);
                auto width = this->zoneWidth(zone);
                auto height = this->zoneHeight(zone);
                bool reverse = zone->reverse;
                QImage scaled = image.scaled(width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                for (unsigned int y = 0; y < height; y++) {
                    for (unsigned int x = 0; x < width; x++) {
                        auto i = this->zoneLed(zone, x, y);
                        auto color = ColorUtils::fromQColor(scaled.pixelColor(reverse ? width - x - 1 : x, y));
                        color = this->gamma(color, isPreview);
                        zone->SetLED(i, color, Brightness, Temperature, Tint);
                    }
                }
            }
            break;
        }
    }
}
