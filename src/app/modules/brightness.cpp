#include <QApplication>
#include <QProcess>
#include <QWindow>
#include <QDebug>

#include <app/modules/brightness.hpp>

BrightnessModule::BrightnessModule(QMainWindow *window) : QObject(qApp) { this->window = window; }

int MockedBrightnessModule::get_brightness() { return this->window->windowOpacity() * 255; }

void MockedBrightnessModule::set_brightness(int brightness) { this->window->setWindowOpacity(brightness / 255.0); }

XBrightnessModule::XBrightnessModule(QMainWindow *window) : BrightnessModule(window)
{
    this->screen = qApp->screens()[0];
}

int XBrightnessModule::get_brightness()
{
    QProcess process(this);
    process.start("xrandr --current --verbose");
    process.waitForFinished();
    qDebug() << process.readAllStandardOutput();

    return 255;

}

void XBrightnessModule::set_brightness(int brightness)
{
    QProcess process(this);
    process.start(QString("xrandr --output %1 --brightness %2").arg(this->screen->name()).arg(brightness / 255.0));
    process.waitForFinished();
}
