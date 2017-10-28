/*
 *  Copyright (c) 2011  Giorgio Wicklein <g.wicklein@giowisys.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "updatemanager.h"
#include "../utils/definitionholder.h"
#include "../components/settingsmanager.h"
#include "../components/filemanager.h"
#include "../components/databasemanager.h"

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtCore/QStringList>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QApplication>
#include <QtGui/QDesktopServices>
#include <QtCore/QFile>


//-----------------------------------------------------------------------------
// Static init
//-----------------------------------------------------------------------------

UpdateManager* UpdateManager::m_instance = 0;


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

UpdateManager& UpdateManager::getInstance()
{
    if (!m_instance)
        m_instance = new UpdateManager();
    return *m_instance;
}

void UpdateManager::destroy()
{
    if (m_instance)
        delete m_instance;
    m_instance = 0;
}

void UpdateManager::checkForUpdates()
{
    startNetworkRequest(QUrl(DefinitionHolder::UPDATE_URL),
                        UpdateRequestOperation::CheckUpdatesOp);
}

void UpdateManager::requestLatestUpdateMetadata()
{
    startNetworkRequest(QUrl(DefinitionHolder::UPDATE_URL),
                        UpdateRequestOperation::UpdatesMetadataDownloadOp);
}

void UpdateManager::requestPlantDatabaseFile()
{
    startNetworkRequest(QUrl(DefinitionHolder::PLANT_DB_URL),
                        UpdateRequestOperation::PlantDbFileDownloadOp);
}

void UpdateManager::requestPlantImgMetaFile()
{
    startNetworkRequest(QUrl(DefinitionHolder::PLANT_DB_IMG_META_URL),
                        UpdateRequestOperation::PlantImageLicenseMeta);
}

void UpdateManager::requestPlantImageFiles(const QStringList &fileList)
{
    m_toDownloadImageFiles.clear();
    m_toDownloadImageFiles = fileList;

    if (!fileList.isEmpty()) {
        //notify how many files for total update progress
        emit totalSyncProgressMaxStepsSignal(fileList.size());

        downloadImageFile(m_toDownloadImageFiles.first());
    } else {
        emit allImageFilesDownloadedSignal();
    }
}

void UpdateManager::requestPlantDbChangelogFile()
{
    startNetworkRequest(QUrl(DefinitionHolder::PLANT_DB_CHANGELOG),
                        UpdateRequestOperation::PlantDbChangelogFileDownloadOp);
}

void UpdateManager::requestPlantDbEventsFile()
{
    startNetworkRequest(QUrl(DefinitionHolder::PLANT_DB_NOTICE),
                        UpdateRequestOperation::PlantDbEventsFileDownloadOp);
}


//-----------------------------------------------------------------------------
// Private slots
//-----------------------------------------------------------------------------

void UpdateManager::updateResponseSlot(QNetworkReply *reply)
{
    //cache and reset op at the beginning
    //because signals, emitted later, could start new nested requests
    UpdateRequestOperation currentOp = m_currentUpdateRequestOp;
    m_currentUpdateRequestOp = UpdateRequestOperation::NoOp;

    if (reply->error() && (currentOp != UpdateRequestOperation::CheckUpdatesOp)) {
        QMessageBox::critical(0, tr("Network Request Error"),
                              tr("Error code: %1"
                                 "<br />").arg(reply->error())
                              .append(reply->errorString()),
                              QMessageBox::Ok);
        emit networkRequestError();

    } else if (currentOp == UpdateRequestOperation::CheckUpdatesOp) {
        QString tmp(reply->readAll());

        if (tmp.isEmpty()) {
            emit updateErrorSignal();
            return;
        }

        //parse update response metadata
        if (!parseUpdateMetadata(tmp)) {
            emit updateErrorSignal();
            return;
        }

        //check updates
        bool softwareUpdate = m_currentUpdateCheckMetadata->softwareBuild >
                ((quint64) DefinitionHolder::SOFTWARE_BUILD);
        SettingsManager sm;
        bool plantDbUpdate = m_currentUpdateCheckMetadata->plantDbRevision >
                sm.restorePlantDatabaseVersion();

        //if last plant db sync process was incomplete (aborted, cancelled or error)
        //try again
        if (sm.restoresaveLastPlantDatabaseSyncAborted()) {
            plantDbUpdate = true;
        }

        if (softwareUpdate) {
            int r = QMessageBox::question(0, tr("New Software Version"),
                                          tr("A new version of %1 is available!<br>"
                                             "Do you want to download the new version?")
                                          .arg(DefinitionHolder::NAME),
                                          QMessageBox::Yes | QMessageBox::No,
                                          QMessageBox::Yes);

            if (r == QMessageBox::Yes) {
                QUrl downloadUrl(DefinitionHolder::DOWNLOAD_URL);
                QDesktopServices::openUrl(downloadUrl);
                emit updatesAccepted();
            }

        } else if (plantDbUpdate) {
            //only if software build can handle db format
            //notify plant db available
            if (m_currentUpdateCheckMetadata->plantDbMinBuild <= ((quint64) DefinitionHolder::SOFTWARE_BUILD)) {
                emit plantDatabaseUpdateSinal();
            }
        } else {
            //no updates available
            emit noUpdateSignal();
        }

    } else if (currentOp == UpdateRequestOperation::UpdatesMetadataDownloadOp) {
        emit latestUpdateMedataRequestReady(m_currentUpdateCheckMetadata);
    } else if (currentOp == UpdateRequestOperation::PlantImageFileDownloadOp) {
        QString fileName = reply->url().fileName();

        FileManager fm;
        QFile file(fm.getFilesDirectory() + fileName);
        if (!file.open(QFile::WriteOnly)) { //truncates when writing
            QMessageBox::critical(0, tr("Plant Image Writing Error"),
                                  tr("Error: writing image file %1").arg(fileName),
                                     QMessageBox::Ok);
            emit plantImgWriteFileError();
        } else {
            file.write(reply->readAll());
            file.close();

            //remove from list
            m_toDownloadImageFiles.removeOne(fileName);

            //step completed
            incrementTotalProgressStepSignal();

            //continue download
            if (!m_toDownloadImageFiles.isEmpty()) {
                downloadImageFile(m_toDownloadImageFiles.first());
            } else {
                emit allImageFilesDownloadedSignal();
            }
        }

    } else if (currentOp == UpdateRequestOperation::PlantImageLicenseMeta) {
        //write json file
        FileManager fm(this);
        QFile imgJsonFile(fm.getPlantImageMetaJsonFilePath());
        if (!imgJsonFile.open(QFile::WriteOnly)) { //truncates when writing
            QMessageBox::critical(0, tr("Plant Image Metadata Error"),
                                  tr("Error: writing database image metadata file!"),
                                     QMessageBox::Ok);
            emit plantImgMetadataFileError();
        } else {
            imgJsonFile.write(reply->readAll());
            imgJsonFile.close();
            emit plantImgMetadataFileReady();
        }
    } else if (currentOp == UpdateRequestOperation::PlantDbFileDownloadOp) {
        DatabaseManager *dbm = &DatabaseManager::getInstance();
        bool error, backupDone = false;
        QString errorString = "";
        QString dbFilePath = dbm->getDatabasePath();
        dbm->destroy(); //close db connection

        //replace old backup
        if (QFile::exists(dbFilePath + ".backup")) {
            if (!QFile::remove(dbFilePath + ".backup")){
                error = true;
                errorString.append("Failed to remove datababase backup file. ");
            }
        }
        backupDone = QFile::copy(dbFilePath, dbFilePath + ".backup");
        if (!backupDone) {
            error = true;
            errorString.append("Failed to create datababase backup file. ");
        }

        //overwrite db file
        QFile dbFile(dbFilePath);
        if (!dbFile.open(QFile::WriteOnly)) { //truncates when writing
            error = true;
            errorString.append("Failed to write datababase file. ");
        } else {
            dbFile.write(reply->readAll());
            dbFile.close();
        }

        if (error) {
            QMessageBox::critical(0, tr("Plant Database Update Error"),
                                  tr("Plant database was not updated!"
                                     "<br />%1").arg(errorString),
                                  QMessageBox::Ok);

            //restore backup
            if (backupDone) {
                QFile::remove(dbFilePath);
                QFile::copy(dbFilePath + ".backup", dbFilePath);
            }

            emit plantDatabaseUpdateError();
        } else {
            SettingsManager sm;
            sm.savePlantDatabaseVersion(m_currentUpdateCheckMetadata->plantDbRevision);
            emit plantDatabaseUpdateSuccess();
        }

        dbm->getInstance(); //open db

    } else if (currentOp == UpdateRequestOperation::PlantDbChangelogFileDownloadOp) {
        QString changelog = reply->readAll();
        emit plantDbChangelogRequestCompleted(changelog);
    } else if (currentOp == UpdateRequestOperation::PlantDbEventsFileDownloadOp) {
        QString notices = reply->readAll();
        emit plantDbEventsRequestCompleted(notices);
    } else {
        //unknown request op
    }

    //delete reply
    reply->deleteLater();
}

void UpdateManager::downloadProgressSlot(qint64 bytesReceived,
                                         qint64 bytesTotal)
{
    long double recB = bytesReceived;
    long double totB = bytesTotal;
    long double progressPercent = recB / totB * ((long double) 100.0);
    emit downloadProgressSignal((int) progressPercent);
}

//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

UpdateManager::UpdateManager(QObject *parent) :
    QObject(parent), m_currentUpdateRequestOp(NoOp)
{
    m_accessManager = new QNetworkAccessManager(this);

    m_currentUpdateCheckMetadata = new UpdateCheckMetadata;
    m_currentUpdateCheckMetadata->softwareBuild = 0;
    m_currentUpdateCheckMetadata->plantDbMinBuild = 0;
    m_currentUpdateCheckMetadata->plantDbRevision = 0;

    connect(m_accessManager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(updateResponseSlot(QNetworkReply*)));
}

UpdateManager::~UpdateManager()
{
    delete m_currentUpdateCheckMetadata;
}

bool UpdateManager::parseUpdateMetadata(const QString &s)
{
    /* Update response format:
     * softwareBuild;plantDbRevision;plantDbMinBuild
     *
     */

    QStringList l = s.split(";", QString::SkipEmptyParts);
    if (l.size() < 3) {
        return false;
    } else {
        bool a, b, c;
        m_currentUpdateCheckMetadata->softwareBuild = l.at(0).toULongLong(&a);
        m_currentUpdateCheckMetadata->plantDbRevision = l.at(1).toULongLong(&b);
        m_currentUpdateCheckMetadata->plantDbMinBuild = l.at(2).toULongLong(&c);
        if (!a && !b && !c) {
            return false;
        }
    }

    return true;
}

void UpdateManager::startNetworkRequest(const QUrl &url,
                                        const UpdateRequestOperation &op)
{
    QNetworkReply *reply = m_accessManager->get(QNetworkRequest(url));
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
            this, SLOT(downloadProgressSlot(qint64,qint64)));

    m_currentUpdateRequestOp = op;
}

void UpdateManager::downloadImageFile(const QString &filename)
{
    emit (plantImgFileDownloadStarted(filename));
    startNetworkRequest(QUrl(DefinitionHolder::PLANT_DB_IMG_URL + filename),
                        PlantImageFileDownloadOp);
}
