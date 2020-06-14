#ifndef DIALOG_HPP_
#define DIALOG_HPP_

#include <QApplication>
#include <QDialog>
#include <QLabel>
#include <QPainter>
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
    Dialog(bool use_backdrop, QWidget *parent = nullptr);
    void open(int timeout = 0);

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
        if (!this->overlay_enabled && this->use_backdrop) this->add_cancel_button();
        button->setFlat(true);
        this->buttons->addWidget(button, 0, Qt::AlignRight);
    }

   protected:
    void showEvent(QShowEvent *event);
    void keyPressEvent(QKeyEvent *event);

   private:
    QVBoxLayout *title;
    QVBoxLayout *body;
    QHBoxLayout *buttons;
    bool use_backdrop;
    bool overlay_enabled = false;

    QWidget *content_widget();

    inline void add_cancel_button()
    {
        QPushButton *cancel = new QPushButton("cancel");
        cancel->setFont(Theme::font_16);
        cancel->setFlat(true);
        connect(cancel, &QPushButton::clicked, [this]() { this->reject(); });
        this->buttons->addWidget(cancel, 0, Qt::AlignRight);
        qApp->processEvents();
        Theme::get_instance()->update();
    }
};

#endif
