#include "Ambient.h"
#include "ColorUtils.h"
#include "OpenRGBEffectSettings.h"
#include "QtScreenCapturer.h"

#ifdef __linux__
#include "WaylandScreenCapturer.h"
#elif _WIN32
#include "WindowsScreenCapturer.h"
#endif

REGISTER_EFFECT(Ambient);

Ambient::Ambient(QWidget *parent) :
    RGBEffect(parent),
    ui(new Ui::Ambient)
{
    ui->setupUi(this);

    EffectDetails.EffectName = "Ambient";
    EffectDetails.EffectClassName = ClassName();
    EffectDetails.EffectDescription = "Takes a portion of the screen and reflect it to your devices";
    EffectDetails.HasCustomSettings = true;
    EffectDetails.SupportsRandom = false;

    ui->mode->addItems({
        "Scaled average",
        "Screen copy"
    });

    ui->select_screen->hide();

#ifdef __linux__
    bool isWayland = qgetenv("XDG_SESSION_TYPE") == "wayland";

    if(isWayland)
    {
        capturer = new WaylandScreenCapturer();

        ui->select_rectangle->hide();
        ui->screen_label->hide();
        ui->screen->hide();
        ui->framerate_label->hide();
        ui->framerate->hide();
        ui->select_screen->show();
    }
#elif _WIN32
    capturer = new WindowsScreenCapturer();
#endif

    if(capturer == nullptr)
    {
        capturer = new QtScreenCapturer();
    }

    ui->framerate->setValue(OpenRGBEffectSettings::globalSettings.fpscapture);

    rectangle_selector_overlay = new RectangleSelectorOverlay(this);

    connect(rectangle_selector_overlay, &RectangleSelectorOverlay::SelectionUpdated, [=](QRect rect){
        ui->left->setValue(rect.left());
        ui->top->setValue(rect.top());
        ui->width->setValue(rect.width());
        ui->height->setValue(rect.height());
    });

    QList<QScreen*> screens = QGuiApplication::screens();

    for(QScreen* screen: screens)
    {
        ui->screen->addItem(screen->name());
    }

    connect(capturer, &ScreenCapturer::OnImage, [&](const QImage& img){
        lock.lock();
        image = img.copy(QRect(left,top,width,height));
        lock.unlock();
    });

    capturer->SetScreen(0);

    connect(capturer, &ScreenCapturer::OnRestoreTokenProvided,[&](const QString token){
        restore_token = token;
    });
}

Ambient::~Ambient()
{
    delete rectangle_selector_overlay;
    delete ui;
}

void Ambient::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
}

void Ambient::StepEffect(std::vector<ControllerZone*> controller_zones)
{
    if(controller_zones.empty())
    {
        return;
    }

    if(width == 0 || height == 0)
    {
        for(ControllerZone* controller_zone : controller_zones)
        {
            controller_zone->SetAllZoneLEDs(0, Brightness, Temperature, Tint);
        }

        return;
    }

    lock.lock();

    switch (mode) {

    case SCALED_AVERAGE:
    {
        QImage scaled_internal = image.scaled(1, 1);

        RGBColor color = ColorUtils::fromQColor(scaled_internal.pixelColor(0,0));

        for(ControllerZone* controller_zone : controller_zones)
        {
            RGBColor previous_color = controller_zone->controller->GetLED(0);
            RGBColor smoothed = Smooth(previous_color, color);
            controller_zone->SetAllZoneLEDs(smoothed, Brightness, Temperature, Tint);
        }

        break;
    }

    case SCREEN_COPY:
    {
        for(ControllerZone* controller_zone : controller_zones)
        {
            unsigned int leds_count = controller_zone->leds_count();
            bool reverse = controller_zone->reverse;

            if(controller_zone->type() == ZONE_TYPE_SINGLE || controller_zone->type() == ZONE_TYPE_LINEAR)
            {
                unsigned int width = controller_zone->leds_count();
                unsigned int height = 1;

                QImage scaled = image.scaled(width, height);

                for(unsigned int i = 0; i < width; i++)
                {
                    QColor color = scaled.pixelColor(reverse ? leds_count - i - 1 : i, 0);
                    RGBColor previous_color = controller_zone->controller->GetLED(i);

                    controller_zone->SetLED(i, Smooth(previous_color, ColorUtils::fromQColor(color)), Brightness, Temperature, Tint);
                }

            }
            else if(controller_zone->type() == ZONE_TYPE_MATRIX)
            {
                unsigned int width = controller_zone->matrix_map_width();
                unsigned int height = controller_zone->matrix_map_height();
                unsigned int * map = controller_zone->map();

                QImage scaled = image.scaled(width, height);            

                for(unsigned int h = 0; h < height; h++)
                {
                    for(unsigned int w = 0; w <  width; w++)
                    {
                        QColor color = scaled.pixelColor(reverse ? width - w - 1: w, h);

                        unsigned int led_num = map[h * width + w];
                        RGBColor previous_color = controller_zone->controller->GetLED(led_num);

                        controller_zone->SetLED(led_num, Smooth(previous_color, ColorUtils::fromQColor(color)), Brightness, Temperature, Tint);
                    }
                }
            }
        }

        break;
    }
    }

    lock.unlock();
}

RGBColor Ambient::Smooth(const RGBColor& previous_color, RGBColor color)
{
    if(color != previous_color)
    {
        return ColorUtils::Interpolate(previous_color, color, 0.01 * (100 - smoothness));
    }

    return color;
}

void Ambient::EffectState(bool state)
{
    if(state)
    {
        if(!restore_token.isEmpty())
        {
            capturer->Init(restore_token);
        }
        else
        {
            capturer->Start();
        }
    }
    else
    {
        capturer->Stop();
    }
}

void Ambient::LoadCustomSettings(json settings)
{
    if(settings.contains("left"))           ui->left->setValue(settings["left"]);
    if(settings.contains("top"))            ui->top->setValue(settings["top"]);
    if(settings.contains("width"))          ui->width->setValue(settings["width"]);
    if(settings.contains("height"))         ui->height->setValue(settings["height"]);
    if(settings.contains("mode"))           ui->mode->setCurrentIndex(settings["mode"]);
    if(settings.contains("screen_index"))   ui->screen->setCurrentIndex(settings["screen_index"]);
    if(settings.contains("smoothness"))     ui->smoothness->setValue(settings["smoothness"]);
    if(settings.contains("framerate"))      ui->framerate->setValue(settings["framerate"]);

    if(settings.contains("restore_token"))
    {
        restore_token = QString::fromStdString(settings["restore_token"]);
    }
}

json Ambient::SaveCustomSettings()
{
    json settings;

    settings["left"]            = left;
    settings["top"]             = top;
    settings["height"]          = height;
    settings["width"]           = width;
    settings["mode"]            = mode;
    settings["screen_index"]    = screen_index;
    settings["smoothness"]      = smoothness;
    settings["framerate"]       = framerate;
    settings["restore_token"]   = restore_token.toStdString();

    return settings;
}

void Ambient::on_left_valueChanged(int value)
{
    left = value;   
}

void Ambient::on_top_valueChanged(int value)
{
    top = value;
}

void Ambient::on_width_valueChanged(int value)
{
    width = value;
}

void Ambient::on_height_valueChanged(int value)
{
    height = value;
}

void Ambient::on_mode_currentIndexChanged(int value)
{
    mode = static_cast<AmbientMode>(value);
}

void Ambient::on_screen_currentIndexChanged(int value)
{
    screen_index = value;
    capturer->SetScreen(screen_index);
}

void Ambient::on_select_rectangle_clicked()
{
    rectangle_selector_overlay->StartSelection(screen_index);
}

void Ambient::on_select_screen_clicked()
{
    capturer->Init();
}

void Ambient::on_smoothness_valueChanged(int value)
{
    smoothness = value;
}

void Ambient::on_framerate_valueChanged(int value)
{
    framerate = value;
    capturer->SetFrameRate(framerate);
}
