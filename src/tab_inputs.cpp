// SPDX-License-Identifier: AGPL-3.0-or-later
#include "tab_inputs.h"

#include "navigation_rules.h"
#include "platforms.h"

#include <QUrl>

namespace portails {

namespace {

UrlResolution resolveCustomUrl(const QString &text)
{
    if (text.trimmed().isEmpty()) {
        return {{}, QStringLiteral("a custom url is required")};
    }
    const QUrl parsed(text.trimmed(), QUrl::StrictMode);
    if (!parsed.isValid() || parsed.host().isEmpty()) {
        return {{}, QStringLiteral("the address is not valid")};
    }
    if (parsed.scheme() != QLatin1String("https")) {
        return {{}, QStringLiteral("custom url must start with https://")};
    }
    return {parsed.toString(), {}};
}

}  // namespace

UrlResolution resolveUrl(const QString &platformId, const QString &customUrl)
{
    if (platformId == CUSTOM_PLATFORM_ID) {
        return resolveCustomUrl(customUrl);
    }
    const auto platform = findPlatform(platformId);
    if (!platform) {
        return {{}, QStringLiteral("unknown platform %1").arg(platformId)};
    }
    return {platform->url, {}};
}

UrlResolution resolveLink(const QString &text, const QUrl &tabUrl)
{
    const QUrl link(text.trimmed(), QUrl::TolerantMode);
    if (text.trimmed().isEmpty() || !link.isValid() || link.host().isEmpty()) {
        return {{}, QStringLiteral("the link is not valid")};
    }
    if (link.scheme() != QLatin1String("https")) {
        return {{}, QStringLiteral("the link must start with https://")};
    }
    if (!staysInTab(link, tabUrl)) {
        return {{}, QStringLiteral("the link does not belong to the site of this tab")};
    }
    return {link.toString(QUrl::FullyEncoded), {}};
}

QString resolveName(const QString &requested, const QString &platformId)
{
    const QString trimmed = requested.trimmed();
    if (!trimmed.isEmpty()) {
        return trimmed;
    }
    const auto platform = findPlatform(platformId);
    return platform ? platform->name : QStringLiteral("Custom");
}

}  // namespace portails
