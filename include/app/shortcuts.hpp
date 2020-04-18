#ifndef SHORTCUTS_HPP_
#define SHORTCUTS_HPP_

#include <QApplication>
#include <QKeyEvent>
#include <QPushButton>
#include <QShortcut>
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

class Shortcuts : public QObject {
    Q_OBJECT

   public:
    Shortcuts() : QObject(qApp) {}

    void add_shortcut(QString id, QString description, QShortcut *shortcut);

    inline void update_shortcut(QString id, QKeySequence key) { this->shortcuts[id].second->setKey(key); }
    inline QMap<QString, QPair<QString, QShortcut *>> get_shortcuts() { return this->shortcuts; }

    static Shortcuts *get_instance();

   private:
    QMap<QString, QPair<QString, QShortcut *>> shortcuts;

   signals:
    void shortcut_added(QString id, QString description, QShortcut *shortcut);
};

#endif
