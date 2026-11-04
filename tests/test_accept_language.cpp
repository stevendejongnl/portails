// SPDX-License-Identifier: AGPL-3.0-or-later
#include "accept_language.h"

#include <QTest>

using namespace portails;

class TestAcceptLanguage : public QObject {
    Q_OBJECT

private slots:
    void englishLocaleGivesTheChromeFormat()
    {
        QCOMPARE(buildAcceptLanguage({"en-Latn-US", "en-US", "en-Latn", "en"}), QString("en-US,en;q=0.9"));
    }

    void dutchLocaleKeepsRegionAndLanguage()
    {
        QCOMPARE(buildAcceptLanguage({"nl-Latn-NL", "nl-NL", "nl-Latn", "nl"}), QString("nl-NL,nl;q=0.9"));
    }

    void qualityFallsWithEachEntry()
    {
        QCOMPARE(buildAcceptLanguage({"nl-NL", "nl", "en-US", "en"}), QString("nl-NL,nl;q=0.9,en-US;q=0.8"));
    }

    void repeatedEntriesAreLeftOut() { QCOMPARE(buildAcceptLanguage({"en", "en", "en"}), QString("en")); }

    void emptyListGivesTheFallback() { QCOMPARE(buildAcceptLanguage({}), QString("en-US,en;q=0.9")); }

    void onlyScriptEntriesGiveTheFallback() { QCOMPARE(buildAcceptLanguage({"en-Latn"}), QString("en-US,en;q=0.9")); }
};

QTEST_APPLESS_MAIN(TestAcceptLanguage)
#include "test_accept_language.moc"
