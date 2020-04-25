#include <QDir>
#include <QElapsedTimer>

#include <app/shortcuts.hpp>

static const QRegExp GPIOX_REGEX("gpio\\d+");
static const QString GPIO_DIR("/home/robert/dummy_gpio");
static const QString GPIOX_DIR(GPIO_DIR + "/%1");
static const QString GPIOX_VALUE_PATH(GPIOX_DIR + "/value");
static const QString GPIOX_ACTIVE_LOW_PATH(GPIOX_DIR + "/value");

GpioWatcher::GpioWatcher(QObject *parent) : QObject(parent)
{
    QStringList gpios;
    for (auto gpio : QDir(GPIO_DIR).entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        if (GPIOX_REGEX.exactMatch(gpio)) gpios.append(GPIOX_VALUE_PATH.arg(gpio));
    }

    this->watcher = new QFileSystemWatcher(gpios, this);
    connect(this->watcher, &QFileSystemWatcher::fileChanged,
            [this](QString path) { emit gpio_triggered(path.split('/')[4]); });

    this->disable();
}

ShortcutInput::ShortcutInput(QString shortcut, QWidget *parent) : QPushButton(shortcut, parent)
{
    this->gpio_watcher = new GpioWatcher(this);
    connect(this->gpio_watcher, &GpioWatcher::gpio_triggered, [this](QString gpio) {
        this->setText(gpio);
        emit shortcut_updated(gpio);
    });

    QElapsedTimer *timer = new QElapsedTimer();
    connect(this, &QPushButton::pressed, [timer]() { timer->start(); });
    connect(this, &QPushButton::released, [this, timer]() {
        if (timer->hasExpired(500)) {
            this->setText(QString());
            emit shortcut_updated(this->text());
        }
    });
}

void ShortcutInput::keyPressEvent(QKeyEvent *event)
{
    Qt::Key k = static_cast<Qt::Key>(event->key());
    if (k == Qt::Key_unknown || k == Qt::Key_Control || k == Qt::Key_Shift || k == Qt::Key_Alt || k == Qt::Key_Meta)
        return;

    QKeySequence shortcut(event->modifiers() + k);
    this->setText(shortcut.toString());
    emit shortcut_updated(this->text());
}

Shortcut::Shortcut(QString shortcut, QWidget *parent) : QObject(parent)
{
    this->shortcut = shortcut;
    this->key = new QShortcut(parent);
    this->gpio = new QFileSystemWatcher(parent);

    this->set_shortcut(shortcut);
    connect(this->gpio, &QFileSystemWatcher::fileChanged, [this](QString path) {
        QFile value_attribute(path);
        value_attribute.open(QIODevice::ReadOnly);
        int gpio_value = value_attribute.read(1).at(0);
        value_attribute.close();
        if (this->gpio_active_low == gpio_value) emit activated();
    });
    connect(this->key, &QShortcut::activated, [this]() { emit activated(); });
}

void Shortcut::set_shortcut(QString shortcut)
{
    QStringList gpios = this->gpio->files();
    if (gpios.size() > 0) this->gpio->removePaths(gpios);
    this->key->setKey(QKeySequence());

    this->shortcut = shortcut;
    if (this->shortcut.startsWith("gpio")) {
        QFile active_low_attribute(GPIOX_ACTIVE_LOW_PATH.arg(this->shortcut));
        active_low_attribute.open(QIODevice::ReadOnly);
        this->gpio_active_low = active_low_attribute.read(1)[0];
        active_low_attribute.close();
        this->gpio->addPath(GPIOX_VALUE_PATH.arg(this->shortcut));
    }
    else {
        this->key->setKey(QKeySequence::fromString(this->shortcut));
    }
}

void Shortcuts::add_shortcut(QString id, QString description, Shortcut *shortcut)
{
    this->shortcuts[id] = {description, shortcut};
    emit shortcut_added(id, description, shortcut);
}

Shortcuts *Shortcuts::get_instance()
{
    static Shortcuts shortcuts;
    return &shortcuts;
}
