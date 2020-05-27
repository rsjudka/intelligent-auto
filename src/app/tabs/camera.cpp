#include <app/config.hpp>
#include <app/tabs/camera.hpp>
#include <app/window.hpp>
#include <app/theme.hpp>

CameraTab::CameraTab(QWidget *parent) : QWidget(parent)
{
    config = Config::get_instance();

    QVBoxLayout *layout = new QVBoxLayout(this);
    status = new QLabel;
    status->setFont(Theme::font_14);

    videoWidget = new QVideoWidget;
    player = new QMediaPlayer;
    player->setVideoOutput(videoWidget);
    QString camera = config->get_cam_name();
    cameraName = new QLabel(camera, this);
    cameraName->setFont(Theme::font_16);

    layout->addWidget(cameraName);
    layout->addWidget(videoWidget);
    layout->addWidget(status);

    connect(player, &QMediaPlayer::mediaStatusChanged, this, &CameraTab::changed_status);
    connect_stream();
}

void CameraTab::changed_status(QMediaPlayer::MediaStatus mediaStatus)
{
    qInfo() << "Camera status changed to: " << mediaStatus;

    switch(mediaStatus) {
        case QMediaPlayer::InvalidMedia:
            status->setText("Camera disconnected");
            videoWidget->hide();
            break;
         case QMediaPlayer::LoadingMedia:
            status->setText("Connecting...");
            videoWidget->hide();
            break;
         case QMediaPlayer::LoadedMedia:
         case QMediaPlayer::BufferedMedia:
            status->setText("Connected");
            videoWidget->show();
            break;
         case QMediaPlayer::EndOfMedia:
         case QMediaPlayer::StalledMedia:
         case QMediaPlayer::NoMedia:
        default:
            videoWidget->hide();
            status->setText("Disconnected");
            break;
    }

    emit media_status_changed(status->text());
}

void CameraTab::on_camName_changed(QString name)
{
    cameraName->setText(name);
}

void CameraTab::on_toggle_connect()
{
    if (status->text() == "Connected")
        disconnect_stream();
    else
        connect_stream();    
}

void CameraTab::disconnect_stream()
{
    player->setMedia(QUrl());
    player->stop();
}

void CameraTab::connect_stream()
{
    QString streamUrl = config->get_cam_stream_url(); // e.g. "rtsp://10.0.0.185:8554/unicast"
    QString streamPipeline = QString("gst-pipeline: rtspsrc location=%1 ! decodebin ! video/x-raw ! videoconvert ! videoscale ! xvimagesink sync=false force-aspect-ratio=false name=\"qtvideosink\"").arg(streamUrl);
    qInfo() << "Playing stream pipeline: " << streamPipeline;
    player->setMedia(QUrl(streamPipeline));
    player->play();
}
