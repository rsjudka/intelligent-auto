#ifndef MEDIA_HPP_
#define MEDIA_HPP_

#include <QMediaPlayer>
#include <QtWidgets>

#include <app/bluetooth.hpp>
#include <app/config.hpp>
#include <app/theme.hpp>
#include <app/widgets/tuner.hpp>

class MediaTab : public QTabWidget {
    Q_OBJECT

   public:
    MediaTab(QWidget *parent = nullptr);
};

class BluetoothPlayerSubTab : public QWidget {
    Q_OBJECT

   public:
    BluetoothPlayerSubTab(QWidget *parent = nullptr);

   private:
    QWidget *track_widget();
    QWidget *controls_widget();

    Bluetooth *bluetooth;
};

class RadioPlayerSubTab : public QWidget {
    Q_OBJECT

   public:
    RadioPlayerSubTab(QWidget *parent = nullptr);

   private:
    QWidget *tuner_widget();
    QWidget *controls_widget();

    Config *config;
    Theme *theme;
    Tuner *tuner;
};

class LocalPlayerSubTab : public QWidget {
    Q_OBJECT

   public:
    LocalPlayerSubTab(QWidget *parent = nullptr);

    static QString durationFmt(int total_ms);

   private:
    QWidget *tracks_widget();
    QWidget *seek_widget();
    QWidget *controls_widget();

    Theme *theme;
    QMediaPlayer *player;
    QLabel *path_label;
};

#endif
