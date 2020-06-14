#include <QFrame>
#include <QGridLayout>
#include <QKeyEvent>
#include <QPropertyAnimation>
#include <QTimer>

#include <app/widgets/dialog.hpp>

Dialog::Dialog(bool use_backdrop, QWidget *parent) : QDialog(parent, Qt::FramelessWindowHint)
{
    this->setAttribute(Qt::WA_TranslucentBackground, true);

    this->use_backdrop = use_backdrop;
    if (false) {
        if (this->use_backdrop) {
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
    if (!this->overlay_enabled && this->use_backdrop) this->add_cancel_button();
    layout->addLayout(this->buttons);

    return frame;
}

void Dialog::open(int timeout)
{
    if (this->use_backdrop) {
        this->exec();
    }
    else {
        this->show();
        if (timeout > 0) QTimer::singleShot(timeout, [this]() { this->close(); });
    }
}

void Dialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() != Qt::Key_Escape || this->use_backdrop) QDialog::keyPressEvent(event);
}

void Dialog::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    if (QWidget *parent = this->parentWidget()) {
        QPoint point;
        if (this->use_backdrop) {
            point = parent->geometry().center() - this->rect().center();
        }
        else {
            QWidget *window = parent->window();
            QPoint center = parent->mapToGlobal(parent->rect().center());
            int offset = parent->height() / 2;

            QPoint pivot;
            if (center.y() > (window->height() / 2)) {
                pivot = (center.x() > (window->width() / 2)) ? this->rect().bottomRight() : this->rect().bottomLeft();
                pivot.ry() += offset;
            }
            else {
                pivot = (center.x() > (window->width() / 2)) ? this->rect().topRight() : this->rect().topLeft();
                pivot.ry() -= offset;
            }
            point = this->mapFromGlobal(center) - pivot;
        }
        this->move(point);
    }
}
