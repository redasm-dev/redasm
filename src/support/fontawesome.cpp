#include "fontawesome.h"
#include <QApplication>
#include <QFontDatabase>
#include <QIcon>
#include <QIconEngine>
#include <QPainter>
#include <QPalette>

namespace {

class FAIconEngine: public QIconEngine {
public:
    void paint(QPainter* painter, const QRect& rect, QIcon::Mode mode,
               QIcon::State) override {
        QFont f = this->font;
        int drawsize = qRound(rect.height() * 0.8);
        f.setPixelSize(drawsize);

        QColor pc = this->color;
        const QPalette& p = qApp->palette();

        if(mode == QIcon::Disabled) {
            pc = p.color(QPalette::Disabled, QPalette::ButtonText);
        }
        else if(mode == QIcon::Selected ||
                (mode == QIcon::Active &&
                 QApplication::activePopupWidget() != nullptr)) {
            pc = p.color(QPalette::Active, QPalette::HighlightedText);
        }

        painter->save();
        painter->setPen(QPen{pc});
        painter->setFont(f);
        painter->drawText(rect, Qt::AlignCenter, this->letter);
        painter->restore();
    }

    QPixmap pixmap(const QSize& size, QIcon::Mode mode,
                   QIcon::State state) override {
        QPixmap pixmap(size);
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);
        this->paint(&painter, QRect(QPoint(0, 0), size), mode, state);

        return pixmap;
    }

    [[nodiscard]] QIconEngine* clone() const override {
        return new FAIconEngine(*this);
    }

public:
    QFont font;
    QString letter;
    QColor color;
};

QStringList g_fafamilies, g_fabfamilies;

void init_fontawesome() {
    if(g_fabfamilies.empty()) {
        int id = QFontDatabase::addApplicationFont(
            ":/res/fonts/FontAwesomeBrands6.otf");
        if(id != -1) g_fabfamilies = QFontDatabase::applicationFontFamilies(id);
    }

    if(g_fafamilies.empty()) {
        int id = QFontDatabase::addApplicationFont(
            ":/res/fonts/FontAwesomeSolid6.otf");
        if(id != -1) g_fafamilies = QFontDatabase::applicationFontFamilies(id);
    }
}

} // namespace

namespace font_awesome {

QFont fa_font() {
    init_fontawesome();

    QFont f(g_fafamilies.front());
    f.setStyleStrategy(QFont::NoFontMerging);
    return f;
}
QFont fa_brands_font() {
    init_fontawesome();

    QFont f(g_fabfamilies.front());
    f.setStyleStrategy(QFont::NoFontMerging);
    return f;
}

QIcon icon(const QChar& code, const QColor& color) {
    auto* engine = new FAIconEngine();
    engine->font = font_awesome::fa_font();
    engine->letter = code;
    engine->color = color;
    return QIcon{engine};
}

QIcon brand(const QChar& code, const QColor& color) {
    auto* engine = new FAIconEngine();
    engine->font = font_awesome::fa_brands_font();
    engine->letter = code;
    engine->color = color;
    return QIcon{engine};
}

QIcon icon(const QChar& code) {
    return font_awesome::icon(code, qApp->palette().color(QPalette::Text));
}

QIcon brand(const QChar& code) {
    return font_awesome::brand(code, qApp->palette().color(QPalette::Text));
}

} // namespace font_awesome
