#include <QFile>
#include <QGuiApplication>
#include <QProcess>
#include <QWindow>

#include <app/modules/brightness.hpp>

BrightnessModule::BrightnessModule(bool enable_androidauto_update) : QObject(qApp)
{
    this->enable_androidauto_update = enable_androidauto_update;
}

MockedBrightnessModule::MockedBrightnessModule(QMainWindow *window) : BrightnessModule(true) { this->window = window; }

void MockedBrightnessModule::set_brightness(int brightness) { this->window->setWindowOpacity(brightness / 255.0); }

RpiBrightnessModule::RpiBrightnessModule() : BrightnessModule(false), stream(new QFile(this->PATH))
{
    this->stream.device()->open(QIODevice::WriteOnly | QIODevice::ExistingOnly);
}

RpiBrightnessModule::~RpiBrightnessModule()
{
    QIODevice *device = this->stream.device();
    if (device->isOpen()) device->close();
}

void RpiBrightnessModule::set_brightness(int brightness)
{
    QFileDevice *device = qobject_cast<QFileDevice *>(this->stream.device());
    if (device->isOpen()) {
        device->resize(0);
        this->stream << brightness << endl;
    }
}

XBrightnessModule::XBrightnessModule() : BrightnessModule(false) { this->screen = QGuiApplication::primaryScreen(); }

void XBrightnessModule::set_brightness(int brightness)
{
    QProcess process(this);
    process.start(QString("xrandr --output %1 --brightness %2").arg(this->screen->name()).arg(brightness / 255.0));
    process.waitForFinished();
}
