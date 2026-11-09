// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once

#include <QObject>
#include <QString>

namespace portails {

class AppController;

// The one object the sidebar page can call. It takes a command and a JSON payload and answers with JSON.
class Backend : public QObject {
    Q_OBJECT

public:
    Backend(AppController *controller, QObject *parent);

    Q_INVOKABLE QString call(const QString &command, const QString &payloadJson);

signals:
    // The new state as JSON, pushed to the sidebar when a page navigates.
    void stateChanged(const QString &stateJson);

private:
    AppController *controller;
};

}  // namespace portails
