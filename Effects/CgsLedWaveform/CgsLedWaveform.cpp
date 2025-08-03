#include "CgsLedWaveform.hpp"
#include "global_obs.hpp"

REGISTER_EFFECT(CgsLedWaveform);

CgsLedWaveform::~CgsLedWaveform() { }
CgsLedWaveform::CgsLedWaveform(QWidget* parent) : CgsLedEffect(parent) {
    m_ui->hueSpeed->setRange(-std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity());
    m_ui->hueSpeed->setSingleStep(0.1);
    this->connect(m_ui->hueSpeed, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [&](double value) {
        m_colors.hueSpeed = static_cast<float>(value);
    });

    m_ui->hueOffset->setRange(-std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity());
    m_ui->hueOffset->setSingleStep(1.0);
    this->connect(m_ui->hueOffset, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [&](double value) {
        m_colors.hueOffset = static_cast<float>(value);
    });

    m_ui->rightHueOffset->setRange(-std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity());
    m_ui->rightHueOffset->setSingleStep(1.0);
    this->connect(m_ui->rightHueOffset, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [&](double value) {
        m_colors.rightHueOffset = static_cast<float>(value);
    });

    m_ui->hueRange->setRange(-std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity());
    m_ui->hueRange->setSingleStep(1.0);
    this->connect(m_ui->hueRange, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [&](double value) {
        m_colors.hueRange = static_cast<float>(value);
    });

    m_ui->saturation->setRange(0.0, 1.0);
    m_ui->saturation->setSingleStep(0.01);
    this->connect(m_ui->saturation, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [&](double value) {
        m_colors.saturation = static_cast<float>(value);
    });

    m_ui->hueOffsetMode->addItems({
        "Signed",
        "Absolute",
        "Centered"
    });
    this->connect(m_ui->hueOffsetMode, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [&](int value) {
        m_hueOffsetMode = static_cast<HueOffsetMode>(value);
    });

    m_ui->displaySeconds->setRange(0.0, std::numeric_limits<double>::infinity());
    m_ui->displaySeconds->setSingleStep(0.01);
    this->connect(m_ui->displaySeconds, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [&](double value) {
        m_displaySeconds = static_cast<float>(value);
    });

    this->onShouldUpdateUi();
}

void CgsLedWaveform::onShouldUpdateUi() {
    m_ui->hueSpeed->setValue(static_cast<double>(m_colors.hueSpeed));
    m_ui->hueOffset->setValue(static_cast<double>(m_colors.hueOffset));
    m_ui->rightHueOffset->setValue(static_cast<double>(m_colors.rightHueOffset));
    m_ui->hueRange->setValue(static_cast<double>(m_colors.hueRange));
    m_ui->saturation->setValue(static_cast<double>(m_colors.saturation));
    m_ui->hueOffsetMode->setCurrentIndex(static_cast<int>(m_hueOffsetMode));
    m_ui->displaySeconds->setValue(static_cast<double>(m_displaySeconds));
}

void CgsLedWaveform::load(json settings) {
    if (settings.contains("colors"))
        m_colors = settings["colors"];
    if (settings.contains("displaySeconds"))
        m_displaySeconds = settings["displaySeconds"];
}

json CgsLedWaveform::save() {
    json settings;
    settings["colors"] = m_colors;
    settings["displaySeconds"] = m_displaySeconds;
    return settings;
}

void CgsLedWaveform::start() {
    obs::startMonoAudio(this, [&](float sample) {
        if (this->getDisplayCount() == 0) {
            m_samples.clear();
            m_displayTail = 0;
            m_bufferHead = 0;
            return;
        }
        const std::lock_guard lock(m_samplesLock);
        if (m_samples.size() != this->getDisplayCount()) {
            m_samples.resize(this->getDisplayCount(), 0.0f);
            m_displayTail = 0;
            m_bufferHead = m_samples.size() - 1;
        }
        m_samples[m_bufferHead] = sample;
        m_bufferHead = (m_bufferHead + 1) % m_samples.size();
        m_displayTail = (m_displayTail + 1) % m_samples.size();
    });

    const std::lock_guard lock(m_samplesLock);
    m_samples.clear();
    m_samples.resize(this->getDisplayCount(), 0.0f);
    m_displayTail = 0;
    m_bufferHead = m_samples.size() - 1;
}

void CgsLedWaveform::stop() {
    obs::stopMonoAudio(this);
}

static float lerpUnclamped(float a, float b, float t) { return a + (b - a) * t; }
static float lerp(float a, float b, float t) { return t <= 0.0f ? a : t >= 1.0f ? b : lerpUnclamped(a, b, t); }
RGBColor CgsLedWaveform::draw(unsigned int x, unsigned int, unsigned int width, unsigned int, float t) {
    if (m_samples.empty())
        return m_colors.get(x, width, t, 0.0f, 0.0f);
    size_t head = m_bufferHead;
    if (head < m_displayTail)
        head += m_samples.size();
    float progress = static_cast<float>(x + 1) / width;
    float prevProgress = static_cast<float>(x) / width;
    size_t prevIndex = static_cast<size_t>(lerp(m_displayTail, head, prevProgress));
    size_t index = static_cast<size_t>(lerp(m_displayTail, head, progress));
    float bin = 0.0f;
    size_t count = 0;
    for (size_t i = prevIndex + 1; i <= index; i++) {
        bin += m_samples[i % m_samples.size()];
        count++;
    }
    bin /= std::max<size_t>(count, 1);
    bin = std::clamp(bin, -1.0f, 1.0f);
    switch (m_hueOffsetMode) {
        case HueOffsetMode::Signed:
            break;
        case HueOffsetMode::Absolute:
            bin = std::abs(bin);
            break;
        case HueOffsetMode::Centered:
            bin += 1.0f;
            bin /= 2.0f;
            break;
    }
    return m_colors.get(x, width, t, bin, std::abs(bin));
}
