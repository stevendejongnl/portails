// SPDX-License-Identifier: AGPL-3.0-or-later
#include "navigation_rules.h"

#include <QStringList>

namespace portails {

namespace {

const QStringList AUTH_HOST_SUFFIXES = {
    "google.com", "apple.com", "microsoft.com", "microsoftonline.com", "live.com", "openai.com", "x.ai",
};

}  // namespace

QString registrableSuffix(const QString &host)
{
    const QStringList labels = host.split('.');
    const int start = qMax(0, labels.size() - 2);
    return labels.mid(start).join('.');
}

bool staysInTab(const QUrl &target, const QUrl &tabUrl)
{
    if (target.host().isEmpty() || tabUrl.host().isEmpty()) {
        return false;
    }
    const QString targetSuffix = registrableSuffix(target.host());
    if (targetSuffix == registrableSuffix(tabUrl.host())) {
        return true;
    }
    return AUTH_HOST_SUFFIXES.contains(targetSuffix);
}

}  // namespace portails
