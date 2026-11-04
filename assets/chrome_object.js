// SPDX-License-Identifier: AGPL-3.0-or-later
// Chromium gives every page a window.chrome object with these members.
// Qt WebEngine leaves the object empty, so a page cannot tell which browser it runs in.
(() => {
  const chromeObject = window.chrome || (window.chrome = {});
  const timing = () => performance.timing;
  const navigationEntry = () => performance.getEntriesByType("navigation")[0] || {};
  const paintTime = (name) => {
    const entry = performance.getEntriesByName(name)[0];
    return entry ? (timing().navigationStart + entry.startTime) / 1000 : 0;
  };

  const loadTimes = () => {
    const entry = navigationEntry();
    const start = timing().navigationStart / 1000;
    const protocol = entry.nextHopProtocol || "http/1.1";
    return {
      requestTime: start,
      startLoadTime: start,
      commitLoadTime: timing().responseStart / 1000,
      finishDocumentLoadTime: timing().domContentLoadedEventEnd / 1000,
      finishLoadTime: timing().loadEventEnd / 1000,
      firstPaintTime: paintTime("first-paint"),
      firstPaintAfterLoadTime: 0,
      navigationType: entry.type === "reload" ? "Reload" : "Other",
      wasFetchedViaSpdy: protocol === "h2",
      wasNpnNegotiated: protocol === "h2",
      npnNegotiatedProtocol: protocol === "h2" ? "h2" : "unknown",
      wasAlternateProtocolAvailable: false,
      connectionInfo: protocol,
    };
  };

  const csi = () => ({
    startE: timing().navigationStart,
    onloadT: timing().domContentLoadedEventEnd,
    pageT: performance.now(),
    tran: 15,
  });

  const app = {
    isInstalled: false,
    getDetails: () => null,
    getIsInstalled: () => false,
    installState: (callback) => callback && callback("not_installed"),
    runningState: () => "cannot_run",
    InstallState: { DISABLED: "disabled", INSTALLED: "installed", NOT_INSTALLED: "not_installed" },
    RunningState: { CANNOT_RUN: "cannot_run", READY_TO_RUN: "ready_to_run", RUNNING: "running" },
  };

  if (!chromeObject.loadTimes) chromeObject.loadTimes = loadTimes;
  if (!chromeObject.csi) chromeObject.csi = csi;
  if (!chromeObject.app) chromeObject.app = app;
})();
