// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once

#include <QString>
#include <QUrl>

namespace portails {

QString registrableSuffix(const QString &host);

// True when a link or popup target belongs to the tab's own site or to a known sign-in provider.
bool staysInTab(const QUrl &target, const QUrl &tabUrl);

}  // namespace portails
