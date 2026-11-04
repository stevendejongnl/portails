// SPDX-License-Identifier: AGPL-3.0-or-later
#include "accept_language.h"

#include <QRegularExpression>

namespace portails {

namespace {

constexpr int MAXIMUM_LANGUAGES = 3;
constexpr double QUALITY_START = 0.9;
constexpr double QUALITY_STEP = 0.1;
const QString FALLBACK = QStringLiteral("en-US,en;q=0.9");

bool hasScriptPart(const QString &language)
{
    static const QRegularExpression script("-[A-Za-z]{4}(-|$)");
    return script.match(language).hasMatch();
}

QStringList usableLanguages(const QStringList &uiLanguages)
{
    QStringList languages;
    for (const QString &language : uiLanguages) {
        if (!hasScriptPart(language) && !languages.contains(language)) {
            languages.append(language);
        }
    }
    return languages.mid(0, MAXIMUM_LANGUAGES);
}

QString withQuality(const QString &language, int position)
{
    if (position == 0) {
        return language;
    }
    const double quality = QUALITY_START - QUALITY_STEP * (position - 1);
    return QStringLiteral("%1;q=%2").arg(language).arg(quality, 0, 'f', 1);
}

}  // namespace

QString buildAcceptLanguage(const QStringList &uiLanguages)
{
    const QStringList languages = usableLanguages(uiLanguages);
    if (languages.isEmpty()) {
        return FALLBACK;
    }
    QStringList entries;
    for (int position = 0; position < languages.size(); ++position) {
        entries.append(withQuality(languages[position], position));
    }
    return entries.join(',');
}

}  // namespace portails
