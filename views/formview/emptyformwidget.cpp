/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "emptyformwidget.h"
#include "ui_emptyformwidget.h"


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

EmptyFormWidget::EmptyFormWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EmptyFormWidget)
{
    ui->setupUi(this);
}

EmptyFormWidget::~EmptyFormWidget()
{
    delete ui;
}
