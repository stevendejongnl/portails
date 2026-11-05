// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once

#include "tab_store.h"

namespace portails {

// The address of a service can move. Tabs of a catalog service follow the catalog for the address.
// The name of a tab is never changed: it is the name the user typed or accepted in the setup screen.
// Returns true when the address of a tab changed.
bool refreshAddressesFromCatalog(TabStore &store);

}  // namespace portails
