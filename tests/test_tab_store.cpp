// SPDX-License-Identifier: AGPL-3.0-or-later
#include "tab_store.h"

#include <QFile>
#include <QTemporaryDir>
#include <QTest>

using namespace portails;

namespace {

TabStore storeWithTabs(const QStringList &ids)
{
    TabStore store;
    for (const QString &id : ids) {
        store.addUnconfigured(id);
    }
    return store;
}

}  // namespace

class TestTabStore : public QObject {
    Q_OBJECT

private slots:
    void newTabIsUnconfiguredAndActive()
    {
        const TabStore store = storeWithTabs({"a"});
        QVERIFY(store.activeTab());
        QVERIFY(!store.activeTab()->isConfigured());
    }

    void configureMarksTabConfigured()
    {
        TabStore store = storeWithTabs({"a"});
        QVERIFY(store.configure("a", "Claude", "claude", "https://claude.ai/").isEmpty());
        QVERIFY(store.activeTab()->isConfigured());
    }

    void unknownTabIsAnError()
    {
        TabStore store = storeWithTabs({"a"});
        QVERIFY(!store.rename("missing", "x").isEmpty());
        QVERIFY(!store.select("missing").isEmpty());
        QVERIFY(!store.remove("missing").isEmpty());
        QVERIFY(!store.configure("missing", "x", "claude", "https://x/").isEmpty());
    }

    void removingActiveTabSelectsTheNextOne()
    {
        TabStore store = storeWithTabs({"a", "b", "c"});
        store.select("b");
        store.remove("b");
        QCOMPARE(store.activeId(), QString("c"));
    }

    void removingLastTabSelectsThePreviousOne()
    {
        TabStore store = storeWithTabs({"a", "b"});
        store.remove("b");
        QCOMPARE(store.activeId(), QString("a"));
    }

    void removingTheOnlyTabLeavesNothingActive()
    {
        TabStore store = storeWithTabs({"a"});
        store.remove("a");
        QVERIFY(store.activeId().isEmpty());
        QVERIFY(!store.activeTab());
    }

    void removingABackgroundTabKeepsTheActiveOne()
    {
        TabStore store = storeWithTabs({"a", "b"});
        store.remove("a");
        QCOMPARE(store.activeId(), QString("b"));
    }

    void saveAndLoadRoundTrip()
    {
        QTemporaryDir directory;
        const QString path = directory.filePath("tabs.json");
        TabStore store = storeWithTabs({"a"});
        store.rename("a", "Work");
        QVERIFY(store.save(path));
        const TabStore loaded = TabStore::load(path);
        QCOMPARE(loaded.tabs().size(), 1);
        QCOMPARE(loaded.tabs()[0].name, QString("Work"));
        QCOMPARE(loaded.activeId(), QString("a"));
    }

    void savedFileIsOnlyReadableByTheOwner()
    {
        QTemporaryDir directory;
        const QString path = directory.filePath("tabs.json");
        QVERIFY(storeWithTabs({"a"}).save(path));
        QCOMPARE(QFile::permissions(path) & (QFile::ReadGroup | QFile::ReadOther | QFile::WriteGroup | QFile::WriteOther),
                 QFileDevice::Permissions());
    }

    void missingOrBrokenFileGivesAnEmptyStore()
    {
        QTemporaryDir directory;
        QVERIFY(TabStore::load(directory.filePath("missing.json")).tabs().isEmpty());
        QFile broken(directory.filePath("broken.json"));
        QVERIFY(broken.open(QIODevice::WriteOnly));
        broken.write("not json");
        broken.close();
        QVERIFY(TabStore::load(directory.filePath("broken.json")).tabs().isEmpty());
    }

    void tabIdsDiffer() { QVERIFY(newTabId() != newTabId()); }
};

QTEST_APPLESS_MAIN(TestTabStore)
#include "test_tab_store.moc"
