// SPDX-License-Identifier: AGPL-3.0-or-later
#include "catalog_refresh.h"

#include "platforms.h"

#include <QList>

namespace portails {

bool refreshAddressesFromCatalog(TabStore &store)
{
    bool changed = false;
    const QList<Tab> tabs = store.tabs();
    for (const Tab &tab : tabs) {
        const auto platform = findPlatform(tab.platformId);
        if (!platform || !tab.isConfigured() || platform->url == tab.url) {
            continue;
        }
        store.configure(tab.id, tab.name, tab.platformId, platform->url);
        changed = true;
    }
    return changed;
}

}  // namespace portails
