// SPDX-License-Identifier: AGPL-3.0-or-later
#include "browser_features.h"

#include "navigation_rules.h"

#include <QFile>
#include <QWebEnginePage>
#include <QWebEnginePermission>
#include <QWebEngineProfile>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#include <QWebEngineSettings>

namespace portails {

namespace {

const char *const CHROME_OBJECT_RESOURCE = ":/assets/chrome_object.js";

QString readResource(const char *path)
{
    QFile file(path);
    return file.open(QIODevice::ReadOnly) ? QString::fromUtf8(file.readAll()) : QString();
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

}  // namespace portails
