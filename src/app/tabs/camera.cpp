#include <app/config.hpp>
#include <app/tabs/camera.hpp>
#include <app/window.hpp>
#include <app/theme.hpp>

CameraTab::CameraTab(QWidget *parent) : QWidget(parent)
{
    QStackedLayout *stack = new QStackedLayout(this);
    stack->addWidget(connect_widget());
    stack->addWidget(cam_widget());

    connect(this, &CameraTab::stream_connected, this, [stack]() {stack->setCurrentIndex(1);} );
    connect(this, &CameraTab::stream_disconnected, this, [stack]() {stack->setCurrentIndex(0);} );

    player = NULL;
    // connect_stream(); TODO: offer auto-connect option
}

QWidget* CameraTab::cam_widget()
{
    QWidget* widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);

    QHBoxLayout *connection = new QHBoxLayout;
    status = new QLabel("Disconnected", widget);
    status->setFont(Theme::font_16);
    QPushButton *disconnect = new QPushButton("disconnect");
    disconnect->setFont(Theme::font_14);
    disconnect->setFlat(true);
    disconnect->setIconSize(Theme::icon_36);
    Theme::get_instance()->add_button_icon("wifi", disconnect);
    connection->addWidget(status);
    connection->addStretch();
    connection->addWidget(disconnect);

    connect(disconnect, &QPushButton::clicked, this, &CameraTab::disconnect_stream);

    video_widget = new QVideoWidget;

    layout->addWidget(video_widget);
    layout->addLayout(connection);

    return widget;
}


QWidget *CameraTab::connect_widget()
{
    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);

    QLabel *label = new QLabel("connect camera stream for camera view", widget);
    label->setFont(Theme::font_16);
    label->setAlignment(Qt::AlignCenter);

    Config *config = Config::get_instance();
    QLineEdit *stream_url_input = new QLineEdit(config->get_cam_stream_url(), widget);
    stream_url_input->setFixedWidth( 18*30 );
    stream_url_input->setFont(Theme::font_18);
    stream_url_input->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(stream_url_input, &QLineEdit::editingFinished, this, [config,stream_url_input]() {config->set_cam_stream_url(stream_url_input->text());});

    status2 = new QLabel("", widget);
    status2->setFont(Theme::font_16);
    status2->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QPushButton *button = new QPushButton("connect", widget);
    button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    button->setFont(Theme::font_14);
    button->setFlat(true);
    button->setIconSize(Theme::icon_36);
    Theme::get_instance()->add_button_icon("wifi", button);
    connect(button, &QPushButton::clicked, this, [this, stream_url_input](){this->connect_stream(stream_url_input->text());});

    connect(this, &CameraTab::stream_connecting, this, [button, stream_url_input]() {button->setEnabled(false); stream_url_input->setEnabled(false);} );
    connect(this, &CameraTab::stream_disconnected, this, [button, stream_url_input]() {button->setEnabled(true); stream_url_input->setEnabled(true);} );

    layout->addStretch();
    layout->addWidget(label, 1);
    layout->addStretch();
    layout->addWidget(stream_url_input, 0, Qt::AlignCenter);
    layout->addWidget(status2, 0, Qt::AlignCenter);
    layout->addStretch();
    layout->addWidget(button, 1, Qt::AlignCenter);

    return widget;
}

void CameraTab::changed_status(QMediaPlayer::MediaStatus mediaStatus)
{
    qInfo() << "Camera status changed to: " << mediaStatus;

    switch(mediaStatus)
    {
        case QMediaPlayer::InvalidMedia:
            status2->setText("unable to connect");
            emit stream_disconnected();
            break;
         case QMediaPlayer::LoadingMedia:
         case QMediaPlayer::LoadedMedia:
         case QMediaPlayer::BufferedMedia:
            status2->setText("connecting...");
            emit stream_connecting();
            break;
         case QMediaPlayer::EndOfMedia:
         case QMediaPlayer::StalledMedia:
         case QMediaPlayer::NoMedia:
        default:
            status2->setText("disconnected");
            emit stream_disconnected();
            break;
    }
}

void CameraTab::new_metadata(const QString &key, const QVariant &value)
{
    status->setText("connected");
    emit stream_connected();
}

void CameraTab::disconnect_stream()
{
    player->setMedia(QUrl());
    player->stop();
}

void CameraTab::connect_stream(QString stream_url)
{
    if ( player != NULL )
        delete player;

    player = new QMediaPlayer;
    player->setVideoOutput(video_widget);

    connect(player, &QMediaPlayer::mediaStatusChanged, this, &CameraTab::changed_status);
    connect(player, qOverload<const QString&,const QVariant&>(&QMediaPlayer::metaDataChanged), this, &CameraTab::new_metadata);

    // e.g. "rtsp://10.0.0.185:8554/unicast"
    QString stream_pipeline = QString("gst-pipeline: rtspsrc location=%1 ! decodebin ! video/x-raw ! videoconvert ! videoscale ! xvimagesink sync=false force-aspect-ratio=false name=\"qtvideosink\"").arg(stream_url);
    qInfo() << "Playing stream pipeline: " << stream_pipeline;
    player->setMedia(QUrl(stream_pipeline));
    player->play();
}
