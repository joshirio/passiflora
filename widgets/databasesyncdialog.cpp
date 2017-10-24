#include "databasesyncdialog.h"
#include "ui_databasesyncdialog.h"

DatabaseSyncDialog::DatabaseSyncDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DatabaseSyncDialog)
{
    ui->setupUi(this);
}

DatabaseSyncDialog::~DatabaseSyncDialog()
{
    delete ui;
}
