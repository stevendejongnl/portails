// SPDX-License-Identifier: AGPL-3.0-or-later
#include "browser_features.h"

#include "navigation_rules.h"

#include <QFile>
#include <QKeySequence>
#include <QMenu>
#include <QPointer>
#include <QShortcut>
#include <QWebEnginePage>
#include <QWebEnginePermission>
#include <QWebEngineProfile>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#include <QWebEngineSettings>
#include <QWebEngineView>

#include <memory>

namespace portails {

namespace {

constexpr int DEV_TOOLS_WIDTH = 1000;
constexpr int DEV_TOOLS_HEIGHT = 700;

const char *const CHROME_OBJECT_RESOURCE = ":/assets/chrome_object.js";

QString readResource(const char *path)
{
    QFile file(path);
    return file.open(QIODevice::ReadOnly) ? QString::fromUtf8(file.readAll()) : QString();
}

// The window is made on first use. It is a window of its own, so it is deleted together with the tab.
void showDevTools(QWebEngineView *view, QPointer<QWebEngineView> &devToolsView)
{
    if (!devToolsView) {
        devToolsView = new QWebEngineView;
        devToolsView->setAttribute(Qt::WA_DeleteOnClose);
        devToolsView->setPage(new QWebEnginePage(view->page()->profile(), devToolsView));
        devToolsView->resize(DEV_TOOLS_WIDTH, DEV_TOOLS_HEIGHT);
        devToolsView->setWindowTitle(QStringLiteral("Developer tools"));
        view->page()->setDevToolsPage(devToolsView->page());
        QObject::connect(view, &QObject::destroyed, devToolsView, &QWidget::close);
    }
    devToolsView->show();
    devToolsView->raise();
    devToolsView->activateWindow();
}

}  // namespace

void installChromeObject(QWebEngineProfile *profile)
{
    const QString source = readResource(CHROME_OBJECT_RESOURCE);
    if (source.isEmpty()) {
        return;
    }
    QWebEngineScript script;
    script.setName("chrome-object");
    script.setSourceCode(source);
    script.setInjectionPoint(QWebEngineScript::DocumentCreation);
    script.setWorldId(QWebEngineScript::MainWorld);
    script.setRunsOnSubFrames(true);
    profile->scripts()->insert(script);
}

void enablePdfViewer(QWebEnginePage *page)
{
    page->settings()->setAttribute(QWebEngineSettings::PdfViewerEnabled, true);
    page->settings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
}

void enableClipboard(QWebEnginePage *page, const QUrl &tabUrl)
{
    page->settings()->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard, true);
    page->settings()->setAttribute(QWebEngineSettings::JavascriptCanPaste, true);
    QObject::connect(page, &QWebEnginePage::permissionRequested, page, [tabUrl](QWebEnginePermission permission) {
        if (permission.permissionType() == QWebEnginePermission::PermissionType::ClipboardReadWrite
            && staysInTab(permission.origin(), tabUrl)) {
            permission.grant();
        }
    });
}

std::function<void()> installDevTools(QWebEngineView *view)
{
    const auto devToolsView = std::make_shared<QPointer<QWebEngineView>>();
    const auto open = [view, devToolsView] { showDevTools(view, *devToolsView); };
    for (const QKeySequence &keys : {QKeySequence(Qt::Key_F12), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_I)}) {
        auto *shortcut = new QShortcut(keys, view);
        shortcut->setContext(Qt::WidgetWithChildrenShortcut);
        QObject::connect(shortcut, &QShortcut::activated, view, open);
    }
    view->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(view, &QWidget::customContextMenuRequested, view, [view, open](const QPoint &position) {
        QMenu *menu = view->createStandardContextMenu();
        menu->setAttribute(Qt::WA_DeleteOnClose);
        menu->addSeparator();
        menu->addAction(QStringLiteral("Inspect"), view, [view, open] {
            open();
            view->page()->triggerAction(QWebEnginePage::InspectElement);
        });
        menu->popup(view->mapToGlobal(position));
    });
    return open;
}

}  // namespace portails
