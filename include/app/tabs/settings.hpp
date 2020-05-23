#ifndef SETTINGS_HPP_
#define SETTINGS_HPP_

#include <QMap>
#include <QtWidgets>

#include <app/bluetooth.hpp>
#include <app/config.hpp>
#include <app/theme.hpp>

#include <ui_camera_settings.h>

class SettingsTab : public QTabWidget {
    Q_OBJECT

   public:
    SettingsTab(QWidget *parent = nullptr);
};

class GeneralSettingsSubTab : public QWidget {
    Q_OBJECT

   public:
    GeneralSettingsSubTab(QWidget *parent = nullptr);

   private:
    QWidget *settings_widget();
    QWidget *dark_mode_row_widget();
    QWidget *brightness_module_row_widget();
    QWidget *brightness_module_select_widget();
    QWidget *brightness_row_widget();
    QWidget *brightness_widget();
    QWidget *si_units_row_widget();
    QWidget *color_row_widget();
    QWidget *color_select_widget();
    QWidget *quick_view_row_widget();
    QWidget *quick_view_select_widget();

    Config *config;
    Theme *theme;
};

class BluetoothSettingsSubTab : public QWidget {
    Q_OBJECT

   public:
    BluetoothSettingsSubTab(QWidget *parent = nullptr);

   private:
    QWidget *controls_widget();
    QWidget *scanner_widget();
    QWidget *devices_widget();

    Bluetooth *bluetooth;
    Config *config;
    Theme *theme;
    QMap<BluezQt::DevicePtr, QPushButton *> devices;
};

class OpenAutoSettingsSubTab : public QWidget {
    Q_OBJECT

   public:
    OpenAutoSettingsSubTab(QWidget *parent = nullptr);

   private:
    QWidget *settings_widget();
    QWidget *rhd_row_widget();
    QWidget *frame_rate_row_widget();
    QWidget *resolution_row_widget();
    QWidget *dpi_row_widget();
    QWidget *dpi_widget();
    QWidget *rt_audio_row_widget();
    QWidget *audio_channels_row_widget();
    QWidget *bluetooth_row_widget();

    Bluetooth *bluetooth;
    Config *config;
    Theme *theme;
};

class CameraSettingsSubTab : public QWidget {
    Q_OBJECT

   public:
    CameraSettingsSubTab(QWidget *parent = nullptr);

   signals:
    void cam_toggle_requested();
	void cam_name_changed(QString);

   public slots:
	void on_newConnectionStatus(QString);

   private slots:
    void on_camName_editingFinished();
    void on_streamAddress_editingFinished();
 	void on_connectButton_clicked();

   private:
    Theme *theme;
	QSettings *settings;
	Ui::CameraSettings ui;


// private slots:
//     void on_test_button_clicked();
};

#endif
