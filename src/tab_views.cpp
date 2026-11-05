// SPDX-License-Identifier: AGPL-3.0-or-later
#include "tab_views.h"

#include "accept_language.h"
#include "browser_features.h"
#include "site_page.h"
#include "user_agent.h"

#include <QEventLoop>
#include <QLocale>
#include <QWebEngineProfile>
#include <QWebEngineProfileBuilder>
#include <QWebEngineView>

namespace portails {

namespace {

constexpr int SEAL_INTERVAL_MILLISECONDS = 20000;
constexpr int CACHE_LIMIT_BYTES = 64 * 1024 * 1024;
constexpr int STORAGE_FLUSH_WAIT_MILLISECONDS = 600;

QString userAgentOverride()
{
    return qEnvironmentVariable(USER_AGENT_OVERRIDE_VARIABLE.toLatin1().constData());
}

// Chromium writes cookies and site storage on its own threads. Give it a moment after a profile closes.
void waitForStorageFlush()
{
    QEventLoop loop;
    QTimer::singleShot(STORAGE_FLUSH_WAIT_MILLISECONDS, &loop, &QEventLoop::quit);
    // Without socket notifiers, a termination signal cannot start a second quit inside this wait.
    loop.exec(QEventLoop::ExcludeSocketNotifiers);
}

}  // namespace

TabViews::TabViews(ProfileVault *vault, QStackedWidget *stack, QObject *parent)
    : QObject(parent)
    , vault(vault)
    , stack(stack)
{
    sealTimer.setInterval(SEAL_INTERVAL_MILLISECONDS);
    connect(&sealTimer, &QTimer::timeout, this, &TabViews::sealAll);
    sealTimer.start();
}

QWebEngineProfile *TabViews::createProfile(const QString &tabId)
{
    vault->restore(tabId);
    // The builder sets the paths before the profile exists. A profile made with only a name would first
    // create empty folders in the default data location, next to the encrypted files.
    QWebEngineProfileBuilder builder;
    builder.setPersistentStoragePath(vault->storagePath(tabId))
        .setCachePath(vault->cachePath(tabId))
        .setHttpCacheMaximumSize(CACHE_LIMIT_BYTES)
        .setPersistentCookiesPolicy(QWebEngineProfile::ForcePersistentCookies);
    auto *profile = builder.createProfile(QStringLiteral("tab-") + tabId, this);
    installChromeObject(profile);
    profile->setHttpAcceptLanguage(buildAcceptLanguage(QLocale::system().uiLanguages()));
    profile->setHttpUserAgent(chooseUserAgent(profile->httpUserAgent(), userAgentOverride()));
    return profile;
}

TabViews::OpenTab TabViews::openTab(const Tab &tab)
{
    const auto existing = openTabs.constFind(tab.id);
    if (existing != openTabs.constEnd()) {
        return *existing;
    }
    const QUrl url(tab.url);
    OpenTab created;
    created.profile = createProfile(tab.id);
    created.view = new QWebEngineView;
    created.view->setPage(new SitePage(created.profile, url, created.view));
    enablePdfViewer(created.view->page());
    created.view->setUrl(url);
    stack->addWidget(created.view);
    openTabs.insert(tab.id, created);
    return created;
}

void TabViews::show(const Tab &tab)
{
    stack->setCurrentWidget(openTab(tab).view);
}

void TabViews::load(const Tab &tab, const QUrl &url)
{
    openTab(tab).view->setUrl(url);
}

void TabViews::reload(const QString &tabId)
{
    const auto existing = openTabs.constFind(tabId);
    if (existing != openTabs.constEnd()) {
        existing->view->reload();
    }
}

void TabViews::closeTab(const QString &tabId)
{
    const auto existing = openTabs.constFind(tabId);
    if (existing == openTabs.constEnd()) {
        return;
    }
    const OpenTab closing = *existing;
    openTabs.remove(tabId);
    stack->removeWidget(closing.view);
    delete closing.view->page();
    delete closing.view;
    delete closing.profile;
}

void TabViews::remove(const QString &tabId)
{
    closeTab(tabId);
    vault->discard(tabId);
}

void TabViews::sealAll()
{
    for (auto tab = openTabs.constBegin(); tab != openTabs.constEnd(); ++tab) {
        vault->seal(tab.key());
    }
}

void TabViews::shutdown()
{
    if (hasShutDown) {
        return;
    }
    hasShutDown = true;
    sealTimer.stop();
    const QList<QString> tabIds = openTabs.keys();
    for (const QString &tabId : tabIds) {
        closeTab(tabId);
    }
    waitForStorageFlush();
    for (const QString &tabId : tabIds) {
        vault->seal(tabId);
    }
    vault->clearWorkingDirectory();
}

}  // namespace portails
