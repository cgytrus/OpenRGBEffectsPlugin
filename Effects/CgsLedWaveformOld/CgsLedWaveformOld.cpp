#include "CgsLedWaveformOld.hpp"
#include "global_obs.hpp"

REGISTER_EFFECT(CgsLedWaveformOld);

CgsLedWaveformOld::~CgsLedWaveformOld() { }
CgsLedWaveformOld::CgsLedWaveformOld(QWidget* parent) : CgsLedEffect(parent) {
    EffectDetails.HasCustomSettings = true;
    SetSpeed(100);

    m_ui->hueSpeed->setRange(-std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity());
    m_ui->hueSpeed->setSingleStep(0.1);
    m_ui->hueSpeed->setValue(static_cast<double>(m_colors.hueSpeed));
    this->connect(m_ui->hueSpeed, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [&](double value) {
        m_colors.hueSpeed = static_cast<float>(value);
    });

    m_ui->hueOffset->setRange(-std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity());
    m_ui->hueOffset->setSingleStep(1.0);
    m_ui->hueOffset->setValue(static_cast<double>(m_colors.hueOffset));
    this->connect(m_ui->hueOffset, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [&](double value) {
        m_colors.hueOffset = static_cast<float>(value);
    });

    m_ui->rightHueOffset->setRange(-std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity());
    m_ui->rightHueOffset->setSingleStep(1.0);
    m_ui->rightHueOffset->setValue(static_cast<double>(m_colors.rightHueOffset));
    this->connect(m_ui->rightHueOffset, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [&](double value) {
        m_colors.rightHueOffset = static_cast<float>(value);
    });

    m_ui->hueRange->setRange(-std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity());
    m_ui->hueRange->setSingleStep(1.0);
    m_ui->hueRange->setValue(static_cast<double>(m_colors.hueRange));
    this->connect(m_ui->hueRange, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [&](double value) {
        m_colors.hueRange = static_cast<float>(value);
    });

    m_ui->saturation->setRange(0.0, 1.0);
    m_ui->saturation->setSingleStep(0.01);
    m_ui->saturation->setValue(static_cast<double>(m_colors.saturation));
    this->connect(m_ui->saturation, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [&](double value) {
        m_colors.saturation = static_cast<float>(value);
    });

    m_ui->bufferSeconds->setRange(0.0, std::numeric_limits<double>::infinity());
    m_ui->bufferSeconds->setSingleStep(0.01);
    m_ui->bufferSeconds->setValue(static_cast<double>(m_bufferSeconds));
    this->connect(m_ui->bufferSeconds, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [&](double value) {
        m_bufferSeconds = static_cast<float>(value);
    });

    m_ui->displaySeconds->setRange(0.0, std::numeric_limits<double>::infinity());
    m_ui->displaySeconds->setSingleStep(0.01);
    m_ui->displaySeconds->setValue(static_cast<double>(m_displaySeconds));
    this->connect(m_ui->displaySeconds, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [&](double value) {
        m_displaySeconds = static_cast<float>(value);
    });

    m_ui->avgCount->setRange(0, 48000);
    m_ui->avgCount->setSingleStep(1);
    m_ui->avgCount->setValue(m_avgCount);
    this->connect(m_ui->avgCount, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), [&](int value) {
        m_avgCount = value;
    });
}

void CgsLedWaveformOld::load(json settings) {
    if (settings.contains("colors"))
        m_colors = settings["colors"];
    if (settings.contains("bufferSeconds"))
        m_bufferSeconds = settings["bufferSeconds"];
    if (settings.contains("displaySeconds"))
        m_displaySeconds = settings["displaySeconds"];
    if (settings.contains("avgCount"))
        m_avgCount = settings["avgCount"];
}

json CgsLedWaveformOld::save() {
    json settings;
    settings["colors"] = m_colors;
    settings["bufferSeconds"] = m_bufferSeconds;
    settings["displaySeconds"] = m_displaySeconds;
    settings["avgCount"] = m_avgCount;
    return settings;
}

void CgsLedWaveformOld::start() {
    obs::startMonoAudio(this, [&](float sample) {
        if (m_samples.size() == 0)
            return;
        const std::lock_guard lock(m_samplesLock);
        while (m_samples.size() > this->getBufferSize()) {
            m_samples.erase(m_samples.begin());
            m_displayTail--;
        }
        sample = std::abs(sample);
        if (m_samples.back().count >= m_avgCount) {
            m_samples.push_back({ sample, 1, this->getTime() });
        }
        else {
            auto& s = m_samples.back();
            s.sum += sample;
            s.count++;
        }
    });

    const std::lock_guard lock(m_samplesLock);
    m_samples.clear();
    for (size_t i = 0; i < this->getBufferSize(); i++)
        m_samples.push_back({ 0.0f, 0, 0.0f });
    m_displayTail = m_samples.size();
}

void CgsLedWaveformOld::stop() {
    obs::stopMonoAudio(this);
}

void CgsLedWaveformOld::StepEffect(std::vector<ControllerZone*> zones) {
    const std::lock_guard lock(m_samplesLock);
    while (m_displayTail < m_samples.size() && this->getTime() >= m_samples[std::max<size_t>(m_displayTail, 0)].time)
        m_displayTail++;
    m_showDisplayTail = std::min(m_displayTail, m_samples.size());
    CgsLedEffect::StepEffect(zones);
}

static float lerpUnclamped(float a, float b, float t) { return a + (b - a) * t; }
static float lerp(float a, float b, float t) { return t <= 0.0f ? a : t >= 1.0f ? b : lerpUnclamped(a, b, t); }
RGBColor CgsLedWaveformOld::getColor(unsigned int x, unsigned int, unsigned int width, unsigned int, float t) {
    float progress = static_cast<float>(x) / width * this->getDisplayCount();
    size_t index = (std::max<size_t>(m_showDisplayTail - this->getDisplayCount(), 0) + static_cast<size_t>(progress)) % m_samples.size();
    size_t nextIndex = (index + 1) % m_samples.size();
    float bin = lerp(m_samples[index].getValue(), m_samples[nextIndex].getValue(), progress - index);
    bin = std::clamp(bin, 0.0f, 1.0f);
    return m_colors.get(x, width, t, bin, bin);
}
