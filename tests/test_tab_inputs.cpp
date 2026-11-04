// SPDX-License-Identifier: AGPL-3.0-or-later
#include "tab_inputs.h"

#include <QTest>

using namespace portails;

class TestTabInputs : public QObject {
    Q_OBJECT

private slots:
    void catalogPlatformResolvesToItsUrl()
    {
        const UrlResolution resolution = resolveUrl("claude", {});
        QCOMPARE(resolution.url, QString("https://claude.ai/"));
        QVERIFY(resolution.error.isEmpty());
    }

    void unknownPlatformIsRejected() { QVERIFY(!resolveUrl("nope", {}).error.isEmpty()); }

    void customPlatformNeedsAnHttpsUrl()
    {
        QVERIFY(!resolveUrl("custom", {}).error.isEmpty());
        QVERIFY(!resolveUrl("custom", "http://example.org").error.isEmpty());
        QVERIFY(!resolveUrl("custom", "not a url").error.isEmpty());
        QVERIFY(!resolveUrl("custom", "https://").error.isEmpty());
        QCOMPARE(resolveUrl("custom", " https://example.org/chat ").url, QString("https://example.org/chat"));
    }

    void magicLinkOfTheSameSiteIsAccepted()
    {
        const QUrl tab("https://claude.ai/");
        const UrlResolution resolution = resolveLink(" https://claude.ai/magic-link#abc123:ZW1haWxAZXhhbXBsZS5jb20= ", tab);
        QVERIFY(resolution.error.isEmpty());
        QVERIFY(resolution.url.startsWith("https://claude.ai/magic-link#abc123:"));
    }

    void linkOfAnotherSiteOrSchemeIsRejected()
    {
        const QUrl tab("https://claude.ai/");
        QVERIFY(!resolveLink("https://example.org/", tab).error.isEmpty());
        QVERIFY(!resolveLink("http://claude.ai/magic-link", tab).error.isEmpty());
        QVERIFY(!resolveLink("file:///etc/passwd", tab).error.isEmpty());
        QVERIFY(!resolveLink("javascript:alert(1)", tab).error.isEmpty());
        QVERIFY(!resolveLink("", tab).error.isEmpty());
        QVERIFY(!resolveLink("not a link", tab).error.isEmpty());
    }

    void blankNameFallsBackToThePlatformName()
    {
        QCOMPARE(resolveName("  ", "gemini"), QString("Gemini"));
        QCOMPARE(resolveName(" Work ", "gemini"), QString("Work"));
        QCOMPARE(resolveName("", "custom"), QString("Custom"));
    }
};

QTEST_APPLESS_MAIN(TestTabInputs)
#include "test_tab_inputs.moc"
