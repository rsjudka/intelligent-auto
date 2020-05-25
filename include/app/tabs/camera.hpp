#ifndef CAMERA_HPP_
#define CAMERA_HPP_

#include <QMediaPlayer>
#include <QVideoWidget>
#include <QLabel>

#include <ui_camera_settings.h>

class CameraTab : public QWidget {
    Q_OBJECT

   public:
    CameraTab(QWidget *parent = nullptr);

   signals:
    void media_status_changed(QString statusPrettyName);

   public slots:
    void connect_stream();
    void disconnect_stream();
    void on_camName_changed(QString name);
    void on_toggle_connect();

   private slots:
    void changed_status(QMediaPlayer::MediaStatus);

   private:
     QLabel* status;
     QMediaPlayer* player;
     QVideoWidget* videoWidget;
     QLabel *cameraName;

     QSettings *settings;
     Ui::CameraSettings ui;
};

#endif
