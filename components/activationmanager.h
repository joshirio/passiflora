/**
  * \class ActivationManager
  * \brief Handles software license key activation and checking.
  * \author Giorgio Wicklein
  * \date 23/10/2017
  */

#ifndef ACTIVATIONMANAGER_H
#define ACTIVATIONMANAGER_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include <QObject>


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// ActivationManager
//-----------------------------------------------------------------------------

class ActivationManager : public QObject
{
    Q_OBJECT
public:
    explicit ActivationManager(QObject *parent = nullptr);

    /** Check if software has been activated successfully */
    bool isActiveAndValid() const;

    /** Check and activate the software with
     * the given license code if valid
     * @param keyString - the license activation code
     * @param nameString - the name used on registration
     * @param emailString - email used on registration
     * @return success or failure
     */
    bool activateSoftware(const QString &keyString,
                          const QString &nameString,
                          const QString &emailString);

signals:

public slots:

private:
    bool checkLicenseKey(const QString &keyString,
                          const QString &nameString,
                          const QString &emailString) const;
};

#endif // ACTIVATIONMANAGER_H
