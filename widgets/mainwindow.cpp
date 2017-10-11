/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "mainwindow.h"
#include "../views/formview/formview.h"
#include "../views/tableview/tableview.h"
#include "viewtoolbarwidget.h"
#include "dockwidget.h"
#include "../utils/definitionholder.h"
#include "../components/settingsmanager.h"
#include "../components/metadataengine.h"
#include "../components/databasemanager.h"
#include "../components/filemanager.h"
#include "../components/undocommands.h"
#include "../models/standardmodel.h"
#include "../views/collectionlistview/collectionlistview.h"
#include "field_widgets/addfielddialog.h"
#include "preferencesdialog.h"
#include "backupdialog.h"
#include "printdialog.h"
#include "aboutdialog.h"
#include "../components/updatemanager.h"
#include "../utils/collectionfieldcleaner.h"

#include <QApplication>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QStackedWidget>
#include <QtGui/QCloseEvent>
#include <QVBoxLayout>
#include <QtCore/QString>
#include <QtCore/QAbstractItemModel>
#include <QtCore/QTimer>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QProgressDialog>
#include <QtWidgets/QUndoStack>
#include <QtGui/QDesktopServices>
#include <QtCore/QUrl>


//-----------------------------------------------------------------------------
// Static init
//-----------------------------------------------------------------------------

MainWindow::ViewMode MainWindow::m_currentViewMode = MainWindow::FormViewMode;
QAbstractItemModel* MainWindow::m_currentModel = 0;
QStatusBar* MainWindow::m_statusBar = 0;
QUndoStack* MainWindow::m_undoStack = 0;


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_addFieldDialog(0),
      m_updateManager(0),
      m_lastUsedCollectionId(0)
{
    //init GUI elements
    createComponents();
    createActions();
    createToolBar();
    createDockWidget();
    createMenu();
    createStatusBar();
    createCentralWidget();
    createConnections();

    //set up views with an appropriate model for the last used collection
    attachModelToViews(m_metadataEngine->getCurrentCollectionId());

    restoreSettings();
    init();
    checkForUpdatesSlot();

    setWindowTitle(tr("%1").arg(DefinitionHolder::NAME));
    setMinimumHeight(600);
    setMinimumWidth(800);
}

MainWindow::~MainWindow()
{
    detachModelFromViews();
    detachCollectionModelView();
    delete m_settingsManager;

    m_metadataEngine->destroy();
    DatabaseManager::destroy();
}

MainWindow::ViewMode MainWindow::getCurrentViewMode()
{
    return m_currentViewMode;
}

QAbstractItemModel* MainWindow::getCurrentModel()
{
    return m_currentModel;
}

QStatusBar* MainWindow::getStatusBar()
{
    return m_statusBar;
}

QUndoStack* MainWindow::getUndoStack()
{
    return m_undoStack;
}


//-----------------------------------------------------------------------------
// Protected
//-----------------------------------------------------------------------------

void MainWindow::closeEvent(QCloseEvent *event)
{
    //set focus, to save form view
    setFocus();

    //actions before shutting down
    saveSettings();

    event->accept();
}


//-----------------------------------------------------------------------------
// Private slots
//-----------------------------------------------------------------------------

void MainWindow::aboutActionTriggered()
{
    AboutDialog d(this);
    d.exec();
}

void MainWindow::aboutQtActionTriggered()
{
    qApp->aboutQt();
}

void MainWindow::preferenceActionTriggered()
{
    PreferencesDialog dialog(this);
    dialog.exec();

    //handle changes that requires triggers
    if (dialog.appearanceChanged()) {
        if (m_formView)
            m_formView->reloadAppearanceSettings();
#ifdef Q_OS_LINUX
        //update toolbar style
        if (m_settingsManager->restoreProperty("linuxDarkAmbianceToolbar", "mainWindow").toBool())
            m_toolBar->setStyleSheet("QToolBar {background-color: qlineargradient(spread:pad,"
                                     " x1:0.5, y1:0.01, x2:0.5, y2:0.99, stop:0 rgba(65, 64, "
                                     "59, 255), stop:0.01 rgba(56, 55, 52, 255), stop:0.99 "
                                     "rgba(84, 82, 74, 255), stop:1 rgba(66, 65, 60, 255));} "
                                     "QToolBar:!active {background-color: rgb(60, 59, 55);}");
        else
            m_toolBar->setStyleSheet("");
#endif //Q_OS_LINUX
    }

    if (dialog.softwareResetActivated()) {
        //ask for confirmation
        QMessageBox box(QMessageBox::Warning, tr("Software Reset"),
                        tr("Are you sure you want to delete all data "
                           "from the database including all files and settings?"
                           "<br><br><b>Warning:</b> This cannot be undone!"),
                        QMessageBox::Yes | QMessageBox::No,
                        this);
        box.setDefaultButton(QMessageBox::Yes);
        box.setWindowModality(Qt::WindowModal);
        int r = box.exec();
        if (r == QMessageBox::No) return;

        QProgressDialog *pd = new QProgressDialog(this);
        pd->setWindowModality(Qt::WindowModal);
        pd->setWindowTitle(tr("Progress"));
        pd->setLabelText(tr("Deleting files... Please wait!"));
        pd->setRange(0, 6);
        pd->setValue(1);
        pd->show();
        qApp->processEvents();

        QString fullDbPath = DatabaseManager::getInstance().getDatabasePath();

        //detach views
        detachModelFromViews();
        detachCollectionModelView();
        pd->setValue(2);
        qApp->processEvents();

        //close database
        DatabaseManager::getInstance().destroy();
        pd->setValue(3);
        qApp->processEvents();

        //delete db
        QFile::remove(fullDbPath);
        pd->setValue(4);
        qApp->processEvents();

        //delete all files
        FileManager fm(this);
        fm.removeAllFiles();
        pd->setValue(5);
        qApp->processEvents();

        //delete all settings
        m_settingsManager->removeAllSettings();
        pd->setValue(6);
        qApp->processEvents();

        //close app
        QMessageBox box2(QMessageBox::Information, tr("Software Reset"),
                        tr("Software successfully resetted. "
                           "Terminating now."),
                        QMessageBox::NoButton,
                        this);
        box2.setWindowModality(Qt::WindowModal);
        box2.exec();
        qApp->quit();
    }
}

void MainWindow::formViewModeTriggered()
{
    //set focus to form view and not on its
    //form widgets, to avoid random input
    m_formView->setFocus();

    m_viewStackedWidget->setCurrentIndex(0);
    m_formViewModeAction->setChecked(true);

    //update view toolbar buttons
    m_viewToolBar->setViewModeState(ViewToolBarWidget::FormViewMode);

    m_currentViewMode = FormViewMode;
}

void MainWindow::tableViewModeTriggered()
{
    m_viewStackedWidget->setCurrentIndex(1);
    m_tableViewModeAction->setChecked(true);

    //update view toolbar buttons
    m_viewToolBar->setViewModeState(ViewToolBarWidget::TableViewMode);

    m_currentViewMode = TableViewMode;

    //set current row (record) from FormView
    //this is needed because when editing in form
    //view mode the selection (current) is lost
    int row = m_formView->getCurrentRow();
    if (row != -1) {
        m_tableView->clearSelection();
        m_tableView->setCurrentIndex(m_currentModel->index(row, 1));
    }
}

void MainWindow::fullscreenActionTriggered()
{
    if (isFullScreen()) {
        showNormal();
    } else {
        showFullScreen();
    }
}

/*void MainWindow::toggleDockActionTriggered()
{
    m_dockContainerWidget->setHidden(m_toggleDockAction->isChecked());
}*/

void MainWindow::currentCollectionIdChanged(int collectionId)
{
    //save last active record id (row) to restore it on changeback
    if (m_formView && m_currentModel) {
        QModelIndex index = m_formView->currentIndex();
        if (index.isValid()) {
            int recordId = index.row();
            m_collectionSessionIndexMap[m_lastUsedCollectionId] = recordId;
        }
    }

    detachModelFromViews();
    attachModelToViews(collectionId);

    //restore last active record id (row) if previously saved
    if (m_formView && m_currentModel) {
        int recordId = m_collectionSessionIndexMap[collectionId];
        if (recordId != 0) {
            QModelIndex index = m_formView->model()->index(recordId, 1);
            if (index.isValid())
                m_tableView->setCurrentIndex(index);
        }
    }
}

void MainWindow::currentCollectionChanged()
{
    int currentRow = m_formView->getCurrentRow();

    //for now only StandardCollection as CollectionType
    //is supprted, if new types are added this method needs
    //some adjustments

    //save search filter, in order to restore correct record and index
    StandardModel *sModel = qobject_cast<StandardModel*>(m_currentModel);
    QString currentFilter;
    if (sModel) {
        currentFilter = sModel->filter();
    }

    detachModelFromViews();
    attachModelToViews(m_metadataEngine->getCurrentCollectionId());

    //restore search filter, in order to restore correct record and index
    if (sModel && (!currentFilter.isEmpty())) {
        sModel->setFilter(currentFilter);
    }

    //restore current row
    QModelIndex index = m_formView->model()->index(currentRow, 1);
    if (index.isValid())
        m_tableView->setCurrentIndex(index);
}

void MainWindow::newRecordActionTriggered()
{
    if (!m_currentModel) return;

    if (m_metadataEngine->getFieldCount() <= 1) { //1 cause of _id
        QMessageBox box(QMessageBox::Critical, tr("No Fields"),
                        tr("Failed to create new record!<br>"
                           "Add some fields first."),
                        QMessageBox::NoButton,
                        this);
        box.setWindowModality(Qt::WindowModal);
        box.exec();
        return;
    }

    //for now only StandardCollection as CollectionType
    //is supprted, if new types are added this method needs
    //some adjustments

    StandardModel *sModel = qobject_cast<StandardModel*>(m_currentModel);
    if (sModel) {
        m_formView->setFocus(); //clear focus from form widgets to avoid edit events

        //clear search filter, if any
        if (!sModel->filter().isEmpty())
            sModel->setFilter(""); //clear filter

        //set form view enabled if it was not
        //form view is disabled by searchSlot()
        //if no search result
        if (!m_formView->isEnabled())
            m_formView->setEnabled(true);

        sModel->addRecord(); //add e new empty record

        //create undo action
        QUndoCommand *cmd = new NewRecordCommand;
        m_undoStack->push(cmd);

        statusBar()->showMessage(tr("New record created"));

        //FIXME: temporary workaround for Qt5
        //views should atomatically update but don't
        //needs investigation
        //update views (hard way)
        attachModelToViews(m_metadataEngine->getCurrentCollectionId());

        //select newly created record
        m_formView->selectionModel()->setCurrentIndex(
                    m_currentModel->index(sModel->realRowCount() - 1, 1),
                    QItemSelectionModel::SelectCurrent);
    }
}

void MainWindow::duplicateRecordActionTriggered()
{
    if (!m_currentModel) return;
    if (!m_currentModel->rowCount()) {
        QMessageBox box(QMessageBox::Critical, tr("Duplication Failed"),
                        tr("Failed to duplicate record!<br>"
                           "Add some records first."),
                        QMessageBox::NoButton,
                        this);
        box.setWindowModality(Qt::WindowModal);
        box.exec();
        return;
    }

    //for now only StandardCollection as CollectionType
    //is supprted, if new types are added this method needs
    //some adjustments

    StandardModel *sModel = qobject_cast<StandardModel*>(m_currentModel);
    if (!sModel) return;

    if (m_currentViewMode == FormViewMode) {
        int row = m_formView->getCurrentRow();
        QModelIndex index = m_currentModel->index(row, 0);
        if (index.isValid()) {
            m_formView->setFocus(); //clear focus from form widgets to avoid edit events

            //create undo action
            QUndoCommand *cmd = new DuplicateRecordCommand(row);
            m_undoStack->push(cmd);

            sModel->duplicateRecord(row); //add duplicated record of row
            statusBar()->showMessage(tr("Record %1 duplicated").arg(row+1));

            //FIXME: temporary workaround for Qt5
            //views should atomatically update but don't
            //needs investigation
            //update views (hard way)
            attachModelToViews(m_metadataEngine->getCurrentCollectionId());

            m_formView->selectionModel()->setCurrentIndex(
                        m_currentModel->index(sModel->realRowCount() - 1, 1),
                        QItemSelectionModel::SelectCurrent);
        }
    } else { //table view
        QModelIndexList indexes = m_tableView->selectionModel()->selectedIndexes();
        QSet<int> rows;
        int indexesSize = indexes.size();
        for (int i = 0; i < indexesSize; i++) {
            rows.insert(indexes.at(i).row());
        }
        int rowsSize = rows.size();

        bool canUndo = rowsSize <= 100;
        if (!canUndo) {
            //ask for confirmation
            QMessageBox box(QMessageBox::Question, tr("Duplicate Record"),
                            tr("Are you sure you want to duplicate all selected records?"
                               "<br><br><b>Warning:</b> This cannot be undone!"),
                            QMessageBox::Yes | QMessageBox::No,
                            this);
            box.setDefaultButton(QMessageBox::Yes);
            box.setWindowModality(Qt::WindowModal);
            int r = box.exec();
            if (r == QMessageBox::No) return;
        }

        //create main undo action
        QUndoCommand *mainUndoCommand = new QUndoCommand(tr("record duplication"));

        //init progress dialog
        QProgressDialog progressDialog(tr("Duplicating record 0 of %1")
                                       .arg(rows.size()),
                                       tr("Cancel"), 1,
                                       rowsSize, this);
        progressDialog.setWindowModality(Qt::WindowModal);
        progressDialog.setWindowTitle(tr("Progress"));
        //workaroung bug where dialog remains hidden sometimes
        progressDialog.setMinimumDuration(0);
        progressDialog.show();
        int progress = 0;

        //duplicate selected rows
        m_currentModel->blockSignals(true); //speed up
        DatabaseManager::getInstance().beginTransaction(); //speed up writes

        foreach (const int &row, rows) {
            qApp->processEvents();
            if (progressDialog.wasCanceled())
                break;
            progressDialog.setValue(++progress);
            progressDialog.setLabelText(tr("Duplicating record %1 of %2")
                                        .arg(progress)
                                        .arg(rowsSize));

            if (canUndo) {
                //create child undo command
                new DuplicateRecordCommand(row, mainUndoCommand);
            }

            //duplicate
            sModel->duplicateRecord(row);
        }
        DatabaseManager::getInstance().endTransaction();
        m_currentModel->blockSignals(false);

        if (canUndo) {
            //setup main undo command
            m_undoStack->push(mainUndoCommand);
        } else {
            m_undoStack->clear();
        }

        //update views (hard way)
        attachModelToViews(m_metadataEngine->getCurrentCollectionId());

        statusBar()->showMessage(tr("%1 record(s) duplicated").arg(progress));

        //select first duplicate
        m_formView->selectionModel()->setCurrentIndex(
                    m_currentModel->index(sModel->realRowCount() - rows.size(), 1),
                    QItemSelectionModel::SelectCurrent);
    }

    //set focus back (workaround)
    this->activateWindow();
}

void MainWindow::deleteRecordActionTriggered()
{
    if (!m_currentModel) return;
    if (!m_currentModel->rowCount()) {
        QMessageBox box(QMessageBox::Critical, tr("Deletion Failed"),
                        tr("Failed to delete record!<br>"
                           "The collection is empty."),
                        QMessageBox::NoButton,
                        this);
        box.setWindowModality(Qt::WindowModal);
        box.exec();
        return;
    }

    if (m_currentViewMode == FormViewMode) {
        //ask for confirmation
        QMessageBox box(QMessageBox::Question, tr("Delete Record"),
                        tr("Delete current record?"),
                        QMessageBox::Yes | QMessageBox::No,
                        this);
        box.setDefaultButton(QMessageBox::Yes);
        box.setWindowModality(Qt::WindowModal);
        int r = box.exec();
        if (r == QMessageBox::No) return;

        int currentRow = m_formView->getCurrentRow();
        QModelIndex index = m_currentModel->index(currentRow, 0);
        if (index.isValid()) {
            m_formView->setFocus(); //clear focus from form widgets to avoid edit events

            //create undo action
            QUndoCommand *cmd = new DeleteRecordCommand(index.row());
            m_undoStack->push(cmd);

            //remove
            m_currentModel->removeRow(index.row());

            //FIXME: temporary workaround for Qt5
            //views should atomatically update but don't
            //needs investigation
            //update views (hard way)
            attachModelToViews(m_metadataEngine->getCurrentCollectionId());

            statusBar()->showMessage(tr("Record %1 deleted").arg(index.row()+1));
        }
    } else { //table view
        QModelIndexList indexes = m_tableView->selectionModel()->selectedIndexes();
        QSet<int> rows;
        int indexesSize = indexes.size();
        for (int i = 0; i < indexesSize; i++) {
            rows.insert(indexes.at(i).row());
        }

        //select entire records
        m_tableView->selectionModel()->select(m_tableView->selectionModel()->selection(),
                                              QItemSelectionModel::ClearAndSelect |
                                              QItemSelectionModel::Rows);

        bool canUndo = rows.size() <= 100;
        if (canUndo) {
            //ask for confirmation
            QMessageBox box(QMessageBox::Question, tr("Delete Record"),
                            tr("Delete selected records?"),
                            QMessageBox::Yes | QMessageBox::No,
                            this);
            box.setDefaultButton(QMessageBox::Yes);
            box.setWindowModality(Qt::WindowModal);
            int r = box.exec();
            if (r == QMessageBox::No) return;
        } else {
            //ask for confirmation
            QMessageBox box(QMessageBox::Question, tr("Delete Record"),
                            tr("Are you sure you want to delete all selected records?"
                               "<br><br><b>Warning:</b> This cannot be undone!"),
                            QMessageBox::Yes | QMessageBox::No,
                            this);
            box.setDefaultButton(QMessageBox::Yes);
            box.setWindowModality(Qt::WindowModal);
            int r = box.exec();
            if (r == QMessageBox::No) return;
        }

        //create main undo action
        QUndoCommand *mainUndoCommand = new QUndoCommand(tr("record deletion"));

        //init progress dialog
        QProgressDialog progressDialog(tr("Deleting record 0 of %1")
                                       .arg(rows.size()),
                                       tr("Cancel"), 1,
                                       rows.size(), this);
        progressDialog.setWindowModality(Qt::WindowModal);
        progressDialog.setWindowTitle(tr("Progress"));
#ifdef Q_OS_WIN
        //workaroung for windows bug where dialog remains hidden sometimes
        progressDialog.setMinimumDuration(0);
        progressDialog.show();
#endif // Q_OS_WIN
        int progress = 0;

        //remove selected rows
        m_currentModel->blockSignals(true); //speed up
        DatabaseManager::getInstance().beginTransaction(); //speed up writes

        //sort rows in a list so that removing from bottom to top works
        QList<int> rowList = rows.toList();
        qSort(rowList.begin(), rowList.end());
        int rowListSize = rowList.size();
        for (int i = rowListSize - 1; i >= 0; i--) {
#ifdef Q_OS_WIN
            //workaroung for windows bug where dialog remains hidden sometimes
            qApp->processEvents();
#endif // Q_OS_WIN
            if (progressDialog.wasCanceled())
                break;
            progressDialog.setValue(++progress);
            progressDialog.setLabelText(tr("Deleting record %1 of %2")
                                        .arg(progress)
                                        .arg(rowListSize));
            int r = rowList.at(i);

            if (canUndo) {
                //create child undo command
                new DeleteRecordCommand(r, mainUndoCommand);
            } else {
                m_undoStack->clear();
            }

            //remove
            m_currentModel->removeRow(r);
        }
        DatabaseManager::getInstance().endTransaction();
        m_currentModel->blockSignals(false);

        if (canUndo) {
            //setup main undo command
            m_undoStack->push(mainUndoCommand);
        }

        //update views (hard way)
        attachModelToViews(m_metadataEngine->getCurrentCollectionId());

        //status message
        statusBar()->showMessage(tr("%1 record(s) deleted").arg(progress));

        //select record before deleted items
        int previousRecord = 0;
        if (indexes.count() > 0 )
            previousRecord = indexes.at(0).row() - 1;

        m_formView->selectionModel()->setCurrentIndex(
                    m_currentModel->index(previousRecord, 1),
                    QItemSelectionModel::SelectCurrent);
    }

    //set focus back (workaround)
    this->activateWindow();
}

void MainWindow::newCollectionActionTriggered()
{
    m_dockWidget->createNewCollection();
}

void MainWindow::deleteCollectionActionTriggered()
{
    m_dockWidget->deleteCollection();
}

void MainWindow::deleteAllRecordsActionTriggered()
{
    //ask for confirmation
    QMessageBox box(QMessageBox::Question, tr("Delete All Records"),
                    tr("Are you sure you want to delete all records from the "
                       "current collection?"
                       "<br><br><b>Warning:</b> This cannot be undone!"),
                    QMessageBox::Yes | QMessageBox::No,
                    this);
    box.setDefaultButton(QMessageBox::Yes);
    box.setWindowModality(Qt::WindowModal);
    int r = box.exec();
    if (r == QMessageBox::No) return;

    int id = m_metadataEngine->getCurrentCollectionId();
    m_metadataEngine->deleteAllRecords(id);

    //update views (hard way)
    attachModelToViews(m_metadataEngine->getCurrentCollectionId());

    statusBar()->showMessage(tr("All records successfully deleted"));

    //set focus back (workaround)
    this->activateWindow();

    //clear undo stack since this action is not undoable
    m_undoStack->clear();
}

void MainWindow::optimizeDbSizeActionTriggered()
{
    double bytesBefore = DatabaseManager::getInstance().getDatabaseFileSize();

    FileManager fm(this);

    //remove orphan file ids from database
    //(orphans are files that are in database but not used in any records)
    QStringList orphanFileIds = fm.orphanDatabaseFileList();
    foreach (QString orphanId, orphanFileIds) {
        fm.removeFileMetadata(orphanId.toInt());
    }

    //get unneeded local files (local files that are not in database)
    //this includes orphan files because they just got removed from database
    QStringList files = fm.unneededLocalFileList();


    QString filesDir = fm.getFilesDirectory();
    int steps = 1 + files.size();
    qint64 removedFileBytes = 0;

    //init progress dialog
    QProgressDialog progressDialog(tr("Removing obsolete data..."),
                                   tr("Cancel"), 0,
                                   steps, this);
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.setWindowTitle(tr("Progress"));
#ifdef Q_OS_WIN
    //workaroung for windows bug where dialog remains hidden sometimes
    progressDialog.setMinimumDuration(0);
    progressDialog.show();
#endif // Q_OS_WIN
    int progress = 0;

    //clean db
    DatabaseManager::getInstance().optimizeDatabaseSize();
    progress++;

    foreach (QString s, files) {
#ifdef Q_OS_WIN
        //workaroung for windows bug where dialog remains hidden sometimes
        qApp->processEvents();
#endif // Q_OS_WIN
        if (progressDialog.wasCanceled())
            break;

        //remove file
        removedFileBytes += QFile(filesDir + s).size();
        QFile::remove(filesDir + s);

        progressDialog.setValue(++progress);
    }

    double bytesAfter = DatabaseManager::getInstance().getDatabaseFileSize();
    double kib = (bytesBefore - bytesAfter) / (1024.0); //B -> KiB
    qint64 mib = removedFileBytes / (1024 * 1024); //b -> MiB

    QMessageBox box(QMessageBox::Information, tr("Database Size"),
                    tr("Database size reduced by %1 KiB\n"
                       "Files archive reduced by %2 MiB").arg(kib).arg(mib),
                    QMessageBox::NoButton,
                    this);
    box.setWindowModality(Qt::WindowModal);
    box.exec();

    //clear undo stack (deleted files/images are not undoable)
    m_undoStack->clear();
}

void MainWindow::newFieldActionTriggered()
{
    if (!m_metadataEngine->getCurrentCollectionId()) return;

    if (!m_addFieldDialog) {
        m_addFieldDialog = new AddFieldDialog(this);
        m_addFieldDialog->setWindowModality(Qt::WindowModal);
    }

    m_addFieldDialog->setCreationMode(AbstractFieldWizard::NewEditMode);
    m_addFieldDialog->show();
}

void MainWindow::duplicateFieldActionTriggered()
{
    if (!m_metadataEngine->getCurrentCollectionId()) return;

    int fieldId;
    if (m_currentViewMode == FormViewMode) {
        fieldId = m_formView->getSelectedField();
    } else {
        if (m_tableView->selectionModel()->hasSelection())
            fieldId = m_tableView->currentIndex().column();
        else
            fieldId = -1;
    }

    if (fieldId == -1) { //if no selection
        QMessageBox box(QMessageBox::Information, tr("Missing Field Selection"),
                        tr("Select a field to duplicate first!"),
                        QMessageBox::NoButton,
                        this);
        box.setWindowModality(Qt::WindowModal);
        box.exec();
        return;
    }

    if (!m_addFieldDialog) {
        m_addFieldDialog = new AddFieldDialog(this);
        m_addFieldDialog->setWindowModality(Qt::WindowModal);
    }

    m_addFieldDialog->setCreationMode(AbstractFieldWizard::DuplicateEditMode,
                                      fieldId,
                                      m_metadataEngine->getCurrentCollectionId());
    m_addFieldDialog->show();
}

void MainWindow::deleteFieldActionTriggered()
{
    if (!m_metadataEngine->getCurrentCollectionId()) return;

    int fieldId;
    if (m_currentViewMode == FormViewMode) {
        m_formView->setFocus(); //clear focus from form widgets to avoid edit events
        fieldId = m_formView->getSelectedField();
    } else {
        if (m_tableView->selectionModel()->hasSelection()) {
            fieldId = m_tableView->currentIndex().column();

            //select entire column
            QModelIndex currentIndex = m_tableView->currentIndex();
            m_tableView->selectionModel()->select(currentIndex,
                                                  QItemSelectionModel::ClearAndSelect |
                                                  QItemSelectionModel::Columns);
        } else {
            fieldId = -1;
        }
    }

    if (fieldId == -1) { //if no selection
        QMessageBox box(QMessageBox::Information, tr("Missing Field Selection"),
                        tr("Select a field to delete first!"),
                        QMessageBox::NoButton,
                        this);
        box.setWindowModality(Qt::WindowModal);
        box.exec();
        return;
    } else {
        //ask for confirmation
        QMessageBox box(QMessageBox::Question, tr("Field Deletion"),
                        tr("Are you sure you want to delete the selected "
                           "collection field with all data related?"
                           "<br><br><b>Warning:</b> This cannot be undone!"),
                        QMessageBox::Yes | QMessageBox::No,
                        this);
        box.setDefaultButton(QMessageBox::Yes);
        box.setWindowModality(Qt::WindowModal);
        int r = box.exec();
        if (r == QMessageBox::No) return;
    }

    //check field deletion trigger actions
    CollectionFieldCleaner cleaner(this);
    cleaner.cleanField(m_metadataEngine->getCurrentCollectionId(),
                       fieldId);

    //remove field
    m_metadataEngine->deleteField(fieldId);

    //clear undo stack since this action is not undoable
    m_undoStack->clear();
}

void MainWindow::modifyFieldActionTriggered()
{
    int fieldId;
    if (m_currentViewMode == FormViewMode) {
        fieldId = m_formView->getSelectedField();
    } else {
        if (m_tableView->selectionModel()->hasSelection())
            fieldId = m_tableView->currentIndex().column();
        else
            fieldId = -1;
    }

    if (fieldId == -1) { //if no selection
        return;
    }

    if (!m_addFieldDialog) {
        m_addFieldDialog = new AddFieldDialog(this);
        m_addFieldDialog->setWindowModality(Qt::WindowModal);
    }

    m_addFieldDialog->setCreationMode(AbstractFieldWizard::ModifyEditMode,
                                      fieldId,
                                      m_metadataEngine->getCurrentCollectionId());
    m_addFieldDialog->show();
}

void MainWindow::searchSlot(const QString &s)
{
    if (!m_metadataEngine->getCurrentCollectionId()) return;

    QString key(s);

    //set to table view mode
    tableViewModeTriggered();

    //if searching for double (numeric) values
    //replace ',' with '.', since in db
    //decimal point is always '.'
    //and not ',' as in some languages
    QLocale locale;
    if (locale.decimalPoint() == ',')
        key.replace(',', '.');

    //remove the "'" char because model gets messed up
    //even if removed from search, model is not able
    //to populate again (SQL injection risk?)
    key.remove(QRegExp("'"));

    //adapt if new collection types are added,
    //for now assuming only standard collection
    StandardModel *sModel = qobject_cast<StandardModel*>(m_currentModel);
    if (sModel) {
        int count = m_metadataEngine->getFieldCount();

        //generate filter (where clause)
        QString filter;
        if (count > 0) {
            filter.append(QString("\"1\" LIKE '%%2%'").arg(key));
        }
        for (int i = 2; i < count; i++) { //start with 2 cause 0 is _id and 1 done
            switch(m_metadataEngine->getFieldType(i)) {
            case MetadataEngine::CheckboxType:
            case MetadataEngine::ComboboxType:
            case MetadataEngine::ProgressType:
            case MetadataEngine::ImageType:
            case MetadataEngine::FilesType:
            case MetadataEngine::DateType:
            case MetadataEngine::CreationDateType:
            case MetadataEngine::ModDateType:
                //exclude field type from search results
                break;
            default:
                filter.append(QString(" OR \"%1\" LIKE '%%2%'")
                              .arg(i).arg(key));
                break;
            }
        }
        if (!s.isEmpty())
            sModel->setFilter(filter);
        else
            sModel->setFilter(""); //clear filter
    }

    //select first result, if no result disable form view
    QModelIndex index = m_currentModel->index(0, 1);
    m_formView->selectionModel()->setCurrentIndex(
                index,
                QItemSelectionModel::SelectCurrent);
    m_formView->setEnabled(index.isValid());

    //clear undo stack cause row ids are not longer valid
    m_undoStack->clear();
}

void MainWindow::selectAllActionTriggered()
{
    if (!m_currentModel) return;

    //set form view
    tableViewModeTriggered();

    //select all
    QModelIndex p;
    QModelIndex topLeft = m_currentModel->index(0, 1, p);
    QModelIndex bottomRight = m_currentModel->index(
                m_currentModel->rowCount(p)-1,
                m_currentModel->columnCount(p)-1,
                p);
    QItemSelection selection(topLeft, bottomRight);
    m_formView->selectionModel()->select(selection, QItemSelectionModel::Select);
}

void MainWindow::backupActionTriggered()
{
    //detach views
    detachModelFromViews();
    detachCollectionModelView();
    int id = m_metadataEngine->getCurrentCollectionId();

    BackupDialog dialog(this);
    dialog.exec();

    //if backup restored, restart
    if (dialog.backupRestored()) {
        QMessageBox box(QMessageBox::Information, tr("Software Restart"),
                        tr("Software restart required! "
                           "Please restart %1 manually.").arg(DefinitionHolder::NAME),
                        QMessageBox::NoButton,
                        this);
        box.setWindowModality(Qt::WindowModal);
        box.exec();
        qApp->quit();
    } else {
        //attach views
        attachCollectionModelView();
        m_metadataEngine->setCurrentCollectionId(id);
    }
}

void MainWindow::printActionTriggered()
{
    if ((!m_currentModel) || (!m_currentModel->rowCount())) {
        QMessageBox box(QMessageBox::Information, tr("Printing aborted"),
                        tr("There are no records to print!"),
                        QMessageBox::NoButton,
                        this);
        box.setWindowModality(Qt::WindowModal);
        box.exec();
        return;
    }

    QList<int> recordIdList; //ids of records to print

    if (m_currentViewMode == FormViewMode) {
        //get current record id
        int currentRow = m_formView->getCurrentRow();
        QModelIndex index = m_currentModel->index(currentRow, 0);
        if (index.isValid()) {
            bool ok;
            int recordId = index.data().toInt(&ok);
            if (ok)
                recordIdList.append(recordId);
        }

    } else { //table view
        QModelIndexList indexes = m_tableView->selectionModel()->selectedIndexes();
        QSet<int> rows;
        int indexesSize = indexes.size();
        for (int i = 0; i < indexesSize; i++) {
            rows.insert(indexes.at(i).row());
        }

        QList<int> rowList = rows.toList();
        int size = rowList.size();
        QModelIndex index;
        //extract record ids
        for (int i = 0; i < size; i++) {
            index = m_currentModel->index(rowList.at(i), 0);
            if (index.isValid()) {
                bool ok;
                int recordId = index.data().toInt(&ok);
                if (ok)
                    recordIdList.append(recordId);
            }
        }
    }

    //print dialog
    PrintDialog d(m_metadataEngine->getCurrentCollectionId(),
                  recordIdList,
                  this);
    d.exec();

    //set focus back (workaround)
    this->activateWindow();
}

void MainWindow::checkForUpdatesSlot()
{
    if (DefinitionHolder::APP_STORE ||
            (!m_settingsManager->restoreCheckUpdates()))
        return;

    if (!m_updateManager) {
        m_updateManager = new UpdateManager(this);
        connect(m_updateManager, SIGNAL(noUpdateSignal()),
                this, SLOT(noUpdateFoundSlot()));
        connect(m_updateManager, SIGNAL(updateErrorSignal()),
                this, SLOT(updateErrorSlot()));
        connect(m_updateManager, SIGNAL(updatesAccepted()),
                this, SLOT(close()));
    }

    //start async update check
    statusBar()->showMessage(tr("Checking for updates..."));
    m_updateManager->checkForUpdates();
}

void MainWindow::noUpdateFoundSlot()
{
    statusBar()->showMessage(tr("Your software version is up to date"));
}

void MainWindow::updateErrorSlot()
{
    statusBar()->showMessage(tr("Error while checking for software updates"));
}


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

void MainWindow::createActions()
{
    m_quitAction = new QAction(tr("&Quit"), this);
    m_quitAction->setStatusTip(tr("Exit from this application"));
    m_quitAction->setShortcut(QKeySequence::Quit);
    m_quitAction->setMenuRole(QAction::QuitRole);

    m_aboutAction = new QAction(tr("About %1").arg(DefinitionHolder::NAME), this);
    m_aboutAction->setMenuRole(QAction::AboutRole);

    m_aboutQtAction = new QAction(tr("About Qt"), this);
    m_aboutQtAction->setMenuRole(QAction::AboutQtRole);

    m_newCollectionAction = new QAction(tr("New Collection..."), this);
    m_newCollectionAction->setIcon(QIcon(":/images/icons/newcollection.png"));
    m_newCollectionAction->setStatusTip(tr("Create a new collection"));

    m_deleteCollectionAction = new QAction(tr("Delete Collection"), this);
    m_deleteCollectionAction->setIcon(QIcon(":/images/icons/deletecollection.png"));
    m_deleteCollectionAction->setStatusTip(tr("Delete current collection"));

    m_newRecordAction = new QAction(tr("New Record"), this);
    m_newRecordAction->setShortcut(QKeySequence::New);
    m_newRecordAction->setIcon(QIcon(":/images/icons/newrecord.png"));

    m_newFieldAction = new QAction(tr("New Field..."), this);
    m_newFieldAction->setIcon(QIcon(":/images/icons/newfield.png"));

    m_backupAction = new QAction(tr("Backup..."), this);
    m_backupAction->setStatusTip(tr("Backup or restore a database file"));

    m_settingsAction = new QAction(tr("Preferences"), this);
    m_settingsAction->setMenuRole(QAction::PreferencesRole);
    m_settingsAction->setShortcut(QKeySequence::Preferences);
    m_settingsAction->setStatusTip(tr("Change application settings"));

    m_undoAction = m_undoStack->createUndoAction(this, tr("Undo"));
    m_undoAction->setShortcut(QKeySequence::Undo);

    m_redoAction = m_undoStack->createRedoAction(this, tr("Redo"));
    m_redoAction->setShortcut(QKeySequence::Redo);

    m_selectAllAction = new QAction(tr("Select all records"), this);

    m_findAction = new QAction(tr("Find"), this);
    m_findAction->setShortcut(QKeySequence::Find);

    m_formViewModeAction = new QAction(tr("Form View"), this);
    m_formViewModeAction->setShortcut(QString("CTRL+G"));
    m_formViewModeAction->setStatusTip(tr("Change current view mode to a form-like view"));
    m_formViewModeAction->setCheckable(true);
    m_formViewModeAction->setChecked(true);

    m_tableViewModeAction = new QAction(tr("Table View"), this);
    m_tableViewModeAction->setShortcut(QString("CTRL+T"));
    m_tableViewModeAction->setStatusTip(tr("Change current view mode to a table-like view"));
    m_tableViewModeAction->setCheckable(true);

    m_viewModeActionSeparator = new QAction(tr("View Mode"), this);
    m_viewModeActionSeparator->setSeparator(true);

    m_viewModeActionGroup = new QActionGroup(this);
    m_viewModeActionGroup->addAction(m_formViewModeAction);
    m_viewModeActionGroup->addAction(m_tableViewModeAction);

#ifdef Q_OS_OSX
    m_minimizeAction = new QAction(tr("Minimize"), this);
    m_minimizeAction->setShortcut(QString("CTRL+M"));

    m_closeWindowAction = new QAction(tr("Close"), this);
    m_closeWindowAction->setShortcut(QString("CTRL+W"));
#endif //Q_OS_OSX

    m_fullscreenAction = new QAction(tr("Fullscreen"), this);
#ifndef Q_OS_OSX
    m_fullscreenAction->setShortcut(QString("F11"));
#endif

    /*m_toggleDockAction = new QAction(tr("Hide collection sidebar"), this);
    m_toggleDockAction->setCheckable(true);
    m_toggleDockAction->setShortcut(QString("CTRL+B"));
    */

    m_deleteAllRecordsAction = new QAction(tr("Delete all records"), this);
    m_deleteAllRecordsAction->setStatusTip(
                tr("Remove all records from current collection"));

    m_optimizeDbSizeAction = new QAction(tr("Free unused space"), this);
    m_optimizeDbSizeAction->setStatusTip(tr("Optimize size of database file "
                                            "by freeing unused resources"));

    m_checkUpdatesAction = new QAction(tr("Check for updates"), this);
    m_checkUpdatesAction->setStatusTip(tr("Check for %1 updates")
                                     .arg(DefinitionHolder::NAME));
    m_checkUpdatesAction->setMenuRole(QAction::ApplicationSpecificRole);

    m_printAction = new QAction(tr("Print..."), this);
    m_printAction->setStatusTip(tr("Print records or export them as PDF"));
    m_printAction->setShortcut(QKeySequence::Print);
}

void MainWindow::createToolBar()
{
    m_toolBar = new QToolBar(tr("Toolbar"), this);

    //set object name to allow saveState()
    m_toolBar->setObjectName("mainToolBar");

    m_toolBar->setIconSize(QSize(32, 32));
    m_toolBar->setMovable(false);

    m_toolBar->addAction(m_newCollectionAction);
    m_toolBar->addAction(m_deleteCollectionAction);
    m_toolBar->addSeparator();

    this->addToolBar(m_toolBar);

    //for passiflora no toolbar is needed at this point
    this->removeToolBar(m_toolBar);
}

void MainWindow::createDockWidget()
{
    m_dockWidget = new DockWidget(this);
    m_dockContainerWidget = new QDockWidget(tr("COLLECTIONS"), this);

    //set object name to allow saveState()
    m_dockContainerWidget->setObjectName("collectionDock");

    m_dockContainerWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
    m_dockContainerWidget->setWidget(m_dockWidget);

    m_dockContainerWidget->setStyleSheet("QDockWidget::title {"
                                         "padding-top: 0px;"
                                         "padding-bottom: 0px;"
                                         "margin-top: 0px;"
                                         "margin-bottom: 0px;"
                                         "text-align: left;"
                                         "padding-left: 8px;"
                                         "background-color: transparent;"
                                         "}"
                                         "QDockWidget {"
                                         "font-size: 11px;"
                                         "font-weight: bold;"
                                         "color: #6F7E8B;"
                                         "}");

    addDockWidget(Qt::LeftDockWidgetArea, m_dockContainerWidget);

    //for passiflora only one default collection
    //so hide dock but keep the code
    //in case more collections are needed in future
    m_dockContainerWidget->setHidden(true);
}

void MainWindow::createMenu()
{
    m_newMenu = new QMenu(tr("New"), this);
    m_newMenu->addAction(m_newCollectionAction);
    m_newMenu->addAction(m_newRecordAction);
    m_newMenu->addAction(m_newFieldAction);

    m_fileMenu = menuBar()->addMenu(tr("&File"));
    m_fileMenu->addMenu(m_newMenu);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_backupAction);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_printAction);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_quitAction);

    m_editMenu = menuBar()->addMenu(tr("&Edit"));
    m_editMenu->addAction(m_undoAction);
    m_editMenu->addAction(m_redoAction);
    m_editMenu->addSeparator();
    m_editMenu->addAction(m_selectAllAction);
    m_editMenu->addSeparator();
    m_editMenu->addAction(m_findAction);

    m_viewMenu = menuBar()->addMenu(tr("&View"));
    m_viewMenu->addAction(m_fullscreenAction);
    //m_viewMenu->addAction(m_toggleDockAction);
    m_viewMenu->addAction(m_viewModeActionSeparator);
    m_viewMenu->addAction(m_formViewModeAction);
    m_viewMenu->addAction(m_tableViewModeAction);

    m_recordsMenu = new QMenu(tr("Records"), this);
    m_recordsMenu->addAction(m_deleteAllRecordsAction);

    m_databaseMenu = new QMenu(tr("Database"), this);
    m_databaseMenu->addAction(m_optimizeDbSizeAction);

    m_toolsMenu = menuBar()->addMenu(tr("&Tools"));
    m_toolsMenu->addMenu(m_recordsMenu);
    m_toolsMenu->addMenu(m_databaseMenu);
    m_toolsMenu->addSeparator();
    m_toolsMenu->addAction(m_settingsAction);

#ifdef Q_OS_OSX
    m_windowMenu = menuBar()->addMenu(tr("&Window"));
    m_windowMenu->addAction(m_minimizeAction);
    m_windowMenu->addAction(m_closeWindowAction);
#endif //Q_OS_OSX

    m_helpMenu = menuBar()->addMenu(tr("&Help"));
    m_helpMenu->addAction(m_aboutAction);
    m_helpMenu->addAction(m_aboutQtAction);
    m_helpMenu->addSeparator();
    if (!DefinitionHolder::APP_STORE)
        m_helpMenu->addAction(m_checkUpdatesAction);

    //passiflora disable unused actions
    m_newMenu->menuAction()->setVisible(false);
    m_recordsMenu->menuAction()->setVisible(false);
    m_optimizeDbSizeAction->setVisible(false);
}

void MainWindow::createStatusBar()
{
    m_statusBar = statusBar();
    m_statusBar->showMessage(tr(" Ready "));
}

void MainWindow::createComponents()
{
    m_settingsManager = new SettingsManager;
    m_metadataEngine = &MetadataEngine::getInstance();
    m_undoStack = new QUndoStack(this);
}

void MainWindow::createCentralWidget()
{
    //create widgets
    m_viewToolBar = new ViewToolBarWidget(this);
    m_formView = new FormView(this);
    m_tableView = new TableView(this);
    m_centralStackedWidget = new QStackedWidget(this);
    m_viewStackedWidget = new QStackedWidget(this);

    //setup ViewStackedWidget
    m_viewStackedWidget->addWidget(m_formView);
    m_viewStackedWidget->addWidget(m_tableView);

    //build ViewWidget
    QWidget *viewWidget = new QWidget(this);
    QVBoxLayout *viewMainLayout = new QVBoxLayout(viewWidget);
    viewWidget->setLayout(viewMainLayout);
    viewMainLayout->addWidget(m_viewToolBar);
    viewMainLayout->addWidget(m_viewStackedWidget);
    viewMainLayout->setSpacing(0);
    viewMainLayout->setContentsMargins(0,0,0,0);

    //setup central widget
    m_centralStackedWidget->addWidget(viewWidget);
    setCentralWidget(m_centralStackedWidget);

    m_formView->setFocus();
}

void MainWindow::createConnections()
{
    //main window
    connect(m_quitAction, SIGNAL(triggered()),
            this, SLOT(close()));
    connect(m_aboutAction, SIGNAL(triggered()),
            this, SLOT(aboutActionTriggered()));
    connect(m_aboutQtAction, SIGNAL(triggered()),
            this, SLOT(aboutQtActionTriggered()));
    connect(m_settingsAction, SIGNAL(triggered()),
            this, SLOT(preferenceActionTriggered()));
    connect(m_findAction, SIGNAL(triggered()),
            m_viewToolBar, SLOT(setSearchLineFocus()));
    connect(m_formViewModeAction, SIGNAL(triggered()),
            this, SLOT(formViewModeTriggered()));
    connect(m_tableViewModeAction, SIGNAL(triggered()),
            this, SLOT(tableViewModeTriggered()));
    connect(m_fullscreenAction, SIGNAL(triggered()),
            this, SLOT(fullscreenActionTriggered()));
    /*connect(m_toggleDockAction, SIGNAL(triggered()),
            this, SLOT(toggleDockActionTriggered()));*/
    connect(m_selectAllAction, SIGNAL(triggered()),
            this, SLOT(selectAllActionTriggered()));
    connect(m_backupAction, SIGNAL(triggered()),
            this, SLOT(backupActionTriggered()));
    connect(m_checkUpdatesAction, SIGNAL(triggered()),
            this, SLOT(checkForUpdatesSlot()));
    connect(m_printAction, SIGNAL(triggered()),
            this, SLOT(printActionTriggered()));

    //record actions
    connect(m_newRecordAction, SIGNAL(triggered()),
            this, SLOT(newRecordActionTriggered()));
    connect(m_viewToolBar, SIGNAL(newRecordSignal()),
            this, SLOT(newRecordActionTriggered()));
    connect(m_viewToolBar, SIGNAL(duplicateRecordSignal()),
            this, SLOT(duplicateRecordActionTriggered()));
    connect(m_viewToolBar, SIGNAL(deleteRecordSignal()),
            this, SLOT(deleteRecordActionTriggered()));

    //field actions
    connect(m_newFieldAction, SIGNAL(triggered()),
            this, SLOT(newFieldActionTriggered()));
    connect(m_viewToolBar, SIGNAL(newFieldSignal()),
            this, SLOT(newFieldActionTriggered()));
    connect(m_viewToolBar, SIGNAL(duplicateFieldSignal()),
            this, SLOT(duplicateFieldActionTriggered()));
    connect(m_viewToolBar, SIGNAL(deleteFieldSignal()),
            this, SLOT(deleteFieldActionTriggered()));

    //form view context menu actions
    connect(m_formView, SIGNAL(newFieldSignal()),
           this, SLOT(newFieldActionTriggered()));
    connect(m_formView, SIGNAL(duplicateFieldSignal()),
           this, SLOT(duplicateFieldActionTriggered()));
    connect(m_formView, SIGNAL(deleteFieldSignal()),
           this, SLOT(deleteFieldActionTriggered()));
    connect(m_formView, SIGNAL(modifyFieldSignal()),
           this, SLOT(modifyFieldActionTriggered()));
    connect(m_formView, SIGNAL(newRecordSignal()),
           this, SLOT(newRecordActionTriggered()));
    connect(m_formView, SIGNAL(duplicateRecordSignal()),
           this, SLOT(duplicateRecordActionTriggered()));
    connect(m_formView, SIGNAL(deleteRecordSignal()),
           this, SLOT(deleteRecordActionTriggered()));

    //collection actions
    connect(m_newCollectionAction, SIGNAL(triggered()),
            this, SLOT(newCollectionActionTriggered()));
    connect(m_deleteCollectionAction, SIGNAL(triggered()),
            this, SLOT(deleteCollectionActionTriggered()));

    //tools action
    connect(m_deleteAllRecordsAction, SIGNAL(triggered()),
            this, SLOT(deleteAllRecordsActionTriggered()));
    connect(m_optimizeDbSizeAction, SIGNAL(triggered()),
            this, SLOT(optimizeDbSizeActionTriggered()));

#ifdef Q_OS_OSX
    //minimize action
    connect(m_minimizeAction, SIGNAL(triggered()),
            this, SLOT(showMinimized()));
    connect(m_closeWindowAction, SIGNAL(triggered()),
            this, SLOT(close()));
#endif //Q_OS_OSX

    //view toolbar
    connect(m_viewToolBar, SIGNAL(formViewModeSignal()),
            this, SLOT(formViewModeTriggered()));
    connect(m_viewToolBar, SIGNAL(tableViewModeSignal()),
            this, SLOT(tableViewModeTriggered()));
    connect(m_viewToolBar, SIGNAL(nextRecordSignal()),
            m_formView, SLOT(navigateNextRecord()));
    connect(m_viewToolBar, SIGNAL(previousRecordSignal()),
            m_formView, SLOT(navigatePreviousRecord()));
    connect(m_viewToolBar, SIGNAL(searchSignal(QString)),
            this, SLOT(searchSlot(QString)));

    //collection changing
    connect(m_metadataEngine, SIGNAL(currentCollectionIdChanged(int)),
            this, SLOT(currentCollectionIdChanged(int)));
    connect(m_metadataEngine, SIGNAL(currentCollectionChanged()),
            this, SLOT(currentCollectionChanged()));

    //table view -> form view
    connect(m_tableView, SIGNAL(recordEditFinished(int,int)),
            m_formView, SLOT(updateLastModified(int,int)));
}

void MainWindow::restoreSettings()
{
    restoreGeometry(m_settingsManager->restoreGeometry("mainWindow"));
    restoreState(m_settingsManager->restoreState("mainWindow"));

    //restore view mode
    m_currentViewMode = (ViewMode) m_settingsManager->restoreViewMode();
    if(m_currentViewMode == TableViewMode)
        tableViewModeTriggered();

    //restore last used record
    int last = m_settingsManager->restoreLastUsedRecord();
    if ((last != -1) && m_currentModel) { //if valid
        QModelIndex index = m_formView->model()->index(last, 1);
        if (index.isValid())
            m_tableView->setCurrentIndex(index);
    }

#ifdef Q_OS_OSX
    if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_7) {
        bool f = m_settingsManager->restoreProperty("macLionFullscreen", "mainWindow")
                .toBool();
        if (f) {
            //this is a workaround to activate fullsreen on mac without crashing on start
            QTimer::singleShot(100, this, SLOT(fullscreenActionTriggered()));
        }
    }
#endif //Q_OS_OSX

#ifdef Q_OS_LINUX
    if (m_settingsManager->restoreProperty("linuxDarkAmbianceToolbar", "mainWindow")
            .toBool()) {
        m_toolBar->setStyleSheet("QToolBar {background-color: qlineargradient(spread:pad,"
                                 " x1:0.5, y1:0.01, x2:0.5, y2:0.99, stop:0 rgba(65, 64, "
                                 "59, 255), stop:0.01 rgba(56, 55, 52, 255), stop:0.99 "
                                 "rgba(84, 82, 74, 255), stop:1 rgba(66, 65, 60, 255));} "
                                 "QToolBar:!active {background-color: rgb(60, 59, 55);}");
    }
#endif //Q_OS_LINUX

    /* not needed for passiflora at this point
    //update dock widget status (hidden/visisble) and toggleDockAction
    //dock status is saved as part of geometry so only menu sync is needed
    if (m_dockContainerWidget->isHidden()){
        m_toggleDockAction->setChecked(true);
    }*/
}

void MainWindow::saveSettings()
{
    m_settingsManager->saveGeometry("mainWindow", saveGeometry());
    m_settingsManager->saveState("mainWindow", saveState());
    m_settingsManager->saveSoftwareBuild();
    m_settingsManager->saveViewMode(m_currentViewMode);
    m_settingsManager->saveLastUsedRecord(m_formView->getCurrentRow());
}

void MainWindow::init()
{
#ifdef Q_OS_OSX
    setUnifiedTitleAndToolBarOnMac(true);
#endif // Q_OS_OSX
}

void MainWindow::attachModelToViews(const int collectionId)
{
    if (!collectionId) { //0 stands for invalid
        m_formView->setModel(0);
        m_tableView->setModel(0);
        return;
    }

    //if there are other collection type supported
    //add the code here to find the correct collection type
    //for now only standard type is supported
    MetadataEngine::CollectionType type = MetadataEngine::StandardCollection;

    //create model
    m_currentModel = m_metadataEngine->createModel(type, collectionId);
    if (!m_currentModel) return;

    //fetch all data from model (avoid lazy loading at startup)
    while(m_currentModel->canFetchMore(QModelIndex()))
        m_currentModel->fetchMore(QModelIndex());

    //set model on views
    m_formView->setModel(m_currentModel);
    m_tableView->setModel(m_currentModel);
    //share selection model between the two views
    QItemSelectionModel *old = m_tableView->selectionModel();
    m_tableView->setSelectionModel(m_formView->selectionModel());
    delete old;

    //if collection is empty
    //set form view as default
    //to show help messages
    if (!m_currentModel->rowCount())
        formViewModeTriggered();

    m_lastUsedCollectionId = collectionId;
}

void MainWindow::detachModelFromViews()
{
    //save form view if form widget has focus
    setFocus();

    //content data views
    m_formView->setModel(0);
    m_tableView->setModel(0);
    if (m_currentModel) {
        delete m_currentModel;
        m_currentModel = 0;
    }
}

void MainWindow::attachCollectionModelView()
{
    //collection list view
    CollectionListView *cv = m_dockWidget->getCollectionListView();
    QAbstractItemModel *cm = cv->model();
    if (!cm) {
        cv->attachModel();
    }
}

void MainWindow::detachCollectionModelView()
{
    //collection list view
    CollectionListView *cv = m_dockWidget->getCollectionListView();
    QAbstractItemModel *cm = cv->model();
    if (cm) {
        cv->detachModel();
    }
}
