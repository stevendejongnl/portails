// SPDX-License-Identifier: AGPL-3.0-or-later
#include "tab_store.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>
#include <QUuid>

namespace portails {

namespace {

QString unknownTabMessage(const QString &id)
{
    return QStringLiteral("unknown tab %1").arg(id);
}

QJsonObject tabToJson(const Tab &tab)
{
    return {{"id", tab.id}, {"name", tab.name}, {"platform_id", tab.platformId}, {"url", tab.url}};
}

Tab tabFromJson(const QJsonObject &object)
{
    return {object["id"].toString(), object["name"].toString(), object["platform_id"].toString(),
            object["url"].toString()};
}

}  // namespace

QString newTabId()
{
    return QUuid::createUuid().toString(QUuid::Id128);
}

TabStore TabStore::load(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    if (!document.isObject()) {
        return {};
    }
    return fromJson(document.object());
}

bool TabStore::save(const QString &path) const
{
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    file.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    file.write(QJsonDocument(toJson()).toJson(QJsonDocument::Indented));
    return file.commit();
}

QJsonObject TabStore::toJson() const
{
    QJsonArray tabs;
    for (const Tab &tab : tabList) {
        tabs.append(tabToJson(tab));
    }
    return {{"tabs", tabs}, {"active_id", activeTabId}};
}

TabStore TabStore::fromJson(const QJsonObject &object)
{
    TabStore store;
    for (const QJsonValue &value : object["tabs"].toArray()) {
        store.tabList.append(tabFromJson(value.toObject()));
    }
    store.activeTabId = object["active_id"].toString();
    return store;
}

int TabStore::indexOf(const QString &id) const
{
    for (int index = 0; index < tabList.size(); ++index) {
        if (tabList[index].id == id) {
            return index;
        }
    }
    return -1;
}

const Tab *TabStore::findTab(const QString &id) const
{
    const int index = indexOf(id);
    return index < 0 ? nullptr : &tabList[index];
}

const Tab *TabStore::activeTab() const
{
    return findTab(activeTabId);
}

void TabStore::addUnconfigured(const QString &id)
{
    tabList.append({id, QString(), QString(), QString()});
    activeTabId = id;
}

QString TabStore::configure(const QString &id, const QString &name, const QString &platformId, const QString &url)
{
    const int index = indexOf(id);
    if (index < 0) {
        return unknownTabMessage(id);
    }
    tabList[index].name = name;
    tabList[index].platformId = platformId;
    tabList[index].url = url;
    return {};
}

QString TabStore::rename(const QString &id, const QString &name)
{
    const int index = indexOf(id);
    if (index < 0) {
        return unknownTabMessage(id);
    }
    tabList[index].name = name;
    return {};
}

QString TabStore::select(const QString &id)
{
    if (indexOf(id) < 0) {
        return unknownTabMessage(id);
    }
    activeTabId = id;
    return {};
}

QString TabStore::remove(const QString &id)
{
    const int index = indexOf(id);
    if (index < 0) {
        return unknownTabMessage(id);
    }
    tabList.removeAt(index);
    if (activeTabId != id) {
        return {};
    }
    const int neighborIndex = qMin(index, tabList.size() - 1);
    activeTabId = neighborIndex < 0 ? QString() : tabList[neighborIndex].id;
    return {};
}

}  // namespace portails
