// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once

#include "profile_vault.h"
#include "tab_store.h"

#include <QHash>
#include <QObject>
#include <QStackedWidget>
#include <QTimer>

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

    // Writes the encrypted profile of every open tab.
    void sealAll();

    // Closes every view and profile, then writes the encrypted profiles one last time.
    void shutdown();

private:
    struct OpenTab {
        QWebEngineProfile *profile = nullptr;
        QWebEngineView *view = nullptr;
    };

    OpenTab openTab(const Tab &tab);
    QWebEngineProfile *createProfile(const QString &tabId);
    void closeTab(const QString &tabId);

    ProfileVault *vault;
    QStackedWidget *stack;
    QHash<QString, OpenTab> openTabs;
    QTimer sealTimer;
    bool hasShutDown = false;
};

}  // namespace portails
