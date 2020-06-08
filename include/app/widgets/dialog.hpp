#ifndef DIALOG_HPP_
#define DIALOG_HPP_

#include <QDialog>
#include <QLabel>
#include <QPainter>
#include <QApplication>
#include <QPushButton>
#include <QVBoxLayout>

#include <app/theme.hpp>

class Overlay : public QWidget {
    Q_OBJECT

   public:
    Overlay(QWidget *parent = nullptr) : QWidget(parent) { setAttribute(Qt::WA_NoSystemBackground); }

   protected:
    inline void paintEvent(QPaintEvent *) override { QPainter(this).fillRect(this->rect(), {0, 0, 0, 108}); }
    inline void mouseReleaseEvent(QMouseEvent *) { emit close(); }

   signals:
    void close();
};

class Dialog : public QDialog {
    Q_OBJECT

   public:
    Dialog(QWidget *parent);
    inline void set_title(QLabel *title)
    {
        title->setFont(QFont("Montserrat", 18, QFont::Bold));
        this->title->addWidget(title);
        qApp->processEvents();
        Theme::get_instance()->update();
    }
    inline void set_body(QWidget *body) { this->body->addWidget(body); }
    inline void set_button(QPushButton *button)
    {
        QPushButton *cancel = new QPushButton("cancel");
        cancel->setFlat(true);
        connect(cancel, &QPushButton::clicked, [this]() { this->reject(); });
        this->buttons->addWidget(cancel, 0, Qt::AlignRight);

        button->setFlat(true);
        this->buttons->addWidget(button, 0, Qt::AlignRight);
    }

   private:
    QVBoxLayout *title;
    QVBoxLayout *body;
    QHBoxLayout *buttons;

    QWidget *content_widget();
};

#endif
