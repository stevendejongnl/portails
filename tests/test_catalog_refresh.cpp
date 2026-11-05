// SPDX-License-Identifier: AGPL-3.0-or-later
#include "catalog_refresh.h"

#include <QTest>

using namespace portails;

namespace {

TabStore storeWith(const QString &name, const QString &platformId, const QString &url)
{
    TabStore store;
    store.addUnconfigured("t1");
    store.configure("t1", name, platformId, url);
    return store;
}

}  // namespace

class TestCatalogRefresh : public QObject {
    Q_OBJECT

private slots:
    void movedAddressIsUpdated()
    {
        TabStore store = storeWith("Work Copilot", "copilot", "https://copilot.microsoft.com/");
        QVERIFY(refreshAddressesFromCatalog(store));
        QCOMPARE(store.tabs()[0].url, QString("https://copilot.com/"));
    }

    void tabNamesAreNeverChanged()
    {
        for (const QString &name : {QString("Le Chat"), QString("Le Chat 2"), QString("Le Chat Work"), QString("Mistral Vibe"), QString("")}) {
            TabStore store = storeWith(name, "copilot", "https://copilot.microsoft.com/");
            refreshAddressesFromCatalog(store);
            QCOMPARE(store.tabs()[0].name, name);
        }
    }

    void currentAddressChangesNothing()
    {
        TabStore store = storeWith("Le Chat", "mistral", "https://chat.mistral.ai/");
        QVERIFY(!refreshAddressesFromCatalog(store));
        QCOMPARE(store.tabs()[0].name, QString("Le Chat"));
    }

    void customTabsAreNeverChanged()
    {
        TabStore store = storeWith("Mine", "custom", "https://example.org/");
        QVERIFY(!refreshAddressesFromCatalog(store));
        QCOMPARE(store.tabs()[0].url, QString("https://example.org/"));
    }

    void tabWithoutASiteYetIsSkipped()
    {
        TabStore store;
        store.addUnconfigured("t1");
        QVERIFY(!refreshAddressesFromCatalog(store));
    }

    void secondRunChangesNothing()
    {
        TabStore store = storeWith("Work", "copilot", "https://copilot.microsoft.com/");
        QVERIFY(refreshAddressesFromCatalog(store));
        QVERIFY(!refreshAddressesFromCatalog(store));
    }
};

QTEST_APPLESS_MAIN(TestCatalogRefresh)
#include "test_catalog_refresh.moc"
