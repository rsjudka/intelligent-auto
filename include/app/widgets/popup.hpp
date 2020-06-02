#ifndef POPUP_HPP_
#define POPUP_HPP_

#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

class Dialog : public QDialog {
   public:
    Dialog(QWidget *parent = nullptr);

   protected:
    void mouseReleaseEvent(QMouseEvent *event);
};
#endif
