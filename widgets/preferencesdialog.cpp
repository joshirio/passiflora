/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"
#include "../components/settingsmanager.h"
#include "../components/filemanager.h"
#include "../utils/definitionholder.h"


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog),
    m_settingsManager(0),
    m_softwareReset(false),
    m_appearanceChanged(false)
{
    ui->setupUi(this);

    //init
    ui->listWidget->setCurrentRow(0);
    m_settingsManager = new SettingsManager;

    //setup widgets
    initSettings();

    //load settings
    loadSettings();

    //connections
    connect(ui->closeButton, SIGNAL(clicked()),
            this, SLOT(accept()));
    connect(ui->listWidget, SIGNAL(currentRowChanged(int)),
            this, SLOT(currentCategoryChanged()));
    connect(ui->updatesComboBox, SIGNAL(activated(int)),
            this, SLOT(updatesComboBoxChanged()));
    connect(ui->softwareResetButton, SIGNAL(clicked()),
            this, SLOT(softwareResetButtonClicked()));
    connect(ui->formViewColorCombo, SIGNAL(activated(int)),
            this, SLOT(formViewColorComboChanged()));
    connect(ui->darkToolbarAmbianceCheckBox, SIGNAL(clicked()),
            this, SLOT(darkToolbarAmbianceCheckChanged()));

    if (DefinitionHolder::APP_STORE) {
        //disable updates
        ui->updateGroupBox->setEnabled(false);
    }

#ifndef Q_OS_LINUX
    //if not running linux, hide dark toolbar (ambiance) option
    //NOTE: if more option are added to the main window group box
    //      update this, else valid option are hidden
    ui->mainWindowGroupBox->setVisible(false);
#endif //Q_OS_LINUX
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;

    if (m_settingsManager)
        delete m_settingsManager;
}

bool PreferencesDialog::softwareResetActivated()
{
    return m_softwareReset;
}

bool PreferencesDialog::appearanceChanged()
{
    return m_appearanceChanged;
}


//-----------------------------------------------------------------------------
// Private slots
//-----------------------------------------------------------------------------

void PreferencesDialog::currentCategoryChanged()
{
    ui->stackedWidget->setCurrentIndex(ui->listWidget->currentRow());
}

void PreferencesDialog::updatesComboBoxChanged()
{
    m_settingsManager->saveCheckUpdates(
                ui->updatesComboBox->currentIndex() == 0);
}

void PreferencesDialog::softwareResetButtonClicked()
{
    m_softwareReset = true;
    accept();
}

void PreferencesDialog::formViewColorComboChanged()
{
    int colorIndex = ui->formViewColorCombo->currentIndex();
    QString colorCode = ui->formViewColorCombo->itemData(colorIndex).toString();

    m_settingsManager->saveProperty("backgroundColor", "formView", colorCode);
    m_settingsManager->saveProperty("backgroundColorIndex", "formView", colorIndex);

    m_appearanceChanged = true;
}

void PreferencesDialog::darkToolbarAmbianceCheckChanged()
{
    bool checked = ui->darkToolbarAmbianceCheckBox->isChecked();

    m_settingsManager->saveProperty("linuxDarkAmbianceToolbar", "mainWindow", checked);

    m_appearanceChanged = true;
}


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

void PreferencesDialog::initSettings()
{
    //form view background color
    QMap<QString, QString> colorMap;
    colorMap.insert("AliceBlue", "#F0F8FF");
    colorMap.insert("AntiqueWhite", "#FAEBD7");
    colorMap.insert("Beige", "#F5F5DC");
    colorMap.insert("Gainsboro", "#DCDCDC");
    colorMap.insert("Lavender", "#E6E6FA");
    colorMap.insert("LightCyan", "#E0FFFF");
    colorMap.insert("LightGoldenRodYellow", "#FAFAD2");
    colorMap.insert("LightGrey", "#D3D3D3");
    colorMap.insert("LightGrey2", "#E0E0E0");
    colorMap.insert("LightSkyBlue", "#87CEFA");
    colorMap.insert("LightSteelBlue", "#B0C4DE");
    colorMap.insert("LightYellow", "#FFFFE0");
    colorMap.insert("MistyRose", "#FFE4E1");
    colorMap.insert("Plum", "#DDA0DD");
    colorMap.insert("PowderBlue", "#B0E0E6");
    colorMap.insert("SeaShell", "#FFF5EE");
    colorMap.insert("SkyBlue", "#87CEEB");
    colorMap.insert("Violet", "#EE82EE");

    QMap<QString, QString>::const_iterator i = colorMap.constBegin();
    while (i != colorMap.constEnd()) {
        QPixmap pixmap(64, 64);
        pixmap.fill(QColor(i.value()));
        ui->formViewColorCombo->addItem(QIcon(pixmap),
                                        i.key(),
                                        i.value());
        ++i;
    }
}

void PreferencesDialog::loadSettings()
{
    if (!m_settingsManager->restoreCheckUpdates())
        ui->updatesComboBox->setCurrentIndex(1);

    //form view background color
    int colorCodeIndex = m_settingsManager->restoreProperty(
                "backgroundColorIndex", "formView").toInt();
    ui->formViewColorCombo->setCurrentIndex(colorCodeIndex);

#ifdef Q_OS_LINUX
    //dark toolbar ambiance style
    if (m_settingsManager->restoreProperty("linuxDarkAmbianceToolbar", "mainWindow").toBool())
        ui->darkToolbarAmbianceCheckBox->setChecked(true);
#endif //Q_OS_LINUX
}
