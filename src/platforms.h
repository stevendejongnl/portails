// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once

#include <QJsonArray>
#include <QString>

#include <array>
#include <optional>

namespace portails {

struct Platform {
    QString id;
    QString name;
    QString url;
};

extern const QString CUSTOM_PLATFORM_ID;

const std::array<Platform, 8> &platformCatalog();
std::optional<Platform> findPlatform(const QString &platformId);
QJsonArray platformsToJson();

}  // namespace portails
