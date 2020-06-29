#include <QLineEdit>
#include <QCamera>
#include <QCameraInfo>

#include <app/tabs/camera.hpp>
#include <app/window.hpp>

CameraTab::CameraTab(QWidget *parent) : QWidget(parent)
{
    this->theme = Theme::get_instance();
    this->player = new QMediaPlayer(this);
    this->local_cam = nullptr;

    this->config = Config::get_instance();
    this->url = this->config->get_cam_stream_url();
    this->local_device = this->config->get_cam_local_device();
    this->local = this->config->get_cam_is_local();

    QStackedLayout *layout = new QStackedLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    layout->addWidget(this->connect_widget());
    layout->addWidget(this->local_camera_widget());
    layout->addWidget(this->network_camera_widget());

    connect(this, &CameraTab::connected_network, [layout]() { layout->setCurrentIndex(2); });
    connect(this, &CameraTab::connected_local, [layout]() { layout->setCurrentIndex(1); });
    connect(this, &CameraTab::disconnected, [layout]() { layout->setCurrentIndex(0); });
}

QWidget *CameraTab::network_camera_widget()
{
    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    QPushButton *disconnect = new QPushButton(widget);
    disconnect->setFlat(true);
    disconnect->setIconSize(Theme::icon_16);
    connect(disconnect, &QPushButton::clicked, [this]() {
        this->player->setMedia(QUrl());
        this->status->setText(QString());
        this->player->stop();
    });
    this->theme->add_button_icon("close", disconnect);
    layout->addWidget(disconnect, 0, Qt::AlignRight);

    QVideoWidget *video = new QVideoWidget(widget);
    this->player->setVideoOutput(video);
    layout->addWidget(video);

    return widget;
}

QWidget *CameraTab::local_camera_widget()
{
    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    QPushButton *disconnect = new QPushButton(widget);
    disconnect->setFlat(true);
    disconnect->setIconSize(Theme::icon_16);
    connect(disconnect, &QPushButton::clicked, [this]() {
        this->status->setText(QString());
        this->local_cam->stop();
        this->local_cam->unload();
        this->local_cam->deleteLater();
        this->local_cam = nullptr;
    });
    this->theme->add_button_icon("close", disconnect);
    layout->addWidget(disconnect, 0, Qt::AlignRight);

    this->local_video_widget = new QVideoWidget(widget);
    layout->addWidget(this->local_video_widget);

    return widget;
}

QWidget *CameraTab::connect_widget()
{
    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);

    QLabel *label = new QLabel("connect camera", widget);
    label->setFont(Theme::font_16);

    this->status = new QLabel(widget);
    this->status->setFont(Theme::font_16);

    QPushButton *connect_button = new QPushButton("connect", widget);
    connect_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    connect_button->setFont(Theme::font_14);
    connect_button->setFlat(true);
    connect_button->setIconSize(Theme::icon_36);
    connect(connect_button, &QPushButton::clicked, [this]() {
        this->status->setText("");
        this->config->set_cam_is_local(this->local);
        if (this->local) {
            this->connect_local_stream();
            this->config->set_cam_local_device(this->local_device);
        }
        else {
            this->connect_network_stream();
            this->config->set_cam_stream_url(this->url);
        }
    });

    layout->addStretch();
    layout->addWidget(label, 0, Qt::AlignCenter);
    layout->addStretch();
    layout->addLayout(this->camera_type_selector());
    layout->addWidget(this->status, 0, Qt::AlignCenter);
    layout->addWidget(connect_button, 0, Qt::AlignCenter);
    layout->addStretch();

    return widget;
}

QGridLayout* CameraTab::camera_type_selector()
{
    QGridLayout *layout = new QGridLayout();
    layout->setColumnStretch(0,1);
    layout->setColumnStretch(2,2);
    layout->setColumnStretch(4,1);

    QRadioButton* local_cam_radio = new QRadioButton("Local", this);
    local_cam_radio->setFont(Theme::font_18);
    layout->addWidget(local_cam_radio,0,1);

    this->cams_dropdown = new QComboBox(this);
    this->cams_dropdown->setFont(Theme::font_18);
    bool has_local = this->populate_local_cams();
    connect(local_cam_radio, &QRadioButton::toggled, [=](bool checked){
        this->local=checked;
        this->status->setText("");
    });
    connect(this->cams_dropdown, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index){
        this->local_device=this->cams_dropdown->itemData(index).toString();
        this->status->setText("");
    });
    layout->addWidget(this->cams_dropdown,0,2);

    QRadioButton* network_cam_radio = new QRadioButton("Network", this);
    network_cam_radio->setFont(Theme::font_18);
    layout->addWidget(network_cam_radio,1,1);
    layout->addWidget(this->input_widget(network_cam_radio),1,2);

    QPushButton *refresh = new QPushButton();
    refresh->setFont(Theme::font_14);
    refresh->setFlat(true);
    this->theme->add_button_icon("refresh", refresh);
    layout->addWidget(refresh,0,3);

    if (has_local && this->config->get_cam_is_local())
        local_cam_radio->setChecked(true);
    else
        network_cam_radio->setChecked(true);

    connect(refresh, &QPushButton::clicked, [=](){ local_cam_radio->setChecked(true); });
    connect(refresh, &QPushButton::clicked, this, &CameraTab::populate_local_cams);
    connect(refresh, &QPushButton::clicked, this, [=](){ local_cam_radio->setChecked(true); });

    return layout;
}

bool CameraTab::populate_local_cams()
{
    this->cams_dropdown->clear();

    const QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    for (const QCameraInfo &cameraInfo : cameras) {
        QString pretty_name = cameraInfo.description() + " at " + cameraInfo.deviceName();
        this->cams_dropdown->addItem(pretty_name, cameraInfo.deviceName());
    }

    if (this->cams_dropdown->count() == 0) {
        this->cams_dropdown->addItem("<No local cameras found>");
        this->cams_dropdown->setDisabled(true);
        return false;
    }

    if (!this->local_device.isEmpty()) {
        int pref_cam_index = this->cams_dropdown->findData(this->local_device);
        if (pref_cam_index >= 0)
            this->cams_dropdown->setCurrentIndex(pref_cam_index);
    }
    else {
        QCameraInfo default_cam = QCameraInfo::defaultCamera();
        if (!default_cam.isNull()) {
            int default_cam_index = this->cams_dropdown->findData(default_cam.deviceName());
            if (default_cam_index >= 0)
                this->cams_dropdown->setCurrentIndex(default_cam_index);
        }
    }

    this->cams_dropdown->setDisabled(false);
    return true;
}

QWidget *CameraTab::input_widget(QRadioButton* network_radio)
{
    QWidget *widget = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(widget);

    QLineEdit *input = new QLineEdit(this->url, widget);
    input->setContextMenuPolicy(Qt::NoContextMenu);
    input->setFont(Theme::font_18);
    input->setAlignment(Qt::AlignLeft);
    connect(input, &QLineEdit::textEdited, [this](QString text) { this->url = text; });
    connect(input, &QLineEdit::returnPressed, [this]() {
        this->connect_network_stream();
        this->config->set_cam_stream_url(this->url);
    });

    layout->addWidget(input, 4);

    connect(input, &QLineEdit::textEdited, [=](){ network_radio->setChecked(true); });

    return widget;
}

void CameraTab::update_network_status(QMediaPlayer::MediaStatus media_status)
{
    qInfo() << "camera status changed to: " << media_status;

    switch (media_status) {
        case QMediaPlayer::LoadingMedia:
        case QMediaPlayer::LoadedMedia:
        case QMediaPlayer::BufferedMedia:
            this->status->setText("connecting...");
            break;
        default:
            this->status->setText("connection failed");
            emit disconnected();
            break;
    }
}

void CameraTab::connect_network_stream()
{
    connect(this->player, &QMediaPlayer::mediaStatusChanged,
            [this](QMediaPlayer::MediaStatus media_status) { this->update_network_status(media_status); });
    connect(this->player, QOverload<>::of(&QMediaPlayer::metaDataChanged), [this]() { emit connected_network(); });

    QString pipeline = QString(
                           "gst-pipeline: rtspsrc location=%1 ! decodebin ! video/x-raw ! videoconvert ! videoscale ! "
                           "xvimagesink sync=false force-aspect-ratio=false name=\"qtvideosink\"")
                           .arg(this->url);

    qInfo() << "playing stream pipeline: " << pipeline;
    this->player->setMedia(QUrl(pipeline));
    this->player->play();
}

void CameraTab::connect_local_stream()
{
    if (this->local_cam != nullptr)
        delete this->local_cam;

    if (!this->local_cam_available(this->local_device)) {
        this->status->setText("Camera unavailable");
        return;
    }

    qDebug() << "Connecting to local cam " << this->local_device;
    this->local_cam = new QCamera(this->local_device.toUtf8(), this);
    this->local_cam->setViewfinder(this->local_video_widget);

    connect(this->local_cam, &QCamera::statusChanged, this, &CameraTab::update_local_status);
    this->local_cam->start();                                                                                                              }

bool CameraTab::local_cam_available(QString& device)
{
    if (device.isEmpty())
        return false;

    const QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    for (const QCameraInfo &cameraInfo : cameras) {
        if (cameraInfo.deviceName() == device)
            return true;
    }
    return false;
}

void CameraTab::update_local_status(QCamera::Status status)
{
    qDebug() << "Local camera" << this->local_device << "changed status to" << status;

    switch (status) {
      case QCamera::ActiveStatus:
        emit connected_local();
        break;
      case QCamera::LoadingStatus:
      case QCamera::LoadedStatus:
      case QCamera::StartingStatus:
        this->status->setText("connecting...");
        break;
      case QCamera::UnloadedStatus:
        emit disconnected();
        break;
      default:
        break;
    }

    if (this->local_cam != nullptr && !this->local_cam->error() == QCamera::NoError) {
        qCritical() << "Local camera" << this->local_cam << "got error" << this->local_cam->error();
        this->status->setText("Error connecting to local camera at '" + this->local_device + "'");
    }
}
