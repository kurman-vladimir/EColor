#include "ecolorcore.h"
#include "about.h"
#include "settings.h"
#include "dialogsettings.h"
#include "dialogupdate.h"
#include "versionchecker.h"
#include "popupcolor.h"
#include "popupmessage.h"
#include "nativeeventfilter.h"
#include <QMenu>
#include <QAction>
#include <QFont>
#include <QSettings>
#include <QDir>
#include <QApplication>

EColorCore::EColorCore(QObject *parent) : QObject(parent)
{
    QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);
    if(!settings.contains(KEY_SEQUENCE_PIXEL)){
        settings.setValue(KEY_SEQUENCE_PIXEL, QKeySequence("Ctrl+E").toString());
        settings.sync();
    }
    if(!settings.contains(SETTINGS_PATH_SCREENSHOTS)){
        settings.setValue(SETTINGS_PATH_SCREENSHOTS, QDir::currentPath());
        settings.sync();
    }

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setToolTip("EColor \n"
                         "Приложение для захвата цвета пикселя \n"
                         "на экране монитора");
    trayIcon->setIcon(QIcon(":/images/ecolor.png"));

    QMenu * menu = new QMenu();
    QAction *actionShow = new QAction(trUtf8("Показать"), this);
    QAction *actionConfig = new QAction(trUtf8("Настройки"), this);
    QAction *actionAbout = new QAction(trUtf8("О Приложении"), this);
    QAction *actionQuit = new QAction(trUtf8("Выход"), this);

    connect(actionShow, &QAction::triggered, this, &EColorCore::showTriggered);
    connect(actionConfig, &QAction::triggered, this, &EColorCore::configTriggered);
    connect(actionAbout, &QAction::triggered, this, &EColorCore::aboutTriggered);
    connect(actionQuit, &QAction::triggered, this, &EColorCore::quitTriggered);

    menu->addAction(actionShow);
    menu->addAction(actionConfig);
    menu->addAction(actionAbout);
    menu->addSeparator();
    menu->addAction(actionQuit);

    trayIcon->setContextMenu(menu);
    trayIcon->show();

#ifdef Q_OS_WIN32
    connect(trayIcon, &QSystemTrayIcon::activated, this, &EColorCore::iconActivated);
#endif
    versionChecker = new VersionChecker();
    connect(versionChecker, &VersionChecker::signalNewVersion, this, &EColorCore::showDialogUpdate);
    versionChecker->setSoftName(APPLICATION_NAME);
    versionChecker->setMajVersion(VER_MAJOR);
    versionChecker->setMinVersion(VER_MINOR);
    versionChecker->setPatVersion(VER_PATHES);
    versionChecker->setUrl(QUrl("http://www.evileg.ru/software/ecolor/ecolor.json"));
    versionChecker->startChecker();

    popUpColor = new PopUpColor();
    popUpMessage = new PopUpMessage();
    popUpMessage->setPopupText(trUtf8("Приложение EColor запущено"));
    popUpMessage->show();

    nativeEventFilter = new NativeEventFilter(this);
    qApp->installNativeEventFilter(nativeEventFilter);
    connect(nativeEventFilter, &NativeEventFilter::hotKeyShowPressed, popUpColor, &PopUpColor::onHotKeyShowPressed);
    connect(nativeEventFilter, &NativeEventFilter::hotKeyPixmapPressed, popUpColor, &PopUpColor::onHotKeyPixmapPressed);
    connect(popUpColor, &PopUpColor::hotKeysSettingsReloading, nativeEventFilter, &NativeEventFilter::onHotKeysSettingsReloading);
    emit popUpColor->reloadSettings();
}

EColorCore::~EColorCore()
{
    delete popUpColor;
    delete popUpMessage;
    delete versionChecker;
    delete trayIcon;
}

void EColorCore::showTriggered()
{
    (!popUpColor->isVisible())? popUpColor->slotShow() : popUpColor->slotHide();
}

void EColorCore::configTriggered()
{
    DialogSettings *dialogSettings = new DialogSettings();
    connect(dialogSettings, &DialogSettings::reloadKeySequence, popUpColor, &PopUpColor::reloadSettings);
    dialogSettings->show();
}

void EColorCore::aboutTriggered()
{
    About *about = new About();
    about->show();
}

void EColorCore::quitTriggered()
{ 
    delete versionChecker;
    delete trayIcon;
    popUpColor->saveSettings();
    exit(0);
}

#ifdef Q_OS_WIN32
void EColorCore::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason){
    case QSystemTrayIcon::Trigger:
        (!popUpColor->isVisible())? popUpColor->slotShow() : popUpColor->slotHide();
        break;
    default:
        break;
    }
}
#endif

void EColorCore::showDialogUpdate(QString newVersion)
{
    DialogUpdate *dialogUpdate = new DialogUpdate();
    dialogUpdate->setNewVersion(APPLICATION_NAME " v" + newVersion);
    dialogUpdate->show();
}
