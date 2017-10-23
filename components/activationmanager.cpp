/*
 *  Copyright (c) 2017 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "activationmanager.h"
#include "components/settingsmanager.h"

#include <QtCore/QCryptographicHash>


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

ActivationManager::ActivationManager(QObject *parent) : QObject(parent)
{

}

bool ActivationManager::isActiveAndValid() const
{
    QString license, name, email;
    SettingsManager sm;
    sm.restoreLicenseKey(license, name, email);

    return checkLicenseKey(license, name, email);
}

bool ActivationManager::activateSoftware(const QString &keyString,
                                         const QString &nameString,
                                         const QString &emailString)
{
    bool r = false;

    if (checkLicenseKey(keyString, nameString, emailString)) {
        SettingsManager sm;
        sm.saveLicenseKey(keyString, nameString, emailString);
        r = true;
    }

    return r;
}


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

bool ActivationManager::checkLicenseKey(const QString &keyString,
                                        const QString &nameString,
                                        const QString &emailString) const
{
    QByteArray cH, rS, eS;
    cH = QCryptographicHash::hash(
                QCryptographicHash::hash(nameString.toLatin1(),
                                         QCryptographicHash::Sha1)
                .append(QCryptographicHash::hash(emailString.toLatin1(),
                                                 QCryptographicHash::Sha1)),
                QCryptographicHash::Sha1);

   for (int i = 0; i < cH.length(); ++i) {
       if (!(i % 3)) {
           rS.append(cH.at(i));
       }
   }

   for (int j = 0; j < rS.length(); ++j) {
       QChar c = rS.at(j);
       switch (c.toLatin1()) {
       case 'a':
           eS.append('x');
           break;
       case 'b':
           eS.append('y');
           break;
       case 'c':
           eS.append('m');
           break;
       case 'd':
           eS.append('n');
           break;
       case 'e':
           eS.append('h');
           break;
       case 'f':
           eS.append('z');
           break;
       case '0':
           eS.append('p');
           break;
       case '1':
           eS.append('w');
           break;
       case '2':
           eS.append('c');
           break;
       case '3':
           eS.append('u');
           break;
       default:
           eS.append(c);
           break;
       }
   }

   return QString(eS).toUpper() == keyString;
}
