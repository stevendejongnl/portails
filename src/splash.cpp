// SPDX-License-Identifier: AGPL-3.0-or-later
#include "splash.h"

#include <QFontDatabase>
#include <QLinearGradient>
#include <QPainter>
#include <QPixmap>

namespace portails {

namespace {

constexpr int WIDTH = 520;
constexpr int HEIGHT = 320;
constexpr int LOGO_SIZE = 116;
constexpr int LOGO_TOP = 66;
constexpr int WORDMARK_BASELINE = 236;
constexpr int WORDMARK_PIXELS = 38;

const QColor BACKGROUND_TOP("#232327");
const QColor BACKGROUND_BOTTOM("#141416");
const QColor BORDER(255, 255, 255, 30);
const QColor TEXT("#f5f5f7");
const QColor ACCENT("#5aa2ff");

QString interFamily()
{
    const int fontId = QFontDatabase::addApplicationFont(":/ui/fonts/InterVariable.woff2");
    const QStringList families = fontId < 0 ? QStringList() : QFontDatabase::applicationFontFamilies(fontId);
    return families.isEmpty() ? QString() : families.first();
}

QFont fontOfSize(int pixels, QFont::Weight weight)
{
    static const QString family = interFamily();
    QFont font = family.isEmpty() ? QFont() : QFont(family);
    font.setPixelSize(pixels);
    font.setWeight(weight);
    return font;
}

void drawBackground(QPainter &painter)
{
    QLinearGradient gradient(0, 0, 0, HEIGHT);
    gradient.setColorAt(0, BACKGROUND_TOP);
    gradient.setColorAt(1, BACKGROUND_BOTTOM);
    painter.fillRect(QRect(0, 0, WIDTH, HEIGHT), gradient);
    painter.setPen(BORDER);
    painter.drawRect(QRect(0, 0, WIDTH - 1, HEIGHT - 1));
}

void drawLogo(QPainter &painter)
{
    const QPixmap logo(":/assets/icon.png");
    painter.drawPixmap(QRect((WIDTH - LOGO_SIZE) / 2, LOGO_TOP, LOGO_SIZE, LOGO_SIZE), logo);
}

// "port" and "ls" in white, "ai" in blue, centered as one word.
void drawWordmark(QPainter &painter)
{
    const QFont font = fontOfSize(WORDMARK_PIXELS, QFont::Bold);
    const QFontMetrics metrics(font);
    const QStringList parts = {"port", "ai", "ls"};
    int totalWidth = 0;
    for (const QString &part : parts) {
        totalWidth += metrics.horizontalAdvance(part);
    }
    painter.setFont(font);
    int x = (WIDTH - totalWidth) / 2;
    for (const QString &part : parts) {
        painter.setPen(part == "ai" ? ACCENT : TEXT);
        painter.drawText(x, WORDMARK_BASELINE, part);
        x += metrics.horizontalAdvance(part);
    }
}

QPixmap buildPixmap(qreal pixelRatio)
{
    QPixmap pixmap(QSize(WIDTH, HEIGHT) * pixelRatio);
    pixmap.setDevicePixelRatio(pixelRatio);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
    drawBackground(painter);
    drawLogo(painter);
    drawWordmark(painter);
    return pixmap;
}

}  // namespace

LoadingSplash::LoadingSplash(qreal pixelRatio)
    : QSplashScreen(buildPixmap(pixelRatio), Qt::SplashScreen | Qt::FramelessWindowHint)
{
}

}  // namespace portails
