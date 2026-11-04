// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once

#include <QString>

namespace portails {

extern const QString USER_AGENT_OVERRIDE_VARIABLE;

// The engine is Chromium, so its own user agent is honest. Only the "QtWebEngine/x" token is removed.
// An override of "engine" keeps the engine user agent as it is. Any other override is used as given.
QString chooseUserAgent(const QString &engineUserAgent, const QString &overrideValue);

}  // namespace portails
