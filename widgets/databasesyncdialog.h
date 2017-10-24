#ifndef DATABASESYNCDIALOG_H
#define DATABASESYNCDIALOG_H

#include <QDialog>

namespace Ui {
class DatabaseSyncDialog;
}

class DatabaseSyncDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DatabaseSyncDialog(QWidget *parent = 0);
    ~DatabaseSyncDialog();

private:
    Ui::DatabaseSyncDialog *ui;
};

#endif // DATABASESYNCDIALOG_H
