#include "GlobalSettings.h"
#include "OpenRGBEffectSettings.h"

#include <QDesktopServices>
#include <QUrl>
#include <string>

#include "global_obs.hpp"

GlobalSettings::GlobalSettings(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GlobalSettings)
{
    ui->setupUi(this);

    ui->fpscaptureSlider->setValue(OpenRGBEffectSettings::globalSettings.fpscapture);
    ui->brightnessSlider->setValue(OpenRGBEffectSettings::globalSettings.brightness);
    ui->temperature->setValue(OpenRGBEffectSettings::globalSettings.temperature);
    ui->tint->setValue(OpenRGBEffectSettings::globalSettings.tint);
    ui->fpsSlider->setValue(OpenRGBEffectSettings::globalSettings.fps);
    ui->hide_unsupportedCheckBox->setChecked(OpenRGBEffectSettings::globalSettings.hide_unsupported);
    ui->randomColorsCheckBox->setChecked(OpenRGBEffectSettings::globalSettings.prefer_random);
    ui->usePreferedColorsCheckBox->setChecked(OpenRGBEffectSettings::globalSettings.use_prefered_colors);

    ui->preferedColors->SetText("Prefered colors");
    ui->preferedColors->setEnabled(OpenRGBEffectSettings::globalSettings.use_prefered_colors);
    ui->preferedColors->SetColors(OpenRGBEffectSettings::globalSettings.prefered_colors);
    audio_settings.SetSettings(&OpenRGBEffectSettings::globalSettings.audio_settings);

    connect(&audio_settings, &AudioSettings::AudioDeviceChanged, [=](int value){
        OpenRGBEffectSettings::globalSettings.audio_settings.audio_device = value;
    });

    ui->obsFpsSlider->setValue(obs::getFramerate());
    this->connect(ui->obsFpsSlider, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), [](int value) {
        obs::setFramerate(value);
    });

    ui->obsVideoSourceType->clear();
    ui->obsVideoSourceType->addItems({
        "None",
        "Monitor",
        "Window",
        "Game"
    });
    ui->obsVideoSourceType->setCurrentIndex(static_cast<int>(obs::getVideoSource()));
    this->connect(ui->obsVideoSourceType, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [&](int value) {
        obs::setVideoSource(static_cast<obs::VideoSourceType>(value));
        this->UpdateObsVideoSourcePropertiesView();
    });
    this->UpdateObsVideoSourcePropertiesView();

    ui->obsAudioSourceType->clear();
    ui->obsAudioSourceType->addItems({
        "None",
        "Input",
        "Output",
        "Application"
    });
    ui->obsAudioSourceType->setCurrentIndex(static_cast<int>(obs::getAudioSource()));
    this->connect(ui->obsAudioSourceType, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [&](int value) {
        obs::setAudioSource(static_cast<obs::AudioSourceType>(value));
        this->UpdateObsAudioSourcePropertiesView();
    });
    this->UpdateObsAudioSourcePropertiesView();
}

GlobalSettings::~GlobalSettings()
{
    delete ui;
    delete m_obsVideoSourcePropertiesView;
    delete m_obsAudioSourcePropertiesView;
    OpenRGBEffectSettings::WriteGlobalSettings();
}

void GlobalSettings::UpdateObsVideoSourcePropertiesView() {
    if (m_obsVideoSourcePropertiesView) {
        ui->obsVideoSourcePropertiesLayout->removeWidget(m_obsVideoSourcePropertiesView);
        delete m_obsVideoSourcePropertiesView;
        m_obsVideoSourcePropertiesView = nullptr;
    }
    m_obsVideoSourcePropertiesView = obs::createVideoPropertiesView();
    if (m_obsVideoSourcePropertiesView) {
        ui->obsVideoSourcePropertiesLayout->addWidget(m_obsVideoSourcePropertiesView);
    }
}

void GlobalSettings::UpdateObsAudioSourcePropertiesView() {
    if (m_obsAudioSourcePropertiesView) {
        ui->obsAudioSourcePropertiesLayout->removeWidget(m_obsAudioSourcePropertiesView);
        delete m_obsAudioSourcePropertiesView;
        m_obsAudioSourcePropertiesView = nullptr;
    }
    m_obsAudioSourcePropertiesView = obs::createAudioPropertiesView();
    if (m_obsAudioSourcePropertiesView) {
        ui->obsAudioSourcePropertiesLayout->addWidget(m_obsAudioSourcePropertiesView);
    }
}

void GlobalSettings::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
}

void GlobalSettings::on_fpscaptureSlider_valueChanged(int value)
{
    OpenRGBEffectSettings::globalSettings.fpscapture = value;
}

void GlobalSettings::on_brightnessSlider_valueChanged(int value)
{
    OpenRGBEffectSettings::globalSettings.brightness = value;
}

void GlobalSettings::on_fpsSlider_valueChanged(int value)
{
    OpenRGBEffectSettings::globalSettings.fps = value;
}

void GlobalSettings::on_temperature_valueChanged(int value)
{
    OpenRGBEffectSettings::globalSettings.temperature = value;
}

void GlobalSettings::on_tint_valueChanged(int value)
{
    OpenRGBEffectSettings::globalSettings.tint = value;
}

void GlobalSettings::on_hide_unsupportedCheckBox_stateChanged(int state)
{
    OpenRGBEffectSettings::globalSettings.hide_unsupported = state;
}

void GlobalSettings::on_usePreferedColorsCheckBox_stateChanged(int state)
{
    ui->preferedColors->setEnabled(state);
    OpenRGBEffectSettings::globalSettings.use_prefered_colors = state;
}

void GlobalSettings::on_preferedColors_ColorsChanged()
{
    OpenRGBEffectSettings::globalSettings.prefered_colors = ui->preferedColors->Colors();
}

void GlobalSettings::on_randomColorsCheckBox_stateChanged(int state)
{
    OpenRGBEffectSettings::globalSettings.prefer_random = state;
}

void GlobalSettings::on_audioSettings_clicked()
{
    audio_settings.setModal(true);
    audio_settings.show();
}
