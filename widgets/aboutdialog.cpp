/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "aboutdialog.h"
#include "ui_aboutdialog.h"

#include "../utils/definitionholder.h"
#include "../components/settingsmanager.h"

#include <QtWidgets/QLabel>


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    //init
    ui->versionLabel->setText(DefinitionHolder::VERSION);
    ui->buildLabel->setText(QString::number(DefinitionHolder::SOFTWARE_BUILD));
    ui->copyRightLabel->setText(DefinitionHolder::COPYRIGHT);

    SettingsManager sm;
    QString ks, ns, es;
    sm.restoreLicenseKey(ks, ns, es);
    ui->licensedToName->setText(ns);

    //connections
    connect(ui->closeButton, SIGNAL(clicked()),
            this, SLOT(close()));
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

//-----------------------------------------------------------------------------
// Private slots
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------
