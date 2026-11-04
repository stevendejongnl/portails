// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once

#include <QString>
#include <QUrl>

namespace portails {

struct UrlResolution {
    QString url;
    QString error;
};

// A catalog platform gives its start address. A custom tab needs an https address.
UrlResolution resolveUrl(const QString &platformId, const QString &customUrl);

// A link pasted into a tab must be https and must belong to the site of the tab or to a known sign-in provider.
UrlResolution resolveLink(const QString &text, const QUrl &tabUrl);

// A blank name falls back to the name of the platform.
QString resolveName(const QString &requested, const QString &platformId);

}  // namespace portails
