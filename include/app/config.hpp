#ifndef CONFIG_HPP_
#define CONFIG_HPP_

#include <f1x/openauto/autoapp/Configuration/Configuration.hpp>

#include <QObject>
#include <QWidget>
#include <QFrame>
#include <QSettings>
#include <QString>

#include <app/modules/brightness.hpp>

class Config : public QObject {
    Q_OBJECT

   public:
    Config();

    void save();

    inline int get_volume() { return this->volume; }
    inline void set_volume(int volume) { this->volume = volume; }

    inline bool get_dark_mode() { return this->dark_mode; }
    inline void set_dark_mode(bool dark_mode) { this->dark_mode = dark_mode; }

    inline int get_brightness() { return this->brightness; }
    inline void set_brightness(int brightness)
    {
        this->brightness = brightness;
        emit brightness_changed(this->brightness);
    }

    inline bool get_si_units() { return this->si_units; }
    inline void set_si_units(bool si_units)
    {
        this->si_units = si_units;
        emit si_units_changed(this->si_units);
    }

    inline QString get_color() { return this->color; }
    inline void set_color(QString color) { this->color = color; }

    inline QString get_bluetooth_device() { return this->bluetooth_device; }
    inline void set_bluetooth_device(QString bluetooth_device) { this->bluetooth_device = bluetooth_device; }

    inline double get_radio_station() { return this->radio_station; }
    inline void set_radio_station(double radio_station) { this->radio_station = radio_station; }

    inline bool get_radio_muted() { return this->radio_muted; }
    inline void set_radio_muted(bool radio_muted) { this->radio_muted = radio_muted; }

    inline QString get_media_home() { return this->media_home; }
    inline void set_media_home(QString media_home) { this->media_home = media_home; }

    inline bool get_wireless_active() { return this->wireless_active; }
    inline void set_wireless_active(bool wireless_active) { this->wireless_active = wireless_active; }

    inline QString get_wireless_address() { return this->wireless_address; }
    inline void set_wireless_address(QString wireless_address) { this->wireless_address = wireless_address; }

    inline QString get_quick_view() { return this->quick_view; }
    inline void set_quick_view(QString quick_view)
    {
        this->quick_view = quick_view;
        emit quick_view_changed(this->quick_view);
    }
    inline QMap<QString, QWidget *> get_quick_views() { return this->quick_views; }
    inline QWidget *get_quick_view(QString name) { return this->quick_views[name]; }
    inline void add_quick_view(QString name, QWidget *view) { this->quick_views[name] = view; }

    inline QString get_brightness_module() { return this->brightness_module; }
    inline void set_brightness_module(QString brightness_module) { this->brightness_module = brightness_module; }
    inline QMap<QString, BrightnessModule *> get_brightness_modules() { return this->brightness_modules; }
    inline BrightnessModule *get_brightness_module(QString name) { return this->brightness_modules[name]; }
    inline void add_brightness_module(QString name, BrightnessModule *module)
    {
        this->brightness_modules[name] = module;
    }

    std::shared_ptr<f1x::openauto::autoapp::configuration::Configuration> openauto_config;

    static Config *get_instance();

   private:
    QMap<QString, QWidget *> quick_views;
    QMap<QString, BrightnessModule *> brightness_modules;

    QSettings ia_config;
    int volume;
    bool dark_mode;
    int brightness;
    bool si_units;
    QString color;
    QString bluetooth_device;
    double radio_station;
    bool radio_muted;
    QString media_home;
    bool wireless_active;
    QString wireless_address;
    QString quick_view;
    QString brightness_module;

   signals:
    void brightness_changed(unsigned int brightness);
    void si_units_changed(bool si_units);
    void quick_view_changed(QString quick_view);
};

#endif
