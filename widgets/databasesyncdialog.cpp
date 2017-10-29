/*
 *  Copyright (c) 2017 Giorgio Wicklein <giowckln@gmail.com>
 */

#include "databasesyncdialog.h"
#include "ui_databasesyncdialog.h"
#include "../components/settingsmanager.h"
#include "../components/filemanager.h"
#include "../utils/definitionholder.h"

#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QProgressBar>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonValue>
#include <QtCore/QFile>
#include <QtWidgets/QApplication>

DatabaseSyncDialog::DatabaseSyncDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DatabaseSyncDialog),
    m_updateManager(nullptr),
    m_latestPlantDbRevision(Q_UINT64_C(0))
{
    ui->setupUi(this);

    //set initial focus
    ui->licenseAcceptButton->setFocus();

    m_updateManager = &UpdateManager::getInstance();

    createConnections();
}

DatabaseSyncDialog::~DatabaseSyncDialog()
{
    delete ui;
    this->clearPlantImagesList();
}

void DatabaseSyncDialog::cancelLicenseButtonClicked()
{
    this->reject();
}

void DatabaseSyncDialog::acceptLicenseButtonClicked()
{
    ui->stackedWidget->setCurrentIndex(1);
    startDatabaseSync();
}

void DatabaseSyncDialog::syncCancelButtonClicked()
{
    this->reject();
}

void DatabaseSyncDialog::syncNextButtonClicked()
{
    //load image licensing info to view
    createImageLicensingAndDisplayResults();

    ui->stackedWidget->setCurrentIndex(2);
    ui->imageLicenseNextButton->setFocus();
}

void DatabaseSyncDialog::imageLicenseNextButtonClicked()
{
    ui->stackedWidget->setCurrentIndex(3);
    ui->notesFinishButton->setFocus();
}

void DatabaseSyncDialog::syncMetadataRequestResponse(UpdateManager::UpdateCheckMetadata *metadata)
{
    if (metadata == nullptr) return;

    //cache latest rev
    m_latestPlantDbRevision = metadata->plantDbRevision;

    //check min software build requirements
    if (metadata->plantDbMinBuild > ((quint64) DefinitionHolder::SOFTWARE_BUILD)) {
        QMessageBox::critical(this, tr("Software outdated!"),
                              tr("The new database format requires a newer software version."
                                 "<br />Please update your software version first!")
                                 #ifndef Q_OS_MACOS
                                 + tr("<br /><br />Go to menu: <b>Help->Check for updates</b>"),
                                 #else
                                 + tr("<br /><br />Go to menu: <b>%1->Check for updates</b>")
                                 .arg(DefinitionHolder::NAME),
                                 #endif
                              QMessageBox::Ok);
        this->reject();
    }

    //start db file download request
    ui->syncStatusLabel->setText(tr("data.db"));
    m_updateManager->requestPlantDatabaseFile();
}

void DatabaseSyncDialog::networkRequestError()
{
    this->reject();
}

void DatabaseSyncDialog::downloadProgressSlot(int progressPercent)
{
    ui->syncProgressBar->setRange(0, 100);
    ui->syncProgressBar->setValue(progressPercent);
}

void DatabaseSyncDialog::plantDatabaseUpdateSuccessSlot()
{
    ui->syncProgressBar->setRange(0, 0);
    ui->syncStatusLabel->setText(tr("Checking images..."));

    m_updateManager->requestPlantImgMetaFile();
}

void DatabaseSyncDialog::plantImgMetadataFileReadySlot()
{
    ui->syncStatusLabel->setText(tr("Parsing images..."));
    ui->syncProgressBar->setRange(0, 0);

    //parse json file
    FileManager fm(this);
    QFile jsonFile(fm.getPlantImageMetaJsonFilePath());
    bool error = false;
    QString errorMessage = "";

    if (!jsonFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        error = true;
        errorMessage.append(tr("Filed to open image metadata json file. "));
    } else {
        QString raw = jsonFile.readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(raw.toUtf8());
        jsonFile.close();
        if (jsonDoc.isNull()) {
            error = true;
            errorMessage.append(tr("Invalid json format. "));
        } else {
            //parse json and check missing img files to download
            parseImageMetadataJson(jsonDoc);
            downloadPlantImages();
        }
    }

    if (error) {
        QMessageBox::critical(this, tr("Plant Database Metadata Error"),
                              tr("Error: %1").arg(errorMessage),
                              QMessageBox::Ok);
        this->reject();
    }
}

void DatabaseSyncDialog::plantImgMetadataFileErrorSlot()
{
    this->reject();
}

void DatabaseSyncDialog::plantImgFileDownloadStartedSlot(const QString &fileName)
{
    QString s = fileName;
    s.truncate(7);
    s.append("...");
    s.append(QString(fileName).remove(0, fileName.length() - 7));
    ui->syncStatusLabel->setText(s);
    ui->syncProgressBar->setValue(0);
}

void DatabaseSyncDialog::totalSyncProgressMaxStepsSlot(int t)
{
    ui->syncTotalProgressBar->setRange(0, t);
}

void DatabaseSyncDialog::incrementTotalProgressStepSlot()
{
    ui->syncTotalProgressBar->setValue(ui->syncTotalProgressBar->value() + 1);
}

void DatabaseSyncDialog::plantImgWriteFileErrorSlot()
{
    this->reject();
}

void DatabaseSyncDialog::allImageFilesDownloadedSlot()
{
    ui->syncTotalProgressBar->setRange(0, 0);
    ui->syncProgressBar->setRange(0, 0);
    ui->syncStatusLabel->setText(tr("Loading changes..."));

    m_updateManager->requestPlantDbChangelogFile();
}

void DatabaseSyncDialog::plantDbChangelogRequestCompletedSlot(const QString &html)
{
    ui->updateNotesLabel->setText(html);

    //download event notices
    ui->syncTotalProgressBar->setRange(0, 0);
    ui->syncProgressBar->setRange(0, 0);
    ui->syncStatusLabel->setText(tr("Loading events..."));

    m_updateManager->requestPlantDbEventsFile();
}

void DatabaseSyncDialog::plantDbEventsRequestCompletedSlot(const QString &html)
{
    ui->eventNotesLabel->setText(html);

    this->setDatabaseDownloadComplete();
}

void DatabaseSyncDialog::createConnections()
{
    connect(ui->licenseCancelButton, &QPushButton::clicked,
            this, &DatabaseSyncDialog::cancelLicenseButtonClicked);
    connect(ui->licenseAcceptButton, &QPushButton::clicked,
            this, &DatabaseSyncDialog::acceptLicenseButtonClicked);
    connect(ui->syncCancelButton, &QPushButton::clicked,
            this, &DatabaseSyncDialog::syncCancelButtonClicked);
    connect(ui->syncNextButton, &QPushButton::clicked,
            this, &DatabaseSyncDialog::syncNextButtonClicked);
    connect(ui->notesFinishButton, &QPushButton::clicked,
            this, &DatabaseSyncDialog::accept);
    connect(ui->imageLicenseCancelButton, &QPushButton::clicked,
            this, &DatabaseSyncDialog::reject);
    connect(ui->imageLicenseNextButton, &QPushButton::clicked,
            this, &DatabaseSyncDialog::imageLicenseNextButtonClicked);

    //update manager
    connect(m_updateManager, SIGNAL(latestUpdateMedataRequestReady(UpdateManager::UpdateCheckMetadata*)),
            this, SLOT(syncMetadataRequestResponse(UpdateManager::UpdateCheckMetadata*)));
    connect(m_updateManager, SIGNAL(networkRequestError()),
            this, SLOT(networkRequestError()));
    connect(m_updateManager, SIGNAL(downloadProgressSignal(int)),
            this, SLOT(downloadProgressSlot(int)));
    connect (m_updateManager, SIGNAL(plantDatabaseUpdateError()),
             this, SLOT(networkRequestError()));
    connect(m_updateManager, SIGNAL(plantDatabaseUpdateSuccess()),
            this, SLOT(plantDatabaseUpdateSuccessSlot()));
    connect(m_updateManager, SIGNAL(plantImgMetadataFileReady()),
            this, SLOT(plantImgMetadataFileReadySlot()));
    connect(m_updateManager, SIGNAL(plantImgMetadataFileError()),
            this, SLOT(plantImgMetadataFileErrorSlot()));
    connect(m_updateManager, SIGNAL(plantImgFileDownloadStarted(QString)),
            this, SLOT(plantImgFileDownloadStartedSlot(QString)));
    connect(m_updateManager, SIGNAL(totalSyncProgressMaxStepsSignal(int)),
            this, SLOT(totalSyncProgressMaxStepsSlot(int)));
    connect(m_updateManager, SIGNAL(incrementTotalProgressStepSignal()),
            this, SLOT(incrementTotalProgressStepSlot()));
    connect(m_updateManager, SIGNAL(plantImgWriteFileError()),
            this, SLOT(plantImgWriteFileErrorSlot()));
    connect(m_updateManager, SIGNAL(allImageFilesDownloadedSignal()),
            this, SLOT(allImageFilesDownloadedSlot()));
    connect(m_updateManager, SIGNAL(plantDbChangelogRequestCompleted(QString)),
            this, SLOT(plantDbChangelogRequestCompletedSlot(QString)));
    connect(m_updateManager, SIGNAL(plantDbEventsRequestCompleted(QString)),
            this, SLOT(plantDbEventsRequestCompletedSlot(QString)));
}

void DatabaseSyncDialog::startDatabaseSync()
{
    ui->syncStatusLabel->setText(tr("Checking revision..."));

    m_updateManager->requestLatestUpdateMetadata();

    //set incomplete sync state
    SettingsManager sm;
    sm.saveLastPlantDatabaseSyncAborted(true);
}

void DatabaseSyncDialog::parseImageMetadataJson(const QJsonDocument &jsonDoc)
{
    this->clearPlantImagesList();

    QJsonObject jsonObject = jsonDoc.object();
    QJsonArray jsonArray = jsonObject["plant"].toArray();

    foreach (const QJsonValue &value, jsonArray) {
        UpdateManager::PlantImageMetadata *m = new UpdateManager::PlantImageMetadata;
        QJsonObject obj = value.toObject();
        m->plantName = obj["name"].toString();
        m->licenseString = obj["license html"].toString();
        m->filename = obj["database file name"].toString();
        m_plantImagesList.append(m);
    }
}

void DatabaseSyncDialog::clearPlantImagesList()
{
    foreach (UpdateManager::PlantImageMetadata *m, m_plantImagesList) {
        delete m;
    }
    m_plantImagesList.clear();
}

void DatabaseSyncDialog::downloadPlantImages()
{
    //check missing files
    QStringList filesToDownload;

    FileManager fm(this);
    QStringList localFileList = fm.getAllLocalFiles();

    foreach (UpdateManager::PlantImageMetadata *m, m_plantImagesList) {
        if (!localFileList.contains(m->filename)) {
            filesToDownload.append(m->filename);
        }
    }

    m_updateManager->requestPlantImageFiles(filesToDownload);
}

void DatabaseSyncDialog::setDatabaseDownloadComplete()
{
    SettingsManager sm;
    sm.saveLastPlantDatabaseSyncAborted(false);

    //remove obsolete files
    ui->syncStatusLabel->setText(tr("Removing old files..."));
    qApp->processEvents();
    FileManager fm(this);
    QString filesDir = fm.getFilesDirectory();
    QStringList obsoleteFileList = fm.unneededLocalFileList();
    foreach (QString s, obsoleteFileList) {
        QFile::remove(filesDir + s);
    }

    ui->downloadingTitleLabel->setText("Download complete!");
    ui->syncStatusLabel->setText(tr("Completed!"));
    ui->syncTotalProgressBar->setRange(0, 100);
    ui->syncProgressBar->setRange(0, 100);
    ui->syncTotalProgressBar->setValue(100);
    ui->syncProgressBar->setValue(100);
    ui->syncNextButton->setEnabled(true);
    ui->syncNextButton->setFocus();
}

void DatabaseSyncDialog::createImageLicensingAndDisplayResults()
{
    ui->imageLicenseLabel->clear();

    //create html string
    FileManager fm(this);
    QString filesDir = fm.getFilesDirectory();
    QString htmlString;
    htmlString.append("<table>");
    foreach (UpdateManager::PlantImageMetadata *p, m_plantImagesList) {
        htmlString.append("<tr><td>");
        htmlString.append("<a href=\"file://" + filesDir + p->filename
                          + tr("\">Click<br />to view</a></td>"));
        htmlString.append("<td><b>" + p->plantName + "</b><br />");
        htmlString.append(p->licenseString);
        htmlString.append("<br />" + p->filename + "<br />");
        htmlString.append("</td></tr>");
    }
    htmlString.append("</table>");

    ui->imageLicenseLabel->setText(htmlString);
}
