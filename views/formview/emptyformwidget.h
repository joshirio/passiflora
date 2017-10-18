/**
  * \class EmptyFormWidget
  * \brief This widget represents a placeholder for an empty FormView
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 13/07/2012
  */

#ifndef EMPTYFORMWIDGET_H
#define EMPTYFORMWIDGET_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include <QWidget>


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

namespace Ui {
class EmptyFormWidget;
}


//-----------------------------------------------------------------------------
// EmptyFormWidget
//-----------------------------------------------------------------------------

class EmptyFormWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit EmptyFormWidget(QWidget *parent = 0);
    ~EmptyFormWidget();
    
private:
    Ui::EmptyFormWidget *ui;
};

#endif // EMPTYFORMWIDGET_H
