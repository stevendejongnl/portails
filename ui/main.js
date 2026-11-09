// SPDX-License-Identifier: AGPL-3.0-or-later
import * as api from "./api.js";
import { clear } from "./dom.js";
import { installMenuDismissal } from "./menu.js";
import { chooseDraftPlatform, newDraft, renderSetup } from "./setup-screen.js";
import { renderSidebar } from "./sidebar.js";

const tabList = document.getElementById("tab-list");
const main = document.getElementById("main");

let state = { tabs: [], active_id: null, platforms: [] };
let draft = newDraft(null);
let renamingId = null;

const activeTab = () => state.tabs.find((tab) => tab.id === state.active_id);

function syncDraft() {
  const tab = activeTab();
  if (tab && !tab.url && draft.tabId !== tab.id) {
    draft = newDraft(tab.id);
  }
}

function render() {
  syncDraft();
  renderSidebar(tabList, state, sidebarActions, renamingId);
  const tab = activeTab();
  if (!tab || tab.url) {
    clear(main);
    return;
  }
  renderSetup(main, state, draft, setupHandlers);
}

async function apply(request) {
  try {
    state = await request;
  } catch (error) {
    draft.error = String(error);
  }
  render();
}

const sidebarActions = {
  select: (tab) => tab.id !== state.active_id && apply(api.selectTab(tab.id)),
  rename: (tab) => { renamingId = tab.id; render(); },
  finishRename: (tab, name) => {
    renamingId = null;
    const changed = name !== null && name.trim() !== "" && name.trim() !== tab.name;
    changed ? apply(api.renameTab(tab.id, name)) : render();
  },
  reload: (tab) => apply(api.reloadTab(tab.id)),
  back: (tab) => apply(api.goBack(tab.id)),
  forward: (tab) => apply(api.goForward(tab.id)),
  home: (tab) => apply(api.goHome(tab.id)),
  stop: (tab) => apply(api.stopTab(tab.id)),
  copyAddress: (tab) => apply(api.copyAddress(tab.id)),
  openDevTools: (tab) => apply(api.openDevTools(tab.id)),
  openLink: async (tab, url) => {
    try {
      state = await api.openLink(tab.id, url);
      render();
      return "";
    } catch (error) {
      return String(error);
    }
  },
  remove: (tab) => apply(api.removeTab(tab.id)),
};

const setupHandlers = {
  choosePlatform: (platformId) => { chooseDraftPlatform(state, draft, platformId); render(); },
  back: () => { draft.platformId = null; draft.error = ""; render(); },
  cancel: () => apply(api.removeTab(draft.tabId)),
  submit: () => apply(api.configureTab(draft.tabId, draft.platformId, draft.name, draft.customUrl || null)),
};

// A page navigated. Only the sidebar is drawn again, so a name that is being typed in the
// setup screen or in a rename field keeps its focus.
api.onStateChanged((next) => {
  state = next;
  if (renamingId === null) {
    renderSidebar(tabList, state, sidebarActions, renamingId);
  }
});

async function start() {
  installMenuDismissal();
  document.getElementById("add-tab").addEventListener("click", () => apply(api.createTab()));
  await apply(api.listState());
  if (state.tabs.length === 0) {
    await apply(api.createTab());
  }
}

start();
