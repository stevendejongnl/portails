// SPDX-License-Identifier: AGPL-3.0-or-later
#include "platforms.h"

#include <QJsonObject>

#include <algorithm>

namespace portails {

const QString CUSTOM_PLATFORM_ID = QStringLiteral("custom");

const std::array<Platform, 8> &platformCatalog()
{
    static const std::array<Platform, 8> catalog = {{
        {"claude", "Claude", "https://claude.ai/"},
        {"chatgpt", "ChatGPT", "https://chatgpt.com/"},
        {"gemini", "Gemini", "https://gemini.google.com/"},
        {"perplexity", "Perplexity", "https://www.perplexity.ai/"},
        {"mistral", "Mistral Vibe", "https://chat.mistral.ai/"},
        {"copilot", "Microsoft Copilot", "https://copilot.com/"},
        {"grok", "Grok", "https://grok.com/"},
        {"deepseek", "DeepSeek", "https://chat.deepseek.com/"},
    }};
    return catalog;
}

std::optional<Platform> findPlatform(const QString &platformId)
{
    const auto &catalog = platformCatalog();
    const auto match = std::find_if(catalog.begin(), catalog.end(), [&](const Platform &platform) {
        return platform.id == platformId;
    });
    if (match == catalog.end()) {
        return std::nullopt;
    }
    return *match;
}

QJsonArray platformsToJson()
{
    QJsonArray platforms;
    for (const Platform &platform : platformCatalog()) {
        platforms.append(QJsonObject{{"id", platform.id}, {"name", platform.name}, {"url", platform.url}});
    }
    return platforms;
}

}  // namespace portails
