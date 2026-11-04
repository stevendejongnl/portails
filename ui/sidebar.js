// SPDX-License-Identifier: AGPL-3.0-or-later
import { tabBadge } from "./badge.js";
import { clear, el, svgDots } from "./dom.js";
import { openTabMenu } from "./menu.js";

const DRAFT_LABEL = "New Tab";

function tabLabel(tab) {
  if (!tab.name) {
    return el("span", { className: "tab-label tab-label-draft", text: DRAFT_LABEL });
  }
  return el("span", { className: "tab-label", text: tab.name });
}

function renameField(tab, actions) {
  let finished = false;
  const finish = (commit) => {
    if (finished) {
      return;
    }
    finished = true;
    actions.finishRename(tab, commit ? input.value : null);
  };
  const input = el("input", { className: "tab-rename", value: tab.name, "aria-label": "Account name" });
  input.addEventListener("keydown", (event) => {
    if (event.key === "Enter") finish(true);
    if (event.key === "Escape") finish(false);
  });
  input.addEventListener("blur", () => finish(true));
  input.addEventListener("click", (event) => event.stopPropagation());
  queueMicrotask(() => { input.focus(); input.select(); });
  return input;
}

function moreButton(tab, actions) {
  const button = el("button", { className: "tab-more", type: "button", "aria-label": `Options for ${tab.name || DRAFT_LABEL}` }, [svgDots()]);
  button.addEventListener("click", (event) => {
    event.stopPropagation();
    const box = button.getBoundingClientRect();
    openTabMenu(tab, actions, box.left, box.bottom + 4);
  });
  return button;
}

function tabItem(state, tab, actions, renamingId) {
  const isRenaming = renamingId === tab.id;
  const item = el(
    "div",
    {
      className: "tab-item",
      role: "button",
      tabindex: "0",
      "aria-current": String(tab.id === state.active_id),
    },
    [tabBadge(state, tab), isRenaming ? renameField(tab, actions) : tabLabel(tab), tab.url ? moreButton(tab, actions) : null],
  );
  item.addEventListener("click", () => actions.select(tab));
  item.addEventListener("keydown", (event) => {
    if (event.target === item && (event.key === "Enter" || event.key === " ")) {
      event.preventDefault();
      actions.select(tab);
    }
  });
  item.addEventListener("contextmenu", (event) => {
    event.preventDefault();
    if (tab.url) openTabMenu(tab, actions, event.clientX, event.clientY);
  });
  return item;
}

export function renderSidebar(container, state, actions, renamingId) {
  clear(container);
  container.append(...state.tabs.map((tab) => tabItem(state, tab, actions, renamingId)));
}
