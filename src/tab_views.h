// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once

#include "profile_vault.h"
#include "tab_store.h"

#include <QHash>
#include <QJsonObject>
#include <QObject>
#include <QSet>
#include <QStackedWidget>
#include <QTimer>

#include <functional>

class QWebEngineProfile;
class QWebEngineView;

namespace portails {

// One web view and one isolated profile per tab. Views are created when a tab is first shown
// and stay alive in the background afterwards.
class TabViews : public QObject {
    Q_OBJECT

public:
    TabViews(ProfileVault *vault, QStackedWidget *stack, QObject *parent);

    QStackedWidget *widget() const { return stack; }

    void show(const Tab &tab);
    void reload(const QString &tabId);
    void load(const Tab &tab, const QUrl &url);
    void remove(const QString &tabId);

    void goBack(const QString &tabId);
    void goForward(const QString &tabId);
    void stop(const QString &tabId);
    void openDevTools(const QString &tabId);

    // The address of the page that the tab shows now, or an empty string when the tab is not open.
    QString address(const QString &tabId) const;

    // What the navigation buttons need: can_go_back, can_go_forward, loading and host.
    QJsonObject navigationState(const QString &tabId) const;

    // Writes the encrypted profile of every open tab.
    void sealAll();

    // Closes every view and profile, then writes the encrypted profiles one last time.
    void shutdown();

signals:
    // A page started or finished loading, or its address or history changed.
    void navigationChanged(const QString &tabId);

private:
    struct OpenTab {
        QWebEngineProfile *profile = nullptr;
        QWebEngineView *view = nullptr;
        std::function<void()> openDevTools;
    };

    OpenTab openTab(const Tab &tab);
    QWebEngineProfile *createProfile(const QString &tabId);
    void closeTab(const QString &tabId);
    void watchNavigation(const QString &tabId, QWebEngineView *view);
    void installNavigationShortcuts(const Tab &tab, QWebEngineView *view);

    ProfileVault *vault;
    QStackedWidget *stack;
    QHash<QString, OpenTab> openTabs;
    QSet<QString> loadingTabs;
    QTimer sealTimer;
    bool hasShutDown = false;
};

}  // namespace portails
