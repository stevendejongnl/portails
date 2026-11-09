// SPDX-License-Identifier: AGPL-3.0-or-later
// Small line icons on a 16 by 16 grid. Each entry lists the paths that draw it.
const SVG_NAMESPACE = "http://www.w3.org/2000/svg";

const ICON_PATHS = {
  back: ["M9.5 3.5 5 8l4.5 4.5"],
  forward: ["M6.5 3.5 11 8l-4.5 4.5"],
  reload: ["M13 8a5 5 0 1 1-1.6-3.7", "M13 2.5v3h-3"],
  stop: ["M4 4l8 8M12 4l-8 8"],
  home: ["M2.5 7.5 8 3l5.5 4.5", "M4 6.8V13h8V6.8", "M6.8 13V9.5h2.4V13"],
  rename: ["M3 13h2.2l7-7a1.5 1.5 0 0 0-2.2-2.2l-7 7z", "M9 5l2 2"],
  link: ["M6.8 9.2a2.5 2.5 0 0 0 3.5 0l2-2a2.5 2.5 0 0 0-3.5-3.5l-.7.7", "M9.2 6.8a2.5 2.5 0 0 0-3.5 0l-2 2a2.5 2.5 0 0 0 3.5 3.5l.7-.7"],
  copy: ["M5.5 5.5h6.5v7.5H5.5z", "M3.5 10.5v-8h6.5"],
  tools: ["M5.5 5 2.5 8l3 3", "M10.5 5l3 3-3 3", "M9 3.5 7 12.5"],
  trash: ["M3 4.5h10", "M6.5 4.5V3h3v1.5", "M4.5 4.5l.6 8.5h5.8l.6-8.5", "M7 7v3.5M9 7v3.5"],
};

export function icon(name, size = 14) {
  const svg = document.createElementNS(SVG_NAMESPACE, "svg");
  for (const [attribute, value] of Object.entries({ viewBox: "0 0 16 16", width: size, height: size, "aria-hidden": "true" })) {
    svg.setAttribute(attribute, value);
  }
  for (const data of ICON_PATHS[name]) {
    const path = document.createElementNS(SVG_NAMESPACE, "path");
    for (const [attribute, value] of Object.entries({ d: data, fill: "none", stroke: "currentColor", "stroke-width": "1.5", "stroke-linecap": "round", "stroke-linejoin": "round" })) {
      path.setAttribute(attribute, value);
    }
    svg.append(path);
  }
  return svg;
}
