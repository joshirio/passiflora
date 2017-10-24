#-------------------------------------------------
#
# Project created by QtCreator 2012-03-22T23:00:55
#
#-------------------------------------------------

QT       += core gui sql network svg widgets printsupport

macx {
    TARGET = Passiflora
}

unix:!macx|win32 {
    TARGET = passiflora
}


TEMPLATE = app


SOURCES += main.cpp\
        widgets/mainwindow.cpp \
    utils/definitionholder.cpp \
    views/formview/formview.cpp \
    components/formlayoutmatrix.cpp \
    widgets/form_widgets/abstractformwidget.cpp \
    widgets/form_widgets/testformwidget.cpp \
    views/formview/droprectwidget.cpp \
    views/formview/selectrectwidget.cpp \
    views/formview/resizedotwidget.cpp \
    widgets/viewtoolbarwidget.cpp \
    widgets/searchlineedit.cpp \
    widgets/dockwidget.cpp \
    views/tableview/tableview.cpp \
    components/metadataengine.cpp \
    models/standardmodel.cpp \
    models/testmodel.cpp \
    models/collectionlistmodel.cpp \
    components/databasemanager.cpp \
    views/tableview/tableviewdelegate.cpp \
    components/settingsmanager.cpp \
    views/collectionlistview/collectionlistview.cpp \
    widgets/form_widgets/textformwidget.cpp \
    utils/platformcolorservice.cpp \
    widgets/form_widgets/numberformwidget.cpp \
    widgets/textarea.cpp \
    utils/metadatapropertiesparser.cpp \
    utils/formwidgetvalidator.cpp \
    views/formview/emptyformwidget.cpp \
    widgets/field_widgets/addfielddialog.cpp \
    widgets/field_widgets/abstractfieldwizard.cpp \
    widgets/field_widgets/textfieldwizard.cpp \
    widgets/field_widgets/numberfieldwizard.cpp \
    components/undocommands.cpp \
    views/collectionlistview/collectionviewdelegate.cpp \
    utils/formviewlayoutstate.cpp \
    components/backupmanager.cpp \
    components/filemanager.cpp \
    utils/qtsingleapplication/qtsinglecoreapplication.cpp \
    utils/qtsingleapplication/qtsingleapplication.cpp \
    utils/qtsingleapplication/qtlockedfile.cpp \
    utils/qtsingleapplication/qtlockedfile_win.cpp \
    utils/qtsingleapplication/qtlockedfile_unix.cpp \
    utils/qtsingleapplication/qtlocalpeer.cpp \
    widgets/preferencesdialog.cpp \
    widgets/backupdialog.cpp \
    widgets/form_widgets/checkboxformwidget.cpp \
    widgets/field_widgets/checkboxfieldwizard.cpp \
    widgets/form_widgets/imageformwidget.cpp \
    widgets/field_widgets/imagefieldwizard.cpp \
    views/tableview/editors/imagetypeeditor.cpp \
    components/updatemanager.cpp \
    widgets/form_widgets/comboboxformwidget.cpp \
    widgets/field_widgets/comboboxfieldwizard.cpp \
    widgets/form_widgets/progressformwidget.cpp \
    widgets/field_widgets/progressfieldwizard.cpp \
    widgets/form_widgets/filesformwidget.cpp \
    widgets/field_widgets/filesfieldwizard.cpp \
    views/tableview/editors/filestypeeditor.cpp \
    widgets/field_widgets/datefieldwizard.cpp \
    widgets/form_widgets/dateformwidget.cpp \
    widgets/field_widgets/creationdatefieldwizard.cpp \
    widgets/form_widgets/creationdateformwidget.cpp \
    widgets/field_widgets/moddatefieldwizard.cpp \
    widgets/form_widgets/moddateformwidget.cpp \
    utils/collectionfieldcleaner.cpp \
    widgets/printdialog.cpp \
    widgets/aboutdialog.cpp \
    widgets/form_widgets/urlformwidget.cpp \
    widgets/field_widgets/urlfieldwizard.cpp \
    widgets/field_widgets/emailfieldwizard.cpp \
    widgets/form_widgets/emailformwidget.cpp \
    components/activationmanager.cpp \
    widgets/activationdialog.cpp

HEADERS  += widgets/mainwindow.h \
    utils/definitionholder.h \
    views/formview/formview.h \
    components/formlayoutmatrix.h \
    widgets/form_widgets/abstractformwidget.h \
    widgets/form_widgets/testformwidget.h \
    views/formview/droprectwidget.h \
    views/formview/selectrectwidget.h \
    views/formview/resizedotwidget.h \
    widgets/viewtoolbarwidget.h \
    widgets/searchlineedit.h \
    widgets/dockwidget.h \
    views/tableview/tableview.h \
    components/metadataengine.h \
    models/standardmodel.h \
    models/testmodel.h \
    models/collectionlistmodel.h \
    components/databasemanager.h \
    views/tableview/tableviewdelegate.h \
    components/settingsmanager.h \
    views/collectionlistview/collectionlistview.h \
    widgets/form_widgets/textformwidget.h \
    utils/platformcolorservice.h \
    widgets/form_widgets/numberformwidget.h \
    widgets/textarea.h \
    utils/metadatapropertiesparser.h \
    utils/formwidgetvalidator.h \
    views/formview/emptyformwidget.h \
    widgets/field_widgets/addfielddialog.h \
    widgets/field_widgets/abstractfieldwizard.h \
    widgets/field_widgets/textfieldwizard.h \
    widgets/field_widgets/numberfieldwizard.h \
    components/undocommands.h \
    views/collectionlistview/collectionviewdelegate.h \
    utils/formviewlayoutstate.h \
    components/backupmanager.h \
    components/filemanager.h \
    utils/qtsingleapplication/qtsinglecoreapplication.h \
    utils/qtsingleapplication/qtsingleapplication.h \
    utils/qtsingleapplication/qtlockedfile.h \
    utils/qtsingleapplication/qtlocalpeer.h \
    widgets/preferencesdialog.h \
    widgets/backupdialog.h \
    widgets/form_widgets/checkboxformwidget.h \
    widgets/field_widgets/checkboxfieldwizard.h \
    widgets/form_widgets/imageformwidget.h \
    widgets/field_widgets/imagefieldwizard.h \
    views/tableview/editors/imagetypeeditor.h \
    components/updatemanager.h \
    widgets/form_widgets/comboboxformwidget.h \
    widgets/field_widgets/comboboxfieldwizard.h \
    widgets/form_widgets/progressformwidget.h \
    widgets/field_widgets/progressfieldwizard.h \
    widgets/form_widgets/filesformwidget.h \
    widgets/field_widgets/filesfieldwizard.h \
    views/tableview/editors/filestypeeditor.h \
    widgets/field_widgets/datefieldwizard.h \
    widgets/form_widgets/dateformwidget.h \
    widgets/field_widgets/creationdatefieldwizard.h \
    widgets/form_widgets/creationdateformwidget.h \
    widgets/field_widgets/moddatefieldwizard.h \
    widgets/form_widgets/moddateformwidget.h \
    utils/collectionfieldcleaner.h \
    widgets/printdialog.h \
    widgets/aboutdialog.h \
    widgets/form_widgets/urlformwidget.h \
    widgets/field_widgets/urlfieldwizard.h \
    widgets/field_widgets/emailfieldwizard.h \
    widgets/form_widgets/emailformwidget.h \
    components/activationmanager.h \
    widgets/activationdialog.h

RESOURCES += \
    resources/resources.qrc

FORMS += \
    ui/emptyformwidget.ui \
    ui/textfieldwizard.ui \
    ui/addfielddialog.ui \
    ui/numberfieldwizard.ui \
    ui/preferencesdialog.ui \
    ui/backupdialog.ui \
    ui/checkboxfieldwizard.ui \
    ui/imagefieldwizard.ui \
    ui/comboboxfieldwizard.ui \
    ui/progressfieldwizard.ui \
    ui/filesfieldwizard.ui \
    ui/datefieldwizard.ui \
    ui/creationdatefieldwizard.ui \
    ui/moddatefieldwizard.ui \
    ui/printdialog.ui \
    ui/importdialog.ui \
    ui/aboutdialog.ui \
    ui/urlfieldwizard.ui \
    ui/emailfieldwizard.ui \
    ui/activationdialog.ui

TRANSLATIONS = stuff/translations/passiflora_de.ts

ICON = resources/images/icons/passiflora.icns # for mac
RC_FILE = resources/passiflora.rc # for windows
