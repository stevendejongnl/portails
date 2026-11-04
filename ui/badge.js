// SPDX-License-Identifier: AGPL-3.0-or-later
import { el } from "./dom.js";

export function findPlatform(state, platformId) {
  return state.platforms.find((platform) => platform.id === platformId);
}

export function logoBadge(platformId, extraClass = "") {
  const image = el("img", { src: `logos/${platformId}.svg`, alt: "", draggable: "false" });
  return el("span", { className: `badge ${extraClass}`.trim(), "aria-hidden": "true" }, [image]);
}

export function letterBadge(letter, extraClass = "") {
  return el("span", { className: `badge badge-letter ${extraClass}`.trim(), "aria-hidden": "true", text: letter });
}

export function tabBadge(state, tab) {
  if (!tab.platform_id) {
    return letterBadge("", "badge-draft");
  }
  if (findPlatform(state, tab.platform_id)) {
    return logoBadge(tab.platform_id);
  }
  return letterBadge((tab.name || "?").trim().charAt(0).toUpperCase());
}
