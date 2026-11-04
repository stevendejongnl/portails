// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once

#include <QJsonObject>
#include <QList>
#include <QString>

namespace portails {

struct Tab {
    QString id;
    QString name;
    QString platformId;
    QString url;

    bool isConfigured() const { return !url.isEmpty(); }
};

QString newTabId();

// Every method that can fail returns an error message. An empty string means success.
class TabStore {
public:
    static TabStore load(const QString &path);
    bool save(const QString &path) const;

    const QList<Tab> &tabs() const { return tabList; }
    const QString &activeId() const { return activeTabId; }
    const Tab *findTab(const QString &id) const;
    const Tab *activeTab() const;

    void addUnconfigured(const QString &id);
    QString configure(const QString &id, const QString &name, const QString &platformId, const QString &url);
    QString rename(const QString &id, const QString &name);
    QString select(const QString &id);
    QString remove(const QString &id);

    QJsonObject toJson() const;
    static TabStore fromJson(const QJsonObject &object);

private:
    int indexOf(const QString &id) const;

    QList<Tab> tabList;
    QString activeTabId;
};

}  // namespace portails
