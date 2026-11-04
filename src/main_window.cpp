// SPDX-License-Identifier: AGPL-3.0-or-later
#include "main_window.h"

#include "backend.h"

#include <QColor>
#include <QHBoxLayout>
#include <QIcon>
#include <QUrl>
#include <QWebChannel>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineView>

namespace portails {

namespace {

constexpr int SIDEBAR_WIDTH = 220;
const QColor WINDOW_COLOR("#161618");

}  // namespace

MainWindow::MainWindow(Backend *backend, QStackedWidget *siteStack)
    : layout(new QHBoxLayout(this))
    , sidebarView(createSidebarView(backend))
    , siteStack(siteStack)
{
    setWindowTitle("portails");
    setWindowIcon(QIcon(":/assets/icon.png"));
    resize(1280, 800);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(sidebarView, 1);
    layout->addWidget(siteStack, 1);
    siteStack->hide();
}

QWebEngineView *MainWindow::createSidebarView(Backend *backend)
{
    auto *view = new QWebEngineView(this);
    auto *profile = new QWebEngineProfile(view);
    auto *page = new QWebEnginePage(profile, view);
    auto *channel = new QWebChannel(page);
    channel->registerObject("backend", backend);
    page->setWebChannel(channel);
    page->setBackgroundColor(WINDOW_COLOR);
    view->setPage(page);
    connect(page, &QWebEnginePage::loadFinished, this, &MainWindow::sidebarLoaded);
    view->setContextMenuPolicy(Qt::NoContextMenu);
    view->setUrl(QUrl("qrc:/ui/index.html"));
    return view;
}

void MainWindow::setSiteShowing(bool isShowing)
{
    siteStack->setVisible(isShowing);
    if (isShowing) {
        sidebarView->setFixedWidth(SIDEBAR_WIDTH);
    } else {
        sidebarView->setMinimumWidth(0);
        sidebarView->setMaximumWidth(QWIDGETSIZE_MAX);
    }
    layout->setStretchFactor(sidebarView, isShowing ? 0 : 1);
}

}  // namespace portails
