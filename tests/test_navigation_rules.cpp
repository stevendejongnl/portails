// SPDX-License-Identifier: AGPL-3.0-or-later
#include "navigation_rules.h"

#include <QTest>

using namespace portails;

class TestNavigationRules : public QObject {
    Q_OBJECT

private slots:
    void sameSiteSubdomainStaysInTab()
    {
        QVERIFY(staysInTab(QUrl("https://auth.openai.com/x"), QUrl("https://chatgpt.com/")));
        QVERIFY(staysInTab(QUrl("https://www.claude.ai/x"), QUrl("https://claude.ai/")));
    }

    void googleLoginStaysInTab()
    {
        QVERIFY(staysInTab(QUrl("https://accounts.google.com/o/oauth2"), QUrl("https://claude.ai/")));
    }

    void unrelatedSiteLeavesTheApp() { QVERIFY(!staysInTab(QUrl("https://example.org/"), QUrl("https://claude.ai/"))); }

    void lookAlikeSuffixIsNotAccepted()
    {
        QVERIFY(!staysInTab(QUrl("https://notgoogle.com/"), QUrl("https://claude.ai/")));
    }

    void addressWithoutHostIsNotAccepted() { QVERIFY(!staysInTab(QUrl("mailto:x@y.z"), QUrl("https://claude.ai/"))); }
};

QTEST_APPLESS_MAIN(TestNavigationRules)
#include "test_navigation_rules.moc"
