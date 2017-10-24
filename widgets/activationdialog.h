/**
  * \class ActivationDialog
  * \brief This dialog handles software license activation.
  * \author Giorgio Wicklein
  * \date 24/10/2017
  */

#ifndef ACTIVATIONDIALOG_H
#define ACTIVATIONDIALOG_H

#include <QDialog>

namespace Ui {
class ActivationDialog;
}

class ActivationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ActivationDialog(QWidget *parent = 0);
    ~ActivationDialog();

private slots:
    void updateActivateButtonState();
    void on_activateButton_clicked(); //auto connection

private:
    Ui::ActivationDialog *ui;
};

#endif // ACTIVATIONDIALOG_H
