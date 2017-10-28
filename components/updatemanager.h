/**
  * \class UpdateManager
  * \brief This class handles software and plant database updates.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 13/10/2012
  */

#ifndef UPDATEMANAGER_H
#define UPDATEMANAGER_H

//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include <QtCore/QObject>


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

class QNetworkAccessManager;
class QNetworkReply;


//-----------------------------------------------------------------------------
// UpdateManager
//-----------------------------------------------------------------------------

class UpdateManager : public QObject
{
    Q_OBJECT

public:
    /** Structure of the update check response */
    struct UpdateCheckMetadata {
        quint64 softwareBuild; /**< Software build number */
        quint64 plantDbRevision; /**< Plant Database revision number */
        quint64 plantDbMinBuild; /**< The minimum required build number
                                          to open the plant database file */
    };

    /** Structure of the image metadata json file */
    struct PlantImageMetadata {
        QString plantName;
        QString licenseString;
        QString filename;
    };

    static UpdateManager& getInstance(); //singleton
    static void destroy();

    /** Start checking for updates, emits signals*/
    void checkForUpdates();

    /** Check for latest update metadata,
     *  emits latestUpdateMedataRequestReady
     */
    void requestLatestUpdateMetadata();

    /** Start plant db download request */
    void requestPlantDatabaseFile();

    /** Start plant image meta file download request */
    void requestPlantImgMetaFile();

    /** Start plant images download
     * @param fileList - list of image files to download
     */
    void requestPlantImageFiles(const QStringList &fileList);

    /** Start request to download plant database changelog file */
    void requestPlantDbChangelogFile();

    /** Start request to download plant database events notice file */
    void requestPlantDbEventsFile();

signals:
    /** No new software update found */
    void noUpdateSignal();

    /** A new plant database update is available */
    void plantDatabaseUpdateSinal();

    /** Error during update check */
    void updateErrorSignal();

    /** Error during network request */
    void networkRequestError();

    /** Plant database was not updated */
    void plantDatabaseUpdateError();

    /** Plant database successfully updated */
    void plantDatabaseUpdateSuccess();

    /** Error on plant db img metadata file write */
    void plantImgMetadataFileError();

    /** Successfully downloaded img metadata file */
    void plantImgMetadataFileReady();

    /** Software update available and user accepted to upgrade */
    void updatesAccepted();

    /** This signal is emitted on latest update metadata request completion */
    void latestUpdateMedataRequestReady(UpdateManager::UpdateCheckMetadata *metadata);

    /** This signal is emitted when there's download progress
     * @param percentCompleted - percent of download completed
     */
    void downloadProgressSignal(int percentCompleted);

    /** Info about how many total steps for the update process are required */
    void totalSyncProgressMaxStepsSignal(int);

    /** The total progress of the update process got 1 step closer */
    void incrementTotalProgressStepSignal();

    /** All plant database image files have been downloaded */
    void allImageFilesDownloadedSignal();

    /** Error while file image writing operation after download */
    void plantImgWriteFileError();

    /** Emitted when the specified image file download request has been started */
    void plantImgFileDownloadStarted(const QString &fileName);

    /** Plant database changelog file downloaded and html ready to read */
    void plantDbChangelogRequestCompleted(const QString &changelogHtmlString);

    /** Plant database events notice file downloaded and html ready to read */
    void plantDbEventsRequestCompleted(const QString &eventsHtmlString);

private slots:
    void updateResponseSlot(QNetworkReply*);
    void downloadProgressSlot(qint64 bytesReceived, qint64 bytesTotal);

private:
    UpdateManager(QObject *parent = 0); //singleton
    ~UpdateManager();

    /** Enum of update/download operation requests */
    enum UpdateRequestOperation {
        NoOp, /**< No operation going on */
        CheckUpdatesOp, /**< Checking for software and plant database updates */
        UpdatesMetadataDownloadOp, /** Download updates metadata only */
        PlantDbFileDownloadOp, /**< Plant database file download  */
        PlantImageFileDownloadOp, /**< Plant image download */
        PlantDbChangelogFileDownloadOp, /**< Update changelog file download */
        PlantImageLicenseMeta, /**< Img license meta file doiwnload */
        PlantDbEventsFileDownloadOp /**< Updates events notice file download */
    };

    /** Parse raw update file from network response into m_currentUpdateCheckMetadata
     * @return true if successful parsed, false parsing error
     */
    bool parseUpdateMetadata(const QString &s);

    /** Start network get request and connect appropriate signals (progress download)
     * @param url - the url to download
     * @param op - the UpdateRequestOperation enum
     */
    void startNetworkRequest(const QUrl &url,
                             const UpdateManager::UpdateRequestOperation &op);
    void downloadImageFile(const QString &filename);

    UpdateRequestOperation m_currentUpdateRequestOp;
    QNetworkAccessManager *m_accessManager;
    UpdateCheckMetadata *m_currentUpdateCheckMetadata;
    QStringList m_toDownloadImageFiles;
    static UpdateManager *m_instance; //make singleton,
                                      //cause QNetworkAccessmanager
                                      //should be a single instance across the app
};

#endif // UPDATEMANAGER_H
