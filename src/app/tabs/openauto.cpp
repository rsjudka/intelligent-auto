#include <app/config.hpp>
#include <app/tabs/openauto.hpp>
#include <app/theme.hpp>
#include <app/widgets/ip_input.hpp>
#include <app/widgets/progress.hpp>
#include <app/window.hpp>

OpenAutoWorker::OpenAutoWorker(std::function<void(bool)> callback, QWidget *parent)
    : io_service(),
      work(io_service),
      configuration(Config::get_instance()->openauto_config),
      recent_addresses_list(7),
      tcp_wrapper(),
      usb_wrapper((libusb_init(&usb_context), usb_context)),
      query_factory(usb_wrapper, io_service),
      query_chain_factory(usb_wrapper, io_service, query_factory),
      service_factory(io_service, configuration, parent, callback),
      android_auto_entity_factory(io_service, configuration, service_factory),
      usb_hub(std::make_shared<aasdk::usb::USBHub>(usb_wrapper, io_service, query_chain_factory)),
      connected_accessories_enumerator(
          std::make_shared<aasdk::usb::ConnectedAccessoriesEnumerator>(usb_wrapper, io_service, query_chain_factory)),
      app(std::make_shared<autoapp::App>(io_service, usb_wrapper, tcp_wrapper, android_auto_entity_factory, usb_hub,
                                         connected_accessories_enumerator)),
      socket(std::make_shared<boost::asio::ip::tcp::socket>(io_service))
{
    this->create_usb_workers();
    this->create_io_service_workers();

    this->recent_addresses_list.read();
}

OpenAutoWorker::~OpenAutoWorker()
{
    std::for_each(this->thread_pool.begin(), this->thread_pool.end(),
                  std::bind(&std::thread::join, std::placeholders::_1));
    libusb_exit(this->usb_context);
}

const QStringList OpenAutoWorker::get_recent_addresses()
{
    QStringList addresses;
    for (auto &address : this->recent_addresses_list.getList()) addresses.append(QString::fromStdString(address));

    return addresses;
}

void OpenAutoWorker::connect_wireless(std::string address)
{
    std::cout << address << std::endl;

    try {
        this->tcp_wrapper.asyncConnect(*this->socket, address, 5277, [this](const boost::system::error_code &ec) {
            std::cout << "in handler\n";
            if (!ec) {
                std::cout << "connected\n";
                this->app->start(socket);
                // emit wireless_connected(std::move(socket));
            }
            else {
                std::cout << "handler: " << ec.message();
                // emit wireless_failed(QString::fromStdString(ec.message()));
            }
            std::cout << "exit handler\n";
        });
    }
    catch (const boost::system::system_error &se) {
        qDebug() << "catch: " << QString(se.what());
        // emit wireless_failed(QString(se.what()));
    }
}

void OpenAutoWorker::create_usb_workers()
{
    auto worker = [this]() {
        timeval event_timeout = {180, 0};
        while (!this->io_service.stopped())
            libusb_handle_events_timeout_completed(this->usb_context, &event_timeout, nullptr);
    };

    this->thread_pool.emplace_back(worker);
    this->thread_pool.emplace_back(worker);
    this->thread_pool.emplace_back(worker);
    this->thread_pool.emplace_back(worker);
}

void OpenAutoWorker::create_io_service_workers()
{
    auto worker = [this]() { this->io_service.run(); };

    this->thread_pool.emplace_back(worker);
    this->thread_pool.emplace_back(worker);
    this->thread_pool.emplace_back(worker);
    this->thread_pool.emplace_back(worker);
}

OpenAutoFrame::OpenAutoFrame(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    QWidget *frame = new QWidget(this);
    frame->setStyleSheet("background-color: rgb(0, 0, 0);");
    layout->addWidget(frame);
}

void OpenAutoFrame::mouseDoubleClickEvent(QMouseEvent *)
{
    this->toggle_fullscreen();
    emit double_clicked(this->fullscreen);
}

OpenAutoTab::OpenAutoTab(QWidget *parent) : QWidget(parent)
{
    MainWindow *window = qobject_cast<MainWindow *>(parent);
    connect(window, &MainWindow::set_openauto_state, [this](unsigned int alpha) {
        if (this->worker != nullptr) this->worker->set_opacity(alpha);
        if (alpha > 0) this->setFocus();
    });

    QStackedLayout *layout = new QStackedLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    OpenAutoFrame *frame = new OpenAutoFrame(this);
    connect(frame, &OpenAutoFrame::toggle, [=](bool enable) {
        if (!enable && frame->is_fullscreen()) {
            window->unset_widget();
            window->remove_widget(frame);
            layout->addWidget(frame);
            layout->setCurrentIndex(1);
            frame->toggle_fullscreen();
            if (this->worker != nullptr) this->worker->resize();
        }
        layout->setCurrentIndex(enable ? 1 : 0);
    });
    connect(frame, &OpenAutoFrame::double_clicked, [=](bool fullscreen) {
        if (fullscreen) {
            layout->setCurrentIndex(0);
            layout->removeWidget(frame);
            window->add_widget(frame);
            window->set_widget();
        }
        else {
            window->unset_widget();
            window->remove_widget(frame);
            layout->addWidget(frame);
            layout->setCurrentIndex(1);
        }
        if (this->worker != nullptr) this->worker->resize();
        frame->setFocus();
    });

    connect(window, &MainWindow::is_ready, [this, layout, frame, opacity = window->windowOpacity()]() {
        frame->resize(this->size());
        auto callback = [frame](bool is_active) {
            frame->toggle(is_active);
            frame->setFocus();
        };
        if (this->worker == nullptr) this->worker = new OpenAutoWorker(callback, frame);
        this->worker->set_opacity(opacity * 255);

        layout->addWidget(this->msg_widget());
        layout->addWidget(frame);

        this->worker->start();
    });
}

QWidget *OpenAutoTab::msg_widget()
{
    Config *config = Config::get_instance();

    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);

    QLabel *label = new QLabel("connect device to start OpenAuto", widget);
    label->setFont(Theme::font_16);
    label->setAlignment(Qt::AlignCenter);

    QWidget *connection = this->wireless_widget();
    if (!config->get_wireless_active()) connection->hide();

    QCheckBox *wireless_button = new QCheckBox("Wireless", widget);
    wireless_button->setFont(Theme::font_14);
    wireless_button->setStyleSheet(QString("QCheckBox::indicator {"
                                           "    width: %1px;"
                                           "    height: %1px;"
                                           "}")
                                       .arg(Theme::font_14.pointSize()));
    wireless_button->setChecked(config->get_wireless_active());
    connect(wireless_button, &QCheckBox::toggled, [config, connection](bool checked) {
        checked ? connection->show() : connection->hide();
        config->set_wireless_active(checked);
    });

    layout->addStretch();
    layout->addWidget(label, 1);
    layout->addWidget(connection, 1);
    layout->addStretch();
    layout->addWidget(wireless_button, 1, Qt::AlignLeft);

    return widget;
}

QWidget *OpenAutoTab::wireless_widget()
{
    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);

    QStringList addresses;
    if (this->worker != nullptr) addresses = this->worker->get_recent_addresses();

    IpInput *ip_input = new IpInput(addresses, QFont("Titillium Web", 24), widget);

    layout->addWidget(ip_input);
    layout->addWidget(this->connect_widget());

    connect(this, &OpenAutoTab::connect_wireless, [this, ip_input]() {
        ip_input->setEnabled(false);
        if (this->worker != nullptr) this->worker->connect_wireless(ip_input->active_address().toStdString());
    });

    return widget;
}

QWidget *OpenAutoTab::connect_widget()
{
    QWidget *widget = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(widget);

    ProgressIndicator *loader = new ProgressIndicator(widget);

    QPushButton *button = new QPushButton("connect", widget);
    button->setFont(Theme::font_14);
    button->setFlat(true);
    button->setIconSize(Theme::icon_36);
    Theme::get_instance()->add_button_icon("wifi", button);

    connect(button, &QPushButton::clicked, [this, button, loader]() {
        button->setEnabled(false);
        loader->start_animation();
        emit connect_wireless();
    });

    layout->addStretch(2);
    layout->addWidget(button, 1);
    layout->addWidget(loader, 1);
    layout->addStretch(1);

    return widget;
}
