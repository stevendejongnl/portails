// SPDX-License-Identifier: AGPL-3.0-or-later
#include "platforms.h"

#include <QFileInfo>
#include <QSet>
#include <QTest>

using namespace portails;

class TestPlatforms : public QObject {
    Q_OBJECT

private slots:
    void findsKnownPlatform()
    {
        const auto platform = findPlatform("claude");
        QVERIFY(platform.has_value());
        QCOMPARE(platform->url, QString("https://claude.ai/"));
    }

    void unknownPlatformIsEmpty() { QVERIFY(!findPlatform("nope").has_value()); }

    void idsAreUniqueAndNeverCustom()
    {
        QSet<QString> ids;
        for (const Platform &platform : platformCatalog()) {
            ids.insert(platform.id);
        }
        QCOMPARE(ids.size(), int(platformCatalog().size()));
        QVERIFY(!ids.contains(CUSTOM_PLATFORM_ID));
    }

    void everyUrlIsHttps()
    {
        for (const Platform &platform : platformCatalog()) {
            QVERIFY2(platform.url.startsWith("https://"), qPrintable(platform.id));
        }
    }

    void everyPlatformHasALogoFile()
    {
        for (const Platform &platform : platformCatalog()) {
            const QString logo = QString(PORTAILS_SOURCE_DIR) + "/ui/logos/" + platform.id + ".svg";
            QVERIFY2(QFileInfo::exists(logo), qPrintable(logo));
        }
    }

    void jsonListsEveryPlatform() { QCOMPARE(platformsToJson().size(), int(platformCatalog().size())); }
};

QTEST_APPLESS_MAIN(TestPlatforms)
#include "test_platforms.moc"
