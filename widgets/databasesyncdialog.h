/**
  * \class DatabaseSyncDialog
  * \brief Dialog to handle plant database updates and downloads with license info.
  * \author Giorgio Wicklein
  * \date 24/10/2017
  */

#ifndef DATABASESYNCDIALOG_H
#define DATABASESYNCDIALOG_H

#include "components/updatemanager.h"
#include <QDialog>

namespace Ui {
class DatabaseSyncDialog;
}

class DatabaseSyncDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DatabaseSyncDialog(QWidget *parent = 0);
    ~DatabaseSyncDialog();

private slots:
    void cancelLicenseButtonClicked();
    void acceptLicenseButtonClicked();
    void syncCancelButtonClicked();
    void syncNextButtonClicked();
    void imageLicenseNextButtonClicked();
    void syncMetadataRequestResponse(UpdateManager::UpdateCheckMetadata* metadata);
    void networkRequestError();
    void downloadProgressSlot(int progressPercent);
    void plantDatabaseUpdateSuccessSlot();
    void plantImgMetadataFileReadySlot();
    void plantImgMetadataFileErrorSlot();
    void plantImgFileDownloadStartedSlot(const QString &fileName);
    void totalSyncProgressMaxStepsSlot(int t);
    void incrementTotalProgressStepSlot();
    void plantImgWriteFileErrorSlot();
    void allImageFilesDownloadedSlot();
    void plantDbChangelogRequestCompletedSlot(const QString &html);
    void plantDbEventsRequestCompletedSlot(const QString &html);

private:
    void createConnections();
    void startDatabaseSync();
    void parseImageMetadataJson(const QJsonDocument &jsonDoc);
    void clearPlantImagesList();
    void downloadPlantImages();
    void setDatabaseDownloadComplete();
    void createImageLicensingAndDisplayResults();

    Ui::DatabaseSyncDialog *ui;
    UpdateManager *m_updateManager;
    quint64 m_latestPlantDbRevision;
    QList<UpdateManager::PlantImageMetadata*> m_plantImagesList;
};

#endif // DATABASESYNCDIALOG_H
