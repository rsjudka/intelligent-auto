#ifndef PROGRESS_H
#define PROGRESS_H

#include <QColor>
#include <QWidget>

class ProgressIndicator : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal dash_offset READ get_dash_offset WRITE set_dash_offset)
    Q_PROPERTY(qreal dash_length READ get_dash_length WRITE set_dash_length)
    Q_PROPERTY(int angle READ get_angle WRITE set_angle)

   public:
    ProgressIndicator(QWidget* parent = nullptr);

    inline int get_angle() { return this->angle; }
    inline void set_angle(int angle) { this->angle = angle; }

    inline qreal get_dash_length() { return this->dash_length; }
    inline void set_dash_length(qreal length) { this->dash_length = length; }

    inline qreal get_dash_offset() { return this->dash_offset; }
    inline void set_dash_offset(qreal offset) { this->dash_offset = offset; }

   public slots:
    void startAnimation();
    void stopAnimation();

   protected:
    virtual inline void timerEvent(QTimerEvent* event) { update(); }
    virtual void paintEvent(QPaintEvent* event);

   private:
    int timer_id = -1;
    bool enabled = false;
    int angle = 0;
    qreal dash_length = 24;
    qreal dash_offset = 0;
};

#endif