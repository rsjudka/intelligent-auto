#ifndef SHORTCUT_INPUT_HPP_
#define SHORTCUT_INPUT_HPP_

#include <QKeyEvent>
#include <QPushButton>
#include <QString>
#include <QWidget>

class ShortcutInput : public QPushButton {
    Q_OBJECT

   public:
    ShortcutInput(QString shortcut, QWidget *parent = nullptr);

   protected:
    void keyPressEvent(QKeyEvent *event);

   signals:
    void shortcut_updated(QKeySequence shortcut);
};

#endif
