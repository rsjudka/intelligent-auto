#include <QFrame>
#include <QGridLayout>
#include <QKeyEvent>
#include <QPropertyAnimation>
#include <QTimer>

#include <app/widgets/dialog.hpp>

Dialog::Dialog(bool fullscreen, QWidget *parent) : QDialog(parent, Qt::FramelessWindowHint)
{
    this->setAttribute(Qt::WA_TranslucentBackground, true);

    this->fullscreen = fullscreen;
    if (false) {
        if (this->fullscreen) {
            this->resize(parent->size());
            Overlay *overlay = new Overlay(this);
            overlay->resize(parent->size());
            connect(overlay, &Overlay::close, [this]() { this->close(); });
            this->overlay_enabled = true;
        }
    }

    QGridLayout *layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(this->content_widget());
}

void Dialog::open(int timeout)
{
    if (this->fullscreen) {
        this->exec();
    }
    else {
        this->show();
        if (timeout > 0) QTimer::singleShot(timeout, [this]() { this->close(); });
    }
}

QWidget *Dialog::content_widget()
{
    QFrame *frame = new QFrame(this);
    frame->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QVBoxLayout *layout = new QVBoxLayout(frame);

    this->title = new QVBoxLayout();
    layout->addLayout(this->title);

    this->body = new QVBoxLayout();
    layout->addLayout(this->body);

    this->buttons = new QHBoxLayout();
    if (!this->overlay_enabled && this->fullscreen) this->add_cancel_button();
    layout->addLayout(this->buttons);

    return frame;
}

void Dialog::set_position()
{
    if (QWidget *parent = this->parentWidget()) {
        QPoint point;
        if (this->fullscreen) {
            point = parent->geometry().center() - this->rect().center();
        }
        else {
            QWidget *window = parent->window();
            QPoint center = parent->mapToGlobal(parent->rect().center());

            QPoint pivot;
            if (center.y() > (window->height() / 2)) {
                pivot = (center.x() > (window->width() / 2)) ? this->rect().bottomRight() : this->rect().bottomLeft();
                pivot.ry() += parent->height() / 2;
            }
            else {
                pivot = (center.x() > (window->width() / 2)) ? this->rect().topRight() : this->rect().topLeft();
                pivot.ry() -= parent->height() / 2;
            }
            point = this->mapFromGlobal(center) - pivot;
        }
        this->move(point);
    }
}

void Dialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() != Qt::Key_Escape || this->fullscreen) QDialog::keyPressEvent(event);
}

void Dialog::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    this->set_position();
}
