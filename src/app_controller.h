// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once

#include "tab_store.h"
#include "tab_views.h"

#include <QJsonObject>
#include <QObject>
#include <QString>

namespace portails {

// Runs the commands of the sidebar. Every command changes the tab list, saves it and shows the
// right view. The answer is the new state, or an error message.
class AppController : public QObject {
    Q_OBJECT

public:
    AppController(TabStore store, const QString &storePath, TabViews *views, QObject *parent);

    QJsonObject handle(const QString &command, const QJsonObject &payload);

    // Shows the view of the active tab. Call once after the window exists.
    void start();

signals:
    void siteShowingChanged(bool isShowing);

    // The state changed without a command of the sidebar, for example a page that finished loading.
    void stateChanged(const QJsonObject &state);

private:
    using Command = QString (AppController::*)(const QJsonObject &payload);

    QString listState(const QJsonObject &payload);
    QString createTab(const QJsonObject &payload);
    QString configureTab(const QJsonObject &payload);
    QString renameTab(const QJsonObject &payload);
    QString selectTab(const QJsonObject &payload);
    QString reloadTab(const QJsonObject &payload);
    QString openLink(const QJsonObject &payload);
    QString removeTab(const QJsonObject &payload);
    QString goBack(const QJsonObject &payload);
    QString goForward(const QJsonObject &payload);
    QString goHome(const QJsonObject &payload);
    QString stopTab(const QJsonObject &payload);
    QString copyAddress(const QJsonObject &payload);
    QString openDevTools(const QJsonObject &payload);

    QJsonObject stateJson() const;
    void applyLayout();

    TabStore store;
    QString storePath;
    TabViews *views;
};

}  // namespace portails
