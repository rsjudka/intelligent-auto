#ifndef LAUNCHER_HPP_
#define LAUNCHER_HPP_

#include <QProcess>
#include <QtWidgets>

#include <app/config.hpp>
#include <app/theme.hpp>

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#undef Bool
#undef CurrentTime
#undef CursorShape
#undef Expose
#undef KeyPress
#undef KeyRelease
#undef FocusIn
#undef FocusOut
#undef FontChange
#undef None
#undef Status
#undef Unsorted

class EmbeddedApp : public QWidget {
    Q_OBJECT

   public:
    EmbeddedApp(int delay, QWidget *parent = nullptr);

    void start(QString app);
    void end();

    inline int get_delay() { return this->delay; }
    inline void set_delay(int delay) { this->delay = delay; }

   private:
    struct WindowProp {
        WindowProp(char *prop, unsigned long size);
        ~WindowProp();

        void *prop;
        unsigned long size;
    };

    WindowProp get_window_prop(Window window, Atom type, const char *name);
    std::list<Window> get_clients();
    int get_window();
    QWidget *controls_widget();

    Display *display;
    Window root_window;
    QProcess *process;
    QVBoxLayout *container;
    int delay;

   signals:
    void closed();
    void opened();
};

class LauncherTab : public QWidget {
    Q_OBJECT

   public:
    LauncherTab(QWidget *parent = nullptr);

   private:
    QWidget *launcher_widget();
    QWidget *app_select_widget();
    QWidget *config_widget();
    QWidget *delay_widget();
    void populate_dirs(QString path);
    void populate_apps(QString path);

    Theme *theme;
    Config *config;
    EmbeddedApp *app;
    QLabel *path_label;
    QListWidget *folders;
    QListWidget *apps;
};

#endif
