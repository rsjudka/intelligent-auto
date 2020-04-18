#include <QElapsedTimer>

#include <app/widgets/shortcut_input.hpp>

ShortcutInput::ShortcutInput(QString shortcut, QWidget *parent) : QPushButton(shortcut, parent)
{
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
    emit shortcut_updated(shortcut);
}
