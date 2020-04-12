#ifndef BRIGHTNESS_HPP_
#define BRIGHTNESS_HPP_

#include <QMainWindow>
#include <QObject>
#include <QScreen>

class BrightnessModule : public QObject {
    Q_OBJECT

   public:
    BrightnessModule(QMainWindow *window);

    virtual void set_brightness(int brightness) = 0;
    bool update_androidauto() { return false; }

   protected:
    QMainWindow *window;
};

class MockedBrightnessModule : public BrightnessModule {
    Q_OBJECT

   public:
    MockedBrightnessModule(QMainWindow *window) : BrightnessModule(window) {}
    void set_brightness(int brightness);
    bool update_androidauto() { return true; }
};

// class RpiOfficialBrightnessModule : public BrightnessModule {
//     Q_OBJECT
// };

class XBrightnessModule : public BrightnessModule {
    Q_OBJECT

   public:
    XBrightnessModule(QMainWindow *window);
    void set_brightness(int brightness);

   private:
    QScreen *screen;
};

#endif
