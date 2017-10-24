/*
 *  Copyright (c) 2017 Giorgio Wicklein <giowckln@gmail.com>
 */

#include "activationdialog.h"
#include "ui_activationdialog.h"
#include "components/activationmanager.h"

#include <QtWidgets/QPushButton>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMessageBox>

ActivationDialog::ActivationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ActivationDialog)
{
    ui->setupUi(this);

    connect(ui->nameLineEdit, &QLineEdit::textChanged, this,
            &ActivationDialog::updateActivateButtonState);
    connect(ui->emailLineEdit, &QLineEdit::textChanged, this,
            &ActivationDialog::updateActivateButtonState);
    connect(ui->licenseKeyLineEdit, &QLineEdit::textChanged, this,
            &ActivationDialog::updateActivateButtonState);
}

ActivationDialog::~ActivationDialog()
{
    delete ui;
}

void ActivationDialog::updateActivateButtonState()
{
    bool disable = ui->nameLineEdit->text().trimmed().isEmpty() ||
            ui->emailLineEdit->text().trimmed().isEmpty() ||
            ui->licenseKeyLineEdit->text().trimmed().isEmpty();

    ui->activateButton->setEnabled(!disable);
}

void ActivationDialog::on_activateButton_clicked()
{
    ActivationManager am(this);
    bool success = am.activateSoftware(ui->licenseKeyLineEdit->text().trimmed(),
                                       ui->nameLineEdit->text().trimmed(),
                                       ui->emailLineEdit->text().trimmed());

    if (success) {
        QMessageBox::information(this, tr("Software successfully activated!"),
                                 tr("Your software version was successfully activated"),
                                 QMessageBox::Ok);
        this->accept();
    } else {
        QMessageBox::warning(this, tr("Invalid license details!"),
                                 tr("The entered license activation details are not valid!"
                                    "<br />Please verify your input and try again"),
                                 QMessageBox::Ok);
    }
}
