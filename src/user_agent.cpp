// SPDX-License-Identifier: AGPL-3.0-or-later
#include "user_agent.h"

#include <QRegularExpression>

namespace portails {

const QString USER_AGENT_OVERRIDE_VARIABLE = QStringLiteral("PORTAILS_USER_AGENT");

namespace {

const QString ENGINE_KEYWORD = QStringLiteral("engine");

QString withoutEngineToken(const QString &userAgent)
{
    static const QRegularExpression engineToken(" QtWebEngine/[0-9.]+");
    QString cleaned = userAgent;
    return cleaned.remove(engineToken);
}

}  // namespace

QString chooseUserAgent(const QString &engineUserAgent, const QString &overrideValue)
{
    const QString requested = overrideValue.trimmed();
    if (requested.isEmpty()) {
        return withoutEngineToken(engineUserAgent);
    }
    if (requested == ENGINE_KEYWORD) {
        return engineUserAgent;
    }
    return requested;
}

}  // namespace portails
