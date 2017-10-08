/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "dockwidget.h"
#include "../views/collectionlistview/collectionlistview.h"
#include "../components/settingsmanager.h"

#include <QtWidgets/QListView>
#include <QtWidgets/QVBoxLayout>


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

DockWidget::DockWidget(QWidget *parent) :
    QWidget(parent)
{
    //list view for collections
    m_collectionListView = new CollectionListView(this);

    //layout
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->addWidget(m_collectionListView);
    m_mainLayout->setContentsMargins(0,0,0,0);
    setLayout(m_mainLayout);
    setMinimumWidth(150);
    setMaximumWidth(200);
}

void DockWidget::createNewCollection()
{
    m_collectionListView->createNewCollection();
}

void DockWidget::deleteCollection()
{
    m_collectionListView->deleteCollection();
}

CollectionListView *DockWidget::getCollectionListView()
{
    return m_collectionListView;
}


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------
