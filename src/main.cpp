// SPDX-License-Identifier: AGPL-3.0-or-later
#include "app_controller.h"
#include "backend.h"
#include "crypto.h"
#include "keyring.h"
#include "main_window.h"
#include "profile_vault.h"
#include "signal_quit.h"
#include "splash.h"
#include "tab_store.h"
#include "tab_views.h"

#include <QApplication>
#include <QDir>
#include <QIcon>
#include <QLockFile>
#include <QPalette>
#include <QStandardPaths>
#include <QStyleHints>
#include <QStyleFactory>
#include <QTextStream>
#include <QTimer>

using namespace portails;

namespace {

constexpr int REVEAL_TIMEOUT_MILLISECONDS = 10000;

const QColor WINDOW_COLOR("#161618");
const QColor BASE_COLOR("#1d1d20");
const QColor TEXT_COLOR("#f5f5f7");
const QColor ACCENT_COLOR("#0a6cff");

// Qt WebEngine reports the color scheme of the app to websites as prefers-color-scheme.
// Sites that have a dark theme then use it. Pages without one are not repainted.
void enforceDarkColorScheme(QApplication &application)
{
    application.styleHints()->setColorScheme(Qt::ColorScheme::Dark);
    if (application.styleHints()->colorScheme() != Qt::ColorScheme::Dark) {
        QTextStream(stderr) << "portails: this Qt platform theme cannot set a dark color scheme, "
                               "so websites may stay light\n";
    }
}

void applyDarkTheme(QApplication &application)
{
    enforceDarkColorScheme(application);
    application.setStyle(QStyleFactory::create("Fusion"));
    QPalette palette;
    palette.setColor(QPalette::Window, WINDOW_COLOR);
    palette.setColor(QPalette::WindowText, TEXT_COLOR);
    palette.setColor(QPalette::Base, BASE_COLOR);
    palette.setColor(QPalette::Text, TEXT_COLOR);
    palette.setColor(QPalette::Button, BASE_COLOR);
    palette.setColor(QPalette::ButtonText, TEXT_COLOR);
    palette.setColor(QPalette::Highlight, ACCENT_COLOR);
    application.setPalette(palette);
}

// The default platform theme of Qt on X11 cannot hold a color scheme. The GTK 3 theme can.
// A theme that the user chose stays as it is. This must run before QApplication starts.
void chooseColorSchemePlatformTheme()
{
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORMTHEME")) {
        qputenv("QT_QPA_PLATFORMTHEME", "gtk3");
    }
}

QString privateDirectory(const QString &path)
{
    QDir().mkpath(path);
    QFile::setPermissions(path, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
    return path;
}

// The working folder must be RAM-backed. Without a runtime folder, saving logins is turned off.
QString workingDirectoryPath(bool &isRamBacked)
{
    const QString runtime = QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation);
    isRamBacked = !runtime.isEmpty();
    const QString base = isRamBacked ? runtime : QDir::tempPath();
    return privateDirectory(base + "/portails") + "/profiles";
}

struct ProfileKeys {
    QByteArray key;
    QByteArray legacyKey;
};

// The key needs the machine id and a secret from the desktop keyring. Without both, nothing is saved.
// Saved files are not touched then, so they open again as soon as the keyring works.
ProfileKeys profileKeys(bool isRamBacked)
{
    const QByteArray machineId = readMachineId();
    const QByteArray machineKey = isRamBacked ? deriveProfileKey(machineId) : QByteArray();
    if (machineKey.isEmpty()) {
        QTextStream(stderr) << "portails: logins will not be saved (no machine id or no runtime folder)\n";
        return {};
    }
    const KeyringSecret keyring = loadOrCreateKeyringSecret();
    if (!keyring.error.isEmpty()) {
        QTextStream(stderr) << "portails: logins will not be saved, the keyring cannot be used: " << keyring.error << "\n";
        return {};
    }
    return {deriveProfileKey(machineId, keyring.secret), machineKey};
}

}  // namespace

int main(int argc, char *argv[])
{
    chooseColorSchemePlatformTheme();
    QApplication application(argc, argv);
    QApplication::setApplicationName("portails");
    QApplication::setDesktopFileName("portails");
    QApplication::setWindowIcon(QIcon(":/assets/icon.png"));
    applyDarkTheme(application);
    quitOnTerminationSignals(&application);

    const QString dataDirectory = privateDirectory(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    QLockFile instanceLock(dataDirectory + "/portails.lock");
    if (!instanceLock.tryLock(0)) {
        QTextStream(stderr) << "portails is already running\n";
        return 1;
    }

    LoadingSplash splash(application.devicePixelRatio());
    splash.show();
    application.processEvents();

    bool isRamBacked = false;
    const QString workingDirectory = workingDirectoryPath(isRamBacked);
    const ProfileKeys keys = profileKeys(isRamBacked);
    ProfileVault vault(dataDirectory + "/profiles", workingDirectory, keys.key, keys.legacyKey);
    vault.clearWorkingDirectory();

    const QString storePath = dataDirectory + "/tabs.json";
    auto *siteStack = new QStackedWidget;
    auto *views = new TabViews(&vault, siteStack, &application);
    auto *controller = new AppController(TabStore::load(storePath), storePath, views, &application);
    auto *backend = new Backend(controller, &application);

    MainWindow window(backend, siteStack);
    QObject::connect(controller, &AppController::siteShowingChanged, &window, &MainWindow::setSiteShowing);
    QObject::connect(&application, &QApplication::aboutToQuit, views, &TabViews::shutdown);
    controller->start();

    // The window appears when the sidebar page has loaded, or after a time limit.
    const auto reveal = [&window, &splash]() {
        if (window.isVisible()) {
            return;
        }
        window.show();
        splash.finish(&window);
    };
    QObject::connect(&window, &MainWindow::sidebarLoaded, &window, reveal);
    QTimer::singleShot(REVEAL_TIMEOUT_MILLISECONDS, &window, reveal);
    return application.exec();
}
