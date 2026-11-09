// SPDX-License-Identifier: AGPL-3.0-or-later
import { clear, el } from "./dom.js";
import { icon } from "./icons.js";

const MENU_MARGIN = 8;

const menuLayer = () => document.getElementById("menu-layer");

export function closeMenu() {
  clear(menuLayer());
}

function clampToViewport(value, size, limit) {
  return Math.max(MENU_MARGIN, Math.min(value, limit - size - MENU_MARGIN));
}

// A row with an optional icon on the left and a hint on the right, such as a key.
function menuItem(label, onClick, { extraClass = "", iconName = null, hint = null } = {}) {
  const content = [
    el("span", { className: "menu-icon" }, iconName ? [icon(iconName)] : []),
    el("span", { className: "menu-label", text: label }),
    hint ? el("span", { className: "menu-hint", text: hint }) : null,
  ];
  return el("button", { className: `menu-item ${extraClass}`.trim(), type: "button", role: "menuitem", onClick }, content);
}

// Up and down move between the rows, also past the text field of a prompt.
function moveFocus(menu, step) {
  const rows = [...menu.querySelectorAll(".menu-item, .menu-input")];
  const current = rows.indexOf(document.activeElement);
  const next = current < 0 ? (step > 0 ? 0 : rows.length - 1) : (current + step + rows.length) % rows.length;
  rows[next]?.focus();
}

function handleMenuKeys(menu, event) {
  const keys = { ArrowDown: 1, ArrowUp: -1 };
  if (event.key in keys) {
    event.preventDefault();
    moveFocus(menu, keys[event.key]);
  } else if (event.key === "Home" || event.key === "End") {
    event.preventDefault();
    const rows = menu.querySelectorAll(".menu-item, .menu-input");
    rows[event.key === "Home" ? 0 : rows.length - 1]?.focus();
  }
}

// The menu is placed after it is in the page, so its real height keeps it inside the window.
function showMenu(content, x, y) {
  const menu = el("div", { className: "menu", role: "menu" }, content);
  menu.addEventListener("keydown", (event) => handleMenuKeys(menu, event));
  clear(menuLayer());
  menuLayer().append(menu);
  const box = menu.getBoundingClientRect();
  menu.style.left = `${clampToViewport(x, box.width, window.innerWidth)}px`;
  menu.style.top = `${clampToViewport(y, box.height, window.innerHeight)}px`;
  (menu.querySelector("input") || menu.querySelector(".menu-item"))?.focus();
}

function menuHeader(tab) {
  const host = tab.nav?.host;
  return el("div", { className: "menu-header" }, [
    el("div", { className: "menu-title", text: tab.name }),
    host ? el("div", { className: "menu-subtitle", text: host }) : null,
  ]);
}

function showRemoveConfirmation(tab, actions, x, y) {
  showMenu(
    [
      el("div", { className: "menu-prompt", text: "Remove and sign out?" }),
      menuItem("Remove", () => { closeMenu(); actions.remove(tab); }, { extraClass: "menu-item-danger", iconName: "trash" }),
      menuItem("Cancel", closeMenu),
    ],
    x,
    y,
  );
}

async function submitLink(tab, actions, input, errorLine) {
  const error = await actions.openLink(tab, input.value);
  if (!error) {
    closeMenu();
    return;
  }
  errorLine.textContent = error;
}

function showLinkPrompt(tab, actions, x, y) {
  const input = el("input", { className: "menu-input", placeholder: "https://", autocomplete: "off", spellcheck: "false", "aria-label": "Link from the email" });
  const errorLine = el("div", { className: "menu-error", role: "alert" });
  input.addEventListener("keydown", (event) => event.key === "Enter" && submitLink(tab, actions, input, errorLine));
  showMenu(
    [
      el("div", { className: "menu-prompt", text: "Paste the link from the email" }),
      input,
      errorLine,
      menuItem("Open in this tab", () => submitLink(tab, actions, input, errorLine)),
      menuItem("Cancel", closeMenu),
    ],
    x,
    y,
  );
}

const separator = () => el("div", { className: "menu-separator", role: "separator" });
const run = (action) => () => { closeMenu(); action(); };

export function openTabMenu(tab, actions, x, y) {
  const items = [
    menuHeader(tab),
    menuItem("Home", run(() => actions.home(tab)), { iconName: "home", hint: "Alt+Home" }),
    menuItem("Rename", run(() => actions.rename(tab)), { iconName: "rename" }),
    menuItem("Reload", run(() => actions.reload(tab)), { iconName: "reload", hint: "F5" }),
    separator(),
    menuItem("Open link...", () => showLinkPrompt(tab, actions, x, y), { iconName: "link" }),
    menuItem("Copy page address", run(() => actions.copyAddress(tab)), { iconName: "copy" }),
    menuItem("Developer tools", run(() => actions.openDevTools(tab)), { iconName: "tools", hint: "F12" }),
    separator(),
    menuItem("Remove...", () => showRemoveConfirmation(tab, actions, x, y), { extraClass: "menu-item-danger", iconName: "trash" }),
  ];
  showMenu(items, x, y);
}

export function installMenuDismissal() {
  document.addEventListener("keydown", (event) => event.key === "Escape" && closeMenu());
  document.addEventListener("pointerdown", (event) => {
    if (!event.target.closest(".menu")) {
      closeMenu();
    }
  });
  window.addEventListener("blur", closeMenu);
}
