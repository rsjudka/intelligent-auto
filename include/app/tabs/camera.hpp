#ifndef CAMERA_HPP_
#define CAMERA_HPP_

#include <QLabel>
#include <QMediaPlayer>
#include <QStackedLayout>
#include <QString>
#include <QVideoWidget>
#include <QComboBox>
#include <QRadioButton>
#include <QCamera>

#include <app/config.hpp>
#include <app/theme.hpp>

class CameraTab : public QWidget {
    Q_OBJECT

   public:
    CameraTab(QWidget *parent = nullptr);

   private:
    QWidget *connect_widget();
    QWidget *network_camera_widget();
    QWidget *local_camera_widget();
    QGridLayout *camera_type_selector();
    QWidget *input_widget(QRadioButton *network_radio);
    bool populate_local_cams();
    void connect_network_stream();
    void connect_local_stream();
    bool local_cam_available(QString& device);
    void update_network_status(QMediaPlayer::MediaStatus media_status);
    void update_local_status(QCamera::Status status);

    Theme *theme;
    Config *config;
    QLabel *status;
    QMediaPlayer *player;
    QString url;
    QComboBox* cams_dropdown;
    QVideoWidget *local_video_widget;
    QCamera *local_cam;
    QString local_device;
    bool local;

   signals:
    void connected_network();
    void connected_local();
    void disconnected();
};

#endif
