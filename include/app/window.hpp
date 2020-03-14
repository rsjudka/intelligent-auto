#ifndef WINDOW_HPP_
#define WINDOW_HPP_

#include <stdlib.h>
#include <QBluetoothDeviceInfo>
#include <QBluetoothServiceDiscoveryAgent>
#include <QBluetoothServiceInfo>
#include <QMainWindow>
#include <QStringList>
#include <QtDBus/QtDBus>
#include <QtWidgets>

#include <app/config.hpp>
#include <app/tabs/openauto.hpp>
#include <app/theme.hpp>

class MainWindow : public QMainWindow {
    Q_OBJECT

   public:
    MainWindow();

    void add_widget(QWidget *widget);
    void remove_widget(QWidget *widget);

   protected:
    void showEvent(QShowEvent *event);

   private:
    static void update_system_volume(int position);
    QWidget *window_widget();
    QTabWidget *tabs_widget();
    QWidget *controls_widget();
    QWidget *volume_widget();

    Config *config;
    Theme *theme;
    QStackedLayout *layout;

   signals:
    void is_ready();
    void set_data_state(bool enabled);
    void set_openauto_state(unsigned int alpha);
};

#endif
