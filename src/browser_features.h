// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once

class QWebEnginePage;
class QWebEngineProfile;

namespace portails {

// Adds the window.chrome members that Chromium gives to every page, in every frame of the profile.
void installChromeObject(QWebEngineProfile *profile);

// Turns on the PDF viewer of Chromium. Pages then see the same PDF plugin entries as in Chrome.
void enablePdfViewer(QWebEnginePage *page);

}  // namespace portails
