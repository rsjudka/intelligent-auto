#include <app/config.hpp>
#include <app/tabs/openauto.hpp>
#include <app/theme.hpp>
#include <app/window.hpp>

OpenAutoWorker::OpenAutoWorker(std::function<void(bool)> callback, QWidget *parent)
    : io_service(),
      work(io_service),
      configuration(Config::get_instance()->openauto_config),
      tcp_wrapper(),
      usb_wrapper((libusb_init(&this->usb_context), usb_context)),
      query_factory(usb_wrapper, io_service),
      query_chain_factory(usb_wrapper, io_service, query_factory),
      service_factory(io_service, configuration, parent, callback),
      android_auto_entity_factory(io_service, configuration, service_factory),
      usb_hub(std::make_shared<aasdk::usb::USBHub>(this->usb_wrapper, this->io_service, this->query_chain_factory)),
      connected_accessories_enumerator(std::make_shared<aasdk::usb::ConnectedAccessoriesEnumerator>(
          this->usb_wrapper, this->io_service, this->query_chain_factory)),
      app(std::make_shared<autoapp::App>(this->io_service, this->usb_wrapper, this->tcp_wrapper,
                                         this->android_auto_entity_factory, usb_hub, connected_accessories_enumerator))
{
    this->create_usb_workers();
    this->create_io_service_workers();
}

OpenAutoWorker::~OpenAutoWorker()
{
    std::for_each(this->thread_pool.begin(), this->thread_pool.end(),
                  std::bind(&std::thread::join, std::placeholders::_1));
    libusb_exit(this->usb_context);
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
    this->resize(parent->size());

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    QWidget *frame = new QWidget(this);
    frame->setStyleSheet("background-color: rgb(0, 0, 0);");
    layout->addWidget(frame);
}

void OpenAutoFrame::mouseDoubleClickEvent(QMouseEvent *)
{
    this->fullscreen = !this->fullscreen;
    emit toggle_fullscreen(this->fullscreen);
}

OpenAutoTab::OpenAutoTab(QWidget *parent) : QWidget(parent)
{
    MainWindow *window = qobject_cast<MainWindow *>(parent);

    QStackedLayout *layout = new QStackedLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    connect(window, &MainWindow::is_ready, [this, window, layout]() {
        OpenAutoFrame *frame = new OpenAutoFrame(this);
        layout->addWidget(frame);

        auto callback = [this, layout, window, frame](bool is_active) {
            if (!is_active && frame->is_fullscreen()) {
                window->remove_widget(frame);
                layout->addWidget(frame);
                if (this->worker != nullptr) this->worker->resize(true);
            }
            layout->setCurrentIndex(is_active ? 1 : 0);
            frame->setFocus();
        };
        if (this->worker == nullptr) this->worker = new OpenAutoWorker(callback, frame);

        connect(window, &MainWindow::set_openauto_state, [this, frame](unsigned int alpha) {
            if (this->worker != nullptr) this->worker->set_opacity(alpha);
            if (alpha > 0) frame->setFocus();
        });

        connect(frame, &OpenAutoFrame::toggle_fullscreen, [layout, frame, window, worker = this->worker](bool enable) {
            if (enable) {
                layout->removeWidget(frame);
                window->add_widget(frame);
            }
            else {
                window->remove_widget(frame);
                layout->addWidget(frame);
                layout->setCurrentIndex(1);
            }
            if (worker != nullptr) worker->resize();
            frame->setFocus();
        });

        this->worker->start();
    });
    layout->addWidget(this->msg_widget());
}

QWidget *OpenAutoTab::msg_widget()
{
    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setContentsMargins(24, 24, 24, 24);

    QLabel *top_msg = new QLabel("waiting for device...", widget);
    top_msg->setFont(Theme::font_16);
    top_msg->setAlignment(Qt::AlignHCenter);
    layout->addStretch();
    layout->addWidget(top_msg);

    QLabel *bottom_msg = new QLabel("plug in your device to start OpenAuto", widget);
    bottom_msg->setFont(Theme::font_16);
    bottom_msg->setAlignment(Qt::AlignHCenter);
    layout->addWidget(bottom_msg);
    layout->addStretch();

    return widget;
}
