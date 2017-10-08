/**
  * \class DockWidget
  * \brief This widget represents the content of the QDockWidget in main mindow.
  *        The dock contains a tree-view of the collection's hierarchy and other
  *        indicator/status widgets.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 23/05/2012
  */

#ifndef DOCKWIDGET_H
#define DOCKWIDGET_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include <QWidget>


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

class QVBoxLayout;
class CollectionListView;


//-----------------------------------------------------------------------------
// DockWidget
//-----------------------------------------------------------------------------

class DockWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DockWidget(QWidget *parent = 0);

    /** Create a new collection */
    void createNewCollection();

    /** Delete selected collection */
    void deleteCollection();

    /** Return the collection list item view */
    CollectionListView* getCollectionListView();

private:
    QVBoxLayout *m_mainLayout;
    CollectionListView *m_collectionListView;
};

#endif // DOCKWIDGET_H
