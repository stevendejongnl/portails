// SPDX-License-Identifier: AGPL-3.0-or-later
import { clear, el } from "./dom.js";

const MENU_WIDTH = 230;
const MENU_MARGIN = 8;

const menuLayer = () => document.getElementById("menu-layer");

export function closeMenu() {
  clear(menuLayer());
}

function clampToViewport(value, size, limit) {
  return Math.max(MENU_MARGIN, Math.min(value, limit - size - MENU_MARGIN));
}

function menuItem(label, onClick, extraClass = "") {
  return el("button", { className: `menu-item ${extraClass}`.trim(), type: "button", role: "menuitem", onClick, text: label });
}

function showMenu(content, x, y) {
  const menu = el("div", { className: "menu", role: "menu" }, content);
  menu.style.left = `${clampToViewport(x, MENU_WIDTH, window.innerWidth)}px`;
  menu.style.top = `${clampToViewport(y, 130, window.innerHeight)}px`;
  clear(menuLayer());
  menuLayer().append(menu);
  menu.querySelector("input, button")?.focus();
}

function showRemoveConfirmation(tab, actions, x, y) {
  showMenu(
    [
      el("div", { className: "menu-prompt", text: "Remove and sign out?" }),
      menuItem("Remove", () => { closeMenu(); actions.remove(tab); }, "menu-item-danger"),
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

export function openTabMenu(tab, actions, x, y) {
  const items = [
    menuItem("Rename", () => { closeMenu(); actions.rename(tab); }),
    menuItem("Reload", () => { closeMenu(); actions.reload(tab); }),
    menuItem("Open link...", () => showLinkPrompt(tab, actions, x, y)),
    el("div", { className: "menu-separator", role: "separator" }),
    menuItem("Remove...", () => showRemoveConfirmation(tab, actions, x, y), "menu-item-danger"),
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
