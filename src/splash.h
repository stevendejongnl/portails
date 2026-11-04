// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once

#include <QSplashScreen>

namespace portails {

// Shows the logo and the wordmark while the app starts. It is drawn in code, so it needs no extra file.
class LoadingSplash : public QSplashScreen {
public:
    explicit LoadingSplash(qreal pixelRatio);
};

}  // namespace portails
