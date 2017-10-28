/**
  * \class DefinitionHolder
  * \brief This class holds global definitions relevant to Passiflora App
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 08/04/2012
  */

#ifndef DEFINITIONHOLDER_H
#define DEFINITIONHOLDER_H


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

class QString;


//-----------------------------------------------------------------------------
// DefinitionHolder
//-----------------------------------------------------------------------------

class DefinitionHolder
{
public:
    static QString VERSION;               /**< Software version                    */
    static QString NAME;                  /**< Software name                       */
    static QString COPYRIGHT;             /**< Software copyright                  */
    static QString COMPANY;               /**< Company name used in settings       */
    static QString DOMAIN_NAME;           /**< Domain name used in settings        */
    static QString UPDATE_URL;            /**< Url where to check for updates      */
    static QString DOWNLOAD_URL;          /**< Url where to download the software  */
    static QString PLANT_DB_URL;          /**< Url to the plant databse            */
    static QString PLANT_DB_NOTICE;       /**< Url to the plant database notice    */
    static QString PLANT_DB_CHANGELOG;    /**< Url to the plant database changelog */
    static QString PLANT_DB_IMG_URL;      /**< Base url to the plant images dir    */
    static QString PLANT_DB_IMG_META_URL; /**< Url to the plant db image meta file */
    static int SOFTWARE_BUILD;            /**< Build no. of the software           */
    static int DATABASE_VERSION;          /**< Version no. of the database         */
    static bool APP_STORE;                /**< Deployment target is an app store   */

private:
    DefinitionHolder() {} //static only
};

#endif // DEFINITIONHOLDER_H
