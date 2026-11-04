// SPDX-License-Identifier: AGPL-3.0-or-later
import { findPlatform, letterBadge, logoBadge } from "./badge.js";
import { clear, el } from "./dom.js";

const CUSTOM_ID = "custom";

export function newDraft(tabId) {
  return { tabId, platformId: null, name: "", customUrl: "", error: "" };
}

function defaultName(state, platformId) {
  const base = platformId === CUSTOM_ID ? "Custom" : findPlatform(state, platformId).name;
  const taken = state.tabs.filter((tab) => tab.platform_id === platformId).length;
  return taken === 0 ? base : `${base} ${taken + 1}`;
}

function platformCard(platform, onChoose) {
  return el(
    "button",
    { className: "platform-card", type: "button", onClick: () => onChoose(platform.id) },
    [logoBadge(platform.id, "badge-large"), el("span", { text: platform.name })],
  );
}

function customCard(onChoose) {
  return el(
    "button",
    { className: "platform-card", type: "button", onClick: () => onChoose(CUSTOM_ID) },
    [letterBadge("+", "badge-large"), el("span", { text: "Custom URL" })],
  );
}

function platformStep(state, draft, handlers) {
  const hasOtherTabs = state.tabs.length > 1;
  const grid = el("div", { className: "platform-grid" }, [
    ...state.platforms.map((platform) => platformCard(platform, handlers.choosePlatform)),
    customCard(handlers.choosePlatform),
  ]);
  const cancel = hasOtherTabs ? el("button", { className: "text-button", type: "button", onClick: handlers.cancel, text: "Cancel" }) : null;
  return el("section", { className: "setup" }, [
    el("h2", { className: "setup-title", text: "Add an account" }),
    el("p", { className: "setup-subtitle", text: "Choose a service. Every tab keeps its own login, so you can add the same service more than once." }),
    grid,
    cancel,
  ]);
}

function textField(id, label, value, placeholder, onInput) {
  const input = el("input", { id, value, placeholder, autocomplete: "off", spellcheck: "false" });
  input.addEventListener("input", () => onInput(input.value));
  return el("div", { className: "field" }, [el("label", { for: id, text: label }), input]);
}

function nameStep(state, draft, handlers) {
  const isCustom = draft.platformId === CUSTOM_ID;
  const badge = isCustom ? letterBadge("+", "badge-large") : logoBadge(draft.platformId, "badge-large");
  const title = isCustom ? "Custom site" : findPlatform(state, draft.platformId).name;
  const fields = [textField("tab-name", "Name", draft.name, "Shown in the sidebar", (value) => { draft.name = value; })];
  if (isCustom) {
    fields.push(textField("tab-url", "Address", draft.customUrl, "https://", (value) => { draft.customUrl = value; }));
  }
  const error = draft.error ? el("p", { className: "field-error", role: "alert", text: draft.error }) : null;
  const form = el("form", { className: "panel" }, [
    el("div", { className: "panel-heading" }, [badge, el("h2", { text: title })]),
    ...fields,
    error,
    el("p", { className: "privacy-note", text: "You sign in on the real website. This app never sees your password." }),
    el("div", { className: "button-row" }, [
      el("button", { className: "button", type: "button", onClick: handlers.back, text: "Back" }),
      el("button", { className: "button button-primary", type: "submit", text: "Open and sign in" }),
    ]),
  ]);
  form.addEventListener("submit", (event) => { event.preventDefault(); handlers.submit(); });
  queueMicrotask(() => document.getElementById("tab-name")?.select());
  return el("section", { className: "setup" }, [form]);
}

export function chooseDraftPlatform(state, draft, platformId) {
  draft.platformId = platformId;
  draft.name = defaultName(state, platformId);
  draft.error = "";
}

export function renderSetup(container, state, draft, handlers) {
  clear(container);
  const step = draft.platformId ? nameStep : platformStep;
  container.append(step(state, draft, handlers));
}
