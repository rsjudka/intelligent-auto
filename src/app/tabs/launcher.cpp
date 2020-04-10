#include <app/tabs/launcher.hpp>

EmbeddedApp::WindowProp::WindowProp(char *prop, uint64_t size)
{
    this->size = size;
    this->prop = new char[this->size + 1];

    std::copy(prop, prop + size, (char *)this->prop);
    ((char *)this->prop)[size] = '\0';
}

EmbeddedApp::WindowProp::~WindowProp()
{
    if (this->prop != nullptr) {
        delete (char *)this->prop;
        this->prop = nullptr;
    }
}

EmbeddedApp::EmbeddedApp(QWidget *parent) : QWidget(parent)
{
    this->display = XOpenDisplay(nullptr);
    this->root_window = DefaultRootWindow(this->display);

    this->process = new QProcess();
    process->setStandardOutputFile(QProcess::nullDevice());
    process->setStandardErrorFile(QProcess::nullDevice());

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    this->container = new QVBoxLayout();

    layout->addWidget(this->controls_widget(), 0, Qt::AlignTop | Qt::AlignRight);
    layout->addLayout(this->container, 1);
}

QWidget *EmbeddedApp::controls_widget()
{
    QWidget *widget = new QWidget(this);

    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);

    QPushButton *button = new QPushButton(this);
    button->setFlat(true);
    button->setIconSize(Theme::icon_16);
    connect(button, &QPushButton::clicked, [this]() { this->end(); });
    Theme::get_instance()->add_button_icon("close", button);
    layout->addWidget(button);

    layout->addStretch();
    layout->addWidget(button);

    return widget;
}

void EmbeddedApp::start(QString app)
{
    this->process->setProgram(app);
    this->process->start();

    this->process->waitForStarted();
    sleep(2);

    QWindow *window = QWindow::fromWinId(this->get_window());
    window->setFlags(Qt::FramelessWindowHint);
    sleep(2);

    this->container->addWidget(QWidget::createWindowContainer(window, this));

    emit opened();
}

void EmbeddedApp::end()
{
    this->process->terminate();
    delete this->container->takeAt(0);
    emit closed();
}

EmbeddedApp::WindowProp EmbeddedApp::get_window_prop(Window window, Atom type, const char *name)
{
    Atom prop = XInternAtom(this->display, name, false);

    Atom actual_type_return;
    int32_t actual_format_return;
    uint64_t nitems_return, bytes_after_return;
    uint8_t *prop_return;
    XGetWindowProperty(this->display, window, prop, 0, 1024, false, type, &actual_type_return, &actual_format_return,
                       &nitems_return, &bytes_after_return, &prop_return);

    uint64_t size = (actual_format_return / 8) * nitems_return;
    if (actual_format_return == 32) size *= sizeof(long) / 4;

    WindowProp winProp((char *)prop_return, size);
    XFree(prop_return);

    return winProp;
}

std::list<Window> EmbeddedApp::get_clients()
{
    std::list<Window> windows;

    WindowProp prop = this->get_window_prop(this->root_window, XA_WINDOW, "_NET_CLIENT_LIST");
    Window *window_list = (Window *)prop.prop;
    for (uint64_t i = 0; i < prop.size / sizeof(Window); i++) windows.push_back(window_list[i]);

    return windows;
}

int EmbeddedApp::get_window()
{
    uint64_t pid;
    for (auto client : this->get_clients()) {
        pid = *(uint64_t *)this->get_window_prop(client, XA_CARDINAL, "_NET_WM_PID").prop;
        if ((uint64_t)this->process->processId() == pid) {
            return client;
        }
    }

    return -1;
}

LauncherTab::LauncherTab(QWidget *parent) : QWidget(parent)
{
    this->theme = Theme::get_instance();
    this->config = Config::get_instance();

    QStackedLayout *layout = new QStackedLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    this->app = new EmbeddedApp(parent);
    connect(this->app, &EmbeddedApp::opened, [layout]() { layout->setCurrentIndex(1); });
    connect(this->app, &EmbeddedApp::closed, [layout]() { layout->setCurrentIndex(0); });

    layout->addWidget(this->launcher_widget());
    layout->addWidget(this->app);

    if (this->config->get_launcher_auto_launch() && !this->config->get_launcher_app().isEmpty())
        this->app->start(this->config->get_launcher_app());
}

QWidget *LauncherTab::launcher_widget()
{
    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);

    this->path_label = new QLabel(this->config->get_launcher_home(), this);
    this->path_label->setFont(Theme::font_14);

    QCheckBox *auto_launch = new QCheckBox("launch at startup", widget);
    auto_launch->setFont(Theme::font_14);
    auto_launch->setStyleSheet(QString("QCheckBox::indicator {"
                                       "    width: %1px;"
                                       "    height: %1px;"
                                       "}")
                                   .arg(Theme::font_14.pointSize()));
    auto_launch->setChecked(this->config->get_launcher_auto_launch());
    connect(auto_launch, &QCheckBox::toggled, [this](bool checked) {
        this->config->set_launcher_auto_launch(checked);
        QString launcher_app;
        if (checked) {
            launcher_app.append(this->path_label->text() + '/');
            if (this->apps->currentItem() == nullptr)
                launcher_app.append(this->apps->item(0)->text());
            else
                launcher_app.append(this->apps->currentItem()->text());
        }
        this->config->set_launcher_app(launcher_app);
    });

    layout->addStretch(1);
    layout->addWidget(this->path_label, 1);
    layout->addWidget(this->app_select_widget(), 4);
    layout->addWidget(auto_launch, 1, Qt::AlignRight);
    layout->addStretch(1);

    return widget;
}

QWidget *LauncherTab::app_select_widget()
{
    QWidget *widget = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(widget);

    QString root_path(this->config->get_launcher_home());

    QPushButton *home_button = new QPushButton(widget);
    home_button->setFlat(true);
    home_button->setCheckable(true);
    home_button->setChecked(this->config->get_launcher_home() == root_path);
    home_button->setIconSize(Theme::icon_32);
    connect(home_button, &QPushButton::clicked, [this](bool checked = false) {
        this->config->set_launcher_home(checked ? this->path_label->text() : QDir().absolutePath());
    });
    this->theme->add_button_icon("playlist_add_check", home_button, "playlist_add");
    layout->addWidget(home_button, 0, Qt::AlignTop);

    this->folders = new QListWidget(widget);
    Theme::to_touch_scroller(this->folders);
    this->populate_dirs(root_path);
    layout->addWidget(this->folders, 4);

    this->apps = new QListWidget(widget);
    Theme::to_touch_scroller(this->apps);
    this->populate_apps(root_path);
    connect(this->apps, &QListWidget::itemClicked, [this](QListWidgetItem *item) {
        QString app_path = this->path_label->text() + '/' + item->text();
        if (this->config->get_launcher_auto_launch()) this->config->set_launcher_app(app_path);
        this->app->start(app_path);
    });
    connect(this->folders, &QListWidget::itemClicked, [this, home_button](QListWidgetItem *item) {
        if (!item->isSelected()) return;

        this->apps->clear();
        QString current_path(item->data(Qt::UserRole).toString());
        this->path_label->setText(current_path);
        this->populate_apps(current_path);
        this->populate_dirs(current_path);

        home_button->setChecked(this->config->get_launcher_home() == current_path);
    });
    layout->addWidget(this->apps, 5);

    return widget;
}

void LauncherTab::populate_dirs(QString path)
{
    this->folders->clear();
    QDir current_dir(path);
    for (QFileInfo dir : current_dir.entryInfoList(QDir::AllDirs | QDir::Readable)) {
        if (dir.fileName() == ".") continue;

        QListWidgetItem *item = new QListWidgetItem(dir.fileName(), this->folders);
        if (dir.fileName() == "..") {
            item->setText("↲");

            if (current_dir.isRoot()) item->setFlags(Qt::NoItemFlags);
        }
        else {
            item->setText(dir.fileName());
        }
        item->setFont(Theme::font_16);
        item->setData(Qt::UserRole, QVariant(dir.absoluteFilePath()));
    }
}

void LauncherTab::populate_apps(QString path)
{
    for (QString app : QDir(path).entryList(QDir::Files | QDir::Executable)) {
        QListWidgetItem *item = new QListWidgetItem(app, this->apps);
        item->setFont(Theme::font_16);
    }
}
