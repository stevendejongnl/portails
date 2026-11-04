// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once

#include <QUrl>
#include <QWebEnginePage>

namespace portails {

// The page of one tab. Popups that belong to the site or to a sign-in provider open in a
// small window of the same profile, so the sign-in can talk back to the tab. Other links
// open in the system browser.
class SitePage : public QWebEnginePage {
    Q_OBJECT

public:
    SitePage(QWebEngineProfile *profile, const QUrl &tabUrl, QObject *parent);

protected:
    QWebEnginePage *createWindow(WebWindowType type) override;

private:
    QUrl tabUrl;
};

class PopupPage : public QWebEnginePage {
    Q_OBJECT

public:
    PopupPage(QWebEngineProfile *profile, const QUrl &tabUrl);

protected:
    bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame) override;

private:
    void showInWindow();

    QUrl tabUrl;
    bool destinationChecked = false;
};

}  // namespace portails
