// SPDX-License-Identifier: AGPL-3.0-or-later
#include "app_controller.h"

#include "platforms.h"
#include "tab_inputs.h"

#include <QClipboard>
#include <QGuiApplication>
#include <QHash>
#include <QJsonArray>

namespace portails {

AppController::AppController(TabStore store, const QString &storePath, TabViews *views, QObject *parent)
    : QObject(parent)
    , store(std::move(store))
    , storePath(storePath)
    , views(views)
{
    connect(views, &TabViews::navigationChanged, this, [this] { emit stateChanged(stateJson()); });
}

void AppController::start()
{
    applyLayout();
}

QJsonObject AppController::stateJson() const
{
    QJsonObject state = store.toJson();
    QJsonArray tabs = state["tabs"].toArray();
    for (QJsonValueRef tab : tabs) {
        QJsonObject object = tab.toObject();
        object["nav"] = views->navigationState(object["id"].toString());
        tab = object;
    }
    state["tabs"] = tabs;
    state["platforms"] = platformsToJson();
    return state;
}

void AppController::applyLayout()
{
    const Tab *active = store.activeTab();
    const bool siteIsShowing = active && active->isConfigured();
    if (siteIsShowing) {
        views->show(*active);
    }
    emit siteShowingChanged(siteIsShowing);
}

QJsonObject AppController::handle(const QString &command, const QJsonObject &payload)
{
    static const QHash<QString, Command> commands = {
        {"list_state", &AppController::listState},     {"create_tab", &AppController::createTab},
        {"configure_tab", &AppController::configureTab}, {"rename_tab", &AppController::renameTab},
        {"select_tab", &AppController::selectTab},     {"reload_tab", &AppController::reloadTab},
        {"remove_tab", &AppController::removeTab},     {"open_link", &AppController::openLink},
        {"go_back", &AppController::goBack},           {"go_forward", &AppController::goForward},
        {"go_home", &AppController::goHome},           {"stop_tab", &AppController::stopTab},
        {"copy_address", &AppController::copyAddress}, {"open_dev_tools", &AppController::openDevTools},
    };
    const auto match = commands.constFind(command);
    if (match == commands.constEnd()) {
        return {{"error", QStringLiteral("unknown command %1").arg(command)}};
    }
    const QString error = (this->*match.value())(payload);
    if (!error.isEmpty()) {
        return {{"error", error}};
    }
    store.save(storePath);
    applyLayout();
    return {{"state", stateJson()}};
}

QString AppController::listState(const QJsonObject &)
{
    return {};
}

QString AppController::createTab(const QJsonObject &)
{
    store.addUnconfigured(newTabId());
    return {};
}

QString AppController::configureTab(const QJsonObject &payload)
{
    const QString platformId = payload["platformId"].toString();
    const UrlResolution resolution = resolveUrl(platformId, payload["customUrl"].toString());
    if (!resolution.error.isEmpty()) {
        return resolution.error;
    }
    const QString name = resolveName(payload["name"].toString(), platformId);
    return store.configure(payload["id"].toString(), name, platformId, resolution.url);
}

QString AppController::renameTab(const QJsonObject &payload)
{
    return store.rename(payload["id"].toString(), payload["name"].toString().trimmed());
}

QString AppController::selectTab(const QJsonObject &payload)
{
    return store.select(payload["id"].toString());
}

QString AppController::reloadTab(const QJsonObject &payload)
{
    views->reload(payload["id"].toString());
    return {};
}

QString AppController::openLink(const QJsonObject &payload)
{
    const Tab *tab = store.findTab(payload["id"].toString());
    if (!tab || !tab->isConfigured()) {
        return QStringLiteral("this tab has no site yet");
    }
    const UrlResolution resolution = resolveLink(payload["url"].toString(), QUrl(tab->url));
    if (!resolution.error.isEmpty()) {
        return resolution.error;
    }
    views->load(*tab, QUrl(resolution.url));
    return store.select(tab->id);
}

QString AppController::goBack(const QJsonObject &payload)
{
    views->goBack(payload["id"].toString());
    return {};
}

QString AppController::goForward(const QJsonObject &payload)
{
    views->goForward(payload["id"].toString());
    return {};
}

QString AppController::goHome(const QJsonObject &payload)
{
    const Tab *tab = store.findTab(payload["id"].toString());
    if (!tab || !tab->isConfigured()) {
        return QStringLiteral("this tab has no site yet");
    }
    views->load(*tab, QUrl(tab->url));
    return {};
}

QString AppController::stopTab(const QJsonObject &payload)
{
    views->stop(payload["id"].toString());
    return {};
}

QString AppController::copyAddress(const QJsonObject &payload)
{
    const QString address = views->address(payload["id"].toString());
    if (address.isEmpty()) {
        return QStringLiteral("this tab has no page yet");
    }
    QGuiApplication::clipboard()->setText(address);
    return {};
}

QString AppController::openDevTools(const QJsonObject &payload)
{
    views->openDevTools(payload["id"].toString());
    return {};
}

QString AppController::removeTab(const QJsonObject &payload)
{
    const QString id = payload["id"].toString();
    const QString error = store.remove(id);
    if (error.isEmpty()) {
        views->remove(id);
    }
    return error;
}

}  // namespace portails
