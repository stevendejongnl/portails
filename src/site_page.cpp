// SPDX-License-Identifier: AGPL-3.0-or-later
#include "site_page.h"

#include "browser_features.h"
#include "navigation_rules.h"

#include <QDesktopServices>
#include <QTimer>
#include <QWebEngineView>

namespace portails {

namespace {

constexpr int POPUP_WIDTH = 520;
constexpr int POPUP_HEIGHT = 720;

bool isBlankUrl(const QUrl &url)
{
    return url.isEmpty() || url.toString() == QLatin1String("about:blank");
}

}  // namespace

SitePage::SitePage(QWebEngineProfile *profile, const QUrl &tabUrl, QObject *parent)
    : QWebEnginePage(profile, parent)
    , tabUrl(tabUrl)
{
    enableClipboard(this, tabUrl);
}

QWebEnginePage *SitePage::createWindow(WebWindowType)
{
    return new PopupPage(profile(), tabUrl);
}

PopupPage::PopupPage(QWebEngineProfile *profile, const QUrl &tabUrl)
    : QWebEnginePage(profile, nullptr)
    , tabUrl(tabUrl)
{
    enableClipboard(this, tabUrl);
}

bool PopupPage::acceptNavigationRequest(const QUrl &url, NavigationType, bool isMainFrame)
{
    if (!isMainFrame || destinationChecked || isBlankUrl(url)) {
        return true;
    }
    destinationChecked = true;
    if (!staysInTab(url, tabUrl)) {
        QDesktopServices::openUrl(url);
        QTimer::singleShot(0, this, &QObject::deleteLater);
        return false;
    }
    showInWindow();
    return true;
}

void PopupPage::showInWindow()
{
    auto *view = new QWebEngineView;
    view->setAttribute(Qt::WA_DeleteOnClose);
    view->setPage(this);
    view->resize(POPUP_WIDTH, POPUP_HEIGHT);
    connect(this, &QWebEnginePage::windowCloseRequested, view, &QWidget::close);
    connect(view, &QObject::destroyed, this, &QObject::deleteLater);
    view->show();
}

}  // namespace portails
