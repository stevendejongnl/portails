// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once

#include <QStackedWidget>
#include <QWidget>

class QHBoxLayout;
class QWebEngineView;

namespace portails {

class Backend;

// The sidebar page on the left and the web views of the tabs on the right.
// While a tab has no site yet, the sidebar page fills the whole window and shows the setup screen.
class MainWindow : public QWidget {
    Q_OBJECT

public:
    MainWindow(Backend *backend, QStackedWidget *siteStack);

signals:
    // The sidebar page finished loading. The window can be shown then, without a blank frame.
    void sidebarLoaded();

public slots:
    void setSiteShowing(bool isShowing);

private:
    QWebEngineView *createSidebarView(Backend *backend);

    QHBoxLayout *layout;
    QWebEngineView *sidebarView;
    QStackedWidget *siteStack;
};

}  // namespace portails
