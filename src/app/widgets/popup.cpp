#include <app/widgets/popup.hpp>

Dialog::Dialog(QWidget *parent) : QDialog(parent, Qt::FramelessWindowHint)
{
    setAttribute(Qt::WA_TranslucentBackground, true);

    QHBoxLayout *layout = new QHBoxLayout(this);
    QPushButton *button = new QPushButton("button", this);
    button->setStyleSheet("QPushButton { background-color: white; }");
    layout->addWidget(button);
}

void Dialog::mouseReleaseEvent(QMouseEvent *event) { reject(); }
