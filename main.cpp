/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "widgets/mainwindow.h"
#include "utils/definitionholder.h"
#include "utils/qtsingleapplication/qtsingleapplication.h"

#include <QtCore/QTranslator>
#include <QtCore/QLocale>


//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    //enable high DPI scaling on Windows and Linux
    //NOTE: don't move, must be called before QApplication initialization
    //or it won't work at all
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QtSingleApplication passifloraApp(argc, argv);
    passifloraApp.setApplicationName(DefinitionHolder::NAME);
    passifloraApp.setApplicationVersion(DefinitionHolder::VERSION);
    passifloraApp.setOrganizationName(DefinitionHolder::COMPANY);
    passifloraApp.setOrganizationDomain(DefinitionHolder::DOMAIN_NAME);
    passifloraApp.setWindowIcon(QIcon(":/images/icons/passiflora.png"));

    //only one instance allowed
    if (passifloraApp.sendMessage("Wake up!"))
        return 0;

    //setup translations
    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(),
                      ":/languages");
    passifloraApp.installTranslator(&qtTranslator);
    QTranslator myappTranslator;
    myappTranslator.load("passiflora_" + QLocale::system().name(), ":/languages");
    passifloraApp.installTranslator(&myappTranslator);

    //init gui
    MainWindow w;
    w.show();

    //wake up window
    passifloraApp.setActivationWindow(&w);

    return passifloraApp.exec();
}
