// SPDX-License-Identifier: AGPL-3.0-or-later
export function el(tag, properties = {}, children = []) {
  const element = document.createElement(tag);
  for (const [name, value] of Object.entries(properties)) {
    applyProperty(element, name, value);
  }
  element.append(...children.filter((child) => child !== null && child !== undefined && child !== false));
  return element;
}

function applyProperty(element, name, value) {
  if (value === undefined || value === null || value === false) {
    return;
  }
  if (name.startsWith("on")) {
    element.addEventListener(name.slice(2).toLowerCase(), value);
    return;
  }
  if (name === "className") {
    element.className = value;
    return;
  }
  if (name === "text") {
    element.textContent = value;
    return;
  }
  element.setAttribute(name, value === true ? "" : value);
}

export function clear(element) {
  element.replaceChildren();
}

const SVG_NAMESPACE = "http://www.w3.org/2000/svg";
const DOT_POSITIONS = [3, 8, 13];

function svgElement(tag, attributes) {
  const element = document.createElementNS(SVG_NAMESPACE, tag);
  for (const [name, value] of Object.entries(attributes)) {
    element.setAttribute(name, value);
  }
  return element;
}

export function svgDots() {
  const svg = svgElement("svg", { viewBox: "0 0 16 16", width: "14", height: "14", "aria-hidden": "true" });
  const dots = DOT_POSITIONS.map((cx) => svgElement("circle", { cx, cy: "8", r: "1.4", fill: "currentColor" }));
  svg.append(...dots);
  return svg;
}
