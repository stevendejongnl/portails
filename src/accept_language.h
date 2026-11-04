// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once

#include <QString>
#include <QStringList>

namespace portails {

// Builds the Accept-Language header the way Chrome does: "en-US,en;q=0.9".
// Entries with a script part (en-Latn-US) and repeated entries are left out.
// An empty list gives "en-US,en;q=0.9".
QString buildAcceptLanguage(const QStringList &uiLanguages);

}  // namespace portails
