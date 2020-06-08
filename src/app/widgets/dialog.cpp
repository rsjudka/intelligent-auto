#include <QFrame>
#include <QGridLayout>
#include <QBoxLayout>

#include <app/widgets/dialog.hpp>

Dialog::Dialog(QWidget *parent) : QDialog(parent, Qt::FramelessWindowHint)
{
    this->setAttribute(Qt::WA_TranslucentBackground, true);
    this->resize(parent->size());

    Overlay *overlay = new Overlay(this);
    overlay->resize(parent->size());
    connect(overlay, &Overlay::close, [this]() { this->reject(); });

    QGridLayout *layout = new QGridLayout(this);
    layout->setColumnStretch(0, 1);
    layout->setColumnStretch(2, 1);
    layout->addWidget(this->content_widget(), 1, 1);
    layout->setRowStretch(0, 1);
    layout->setRowStretch(2, 1);
}

QWidget *Dialog::content_widget()
{
    QFrame *frame = new QFrame(this);
    QVBoxLayout *layout = new QVBoxLayout(frame);

    this->title = new QVBoxLayout();
    layout->addLayout(this->title);

    this->body = new QVBoxLayout();
    layout->addLayout(this->body);

    this->buttons = new QHBoxLayout();
    layout->addLayout(this->buttons);

    return frame;
}
