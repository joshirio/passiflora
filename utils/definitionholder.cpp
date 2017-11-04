/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "definitionholder.h"

#include <QtCore/QString>
#include <QtCore/QDate>


//-----------------------------------------------------------------------------
// Static variables initialization
//-----------------------------------------------------------------------------

QString DefinitionHolder::VERSION = "2.3";
QString DefinitionHolder::NAME = "Spagyrik Datenbank Passiflora";
QString DefinitionHolder::COMPANY = "enmed";
QString DefinitionHolder::DOMAIN_NAME = "enmed.de";
QString DefinitionHolder::UPDATE_URL = "http://passiflora.enmed.de/updates_raw/updates";
QString DefinitionHolder::PLANT_DB_URL = "http://passiflora.enmed.de/updates_raw/data.db";
QString DefinitionHolder::PLANT_DB_NOTICE = "http://passiflora.enmed.de/updates_raw/dbnotice";
QString DefinitionHolder::PLANT_DB_CHANGELOG = "http://passiflora.enmed.de/updates_raw/dbchangelog";
QString DefinitionHolder::PLANT_DB_IMG_URL = "http://passiflora.enmed.de/updates_raw/images/";
QString DefinitionHolder::PLANT_DB_IMG_META_URL = "http://passiflora.enmed.de/updates_raw/plantimagesmeta.json";
QString DefinitionHolder::DOWNLOAD_URL = "http://passiflora.enmed.de/update/";
int DefinitionHolder::SOFTWARE_BUILD = 10;
int DefinitionHolder::DATABASE_VERSION = 2;
bool DefinitionHolder::APP_STORE = false;
QString DefinitionHolder::COPYRIGHT =
        QString("Copyright &copy; 2014-%1 Giorgio Wicklein"
                "<br />Copyright &copy; 2012-2014 GIOWISYS Software UG (haftungsbeschr%2nkt)")
        .arg(QDate::currentDate().toString("yyyy"))
        .arg(QChar(228)); // 228=umlaut a

