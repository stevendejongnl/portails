// SPDX-License-Identifier: AGPL-3.0-or-later
#include "backend.h"

#include "app_controller.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace portails {

Backend::Backend(AppController *controller, QObject *parent)
    : QObject(parent)
    , controller(controller)
{
}

QString Backend::call(const QString &command, const QString &payloadJson)
{
    const QJsonObject payload = QJsonDocument::fromJson(payloadJson.toUtf8()).object();
    const QJsonObject reply = controller->handle(command, payload);
    return QString::fromUtf8(QJsonDocument(reply).toJson(QJsonDocument::Compact));
}

}  // namespace portails
