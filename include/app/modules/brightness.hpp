#ifndef BRIGHTNESS_HPP_
#define BRIGHTNESS_HPP_

#include <QObject>
#include <QMainWindow>
#include <QScreen>

class BrightnessModule : public QObject {
    Q_OBJECT

   public:
    BrightnessModule(QMainWindow *window);

    virtual int get_brightness() = 0;
    virtual void set_brightness(int brightness) = 0;
    bool do_update_androidauto() { return this->update_androidauto; }

   protected:
    QMainWindow *window;

   private:
    bool update_androidauto = false;
};

class MockedBrightnessModule : public BrightnessModule {
    Q_OBJECT

    public:
    MockedBrightnessModule(QMainWindow *window) : BrightnessModule(window) {}
    int get_brightness();
    void set_brightness(int brightness);

   private:
    bool update_androidauto = true;
};

// class RpiOfficialBrightnessModule : public BrightnessModule {
//     Q_OBJECT
// };

class XBrightnessModule : public BrightnessModule {
    Q_OBJECT

   public:
    XBrightnessModule(QMainWindow *window);
    int get_brightness();
    void set_brightness(int brightness);

   private:
    QScreen *screen;
};

#endif
