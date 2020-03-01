#ifndef CONFIG_HPP_
#define CONFIG_HPP_

#include <f1x/openauto/autoapp/Configuration/Configuration.hpp>

#include <QObject>
#include <QSettings>
#include <QString>

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

    std::shared_ptr<f1x::openauto::autoapp::configuration::Configuration> open_auto_config;

    static Config *get_instance();

   private:
    QSettings ia_config;
    int volume;
    bool dark_mode;
    int brightness;
    bool si_units;
    QString color;
    QString bluetooth_device;
    double radio_station;
    bool radio_muted;

   signals:
    void brightness_changed(unsigned int);
    void si_units_changed(bool);
};

#endif
