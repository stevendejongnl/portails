// SPDX-License-Identifier: AGPL-3.0-or-later
#include "user_agent.h"

#include <QTest>

using namespace portails;

namespace {
const QString ENGINE_AGENT = "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) QtWebEngine/6.11.3 Chrome/142.0.0.0 Safari/537.36";
}

class TestUserAgent : public QObject {
    Q_OBJECT

private slots:
    void engineTokenIsRemovedByDefault()
    {
        QCOMPARE(chooseUserAgent(ENGINE_AGENT, {}),
                 QString("Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/142.0.0.0 Safari/537.36"));
        QCOMPARE(chooseUserAgent(ENGINE_AGENT, "  "), chooseUserAgent(ENGINE_AGENT, {}));
    }

    void engineKeywordKeepsTheEngineAgent() { QCOMPARE(chooseUserAgent(ENGINE_AGENT, "engine"), ENGINE_AGENT); }

    void customOverrideIsUsedAsGiven() { QCOMPARE(chooseUserAgent(ENGINE_AGENT, " my agent "), QString("my agent")); }
};

QTEST_APPLESS_MAIN(TestUserAgent)
#include "test_user_agent.moc"
