// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once

class QUrl;
class QWebEnginePage;
class QWebEngineProfile;

namespace portails {

// Adds the window.chrome members that Chromium gives to every page, in every frame of the profile.
void installChromeObject(QWebEngineProfile *profile);

// Turns on the PDF viewer of Chromium. Pages then see the same PDF plugin entries as in Chrome.
void enablePdfViewer(QWebEnginePage *page);

// Lets the scripts of a page use the clipboard, as Chrome does for a click on a copy button.
// Clipboard permission is only granted to pages that stay in the site of the tab.
void enableClipboard(QWebEnginePage *page, const QUrl &tabUrl);

}  // namespace portails
