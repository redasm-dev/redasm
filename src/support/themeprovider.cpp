#include "themeprovider.h"
#include "settings.h"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPalette>
#include <QVariant>
#include <cmath>

#define RD_THEME_UI_SET_COLOR(theme, palette, key)                             \
    if(auto it = theme.find(#key); it != theme.end())                          \
        palette.setColor(QPalette::key, it.value().toString());

namespace themeprovider {

namespace {

QJsonDocument g_theme;

// clang-format off
const QHash<QString, RDThemeKind> THEMES = {
    {"foreground", RD_THEME_FOREGROUND}, {"background", RD_THEME_BACKGROUND},
    {"seek", RD_THEME_SEEK},
    {"auto_comment", RD_THEME_AUTOCOMMENT}, {"comment", RD_THEME_COMMENT},
    {"highlight_fg", RD_THEME_HIGHLIGHT_FG}, {"highlight_bg", RD_THEME_HIGHLIGHT_BG},
    {"selection_fg", RD_THEME_SELECTION_FG}, {"selection_bg", RD_THEME_SELECTION_BG},
    {"cursor_fg", RD_THEME_CURSOR_FG}, {"cursor_bg", RD_THEME_CURSOR_BG},
    {"segment", RD_THEME_SEGMENT},
    {"function", RD_THEME_FUNCTION},
    {"address", RD_THEME_ADDRESS},
    {"constant", RD_THEME_CONSTANT},
    {"reg", RD_THEME_REG},
    {"string", RD_THEME_STRING},
    {"label", RD_THEME_LABEL},
    {"data", RD_THEME_DATA},
    {"pointer", RD_THEME_POINTER},
    {"import", RD_THEME_IMPORT},
    {"type", RD_THEME_TYPE},
    {"nop", RD_THEME_NOP},
    {"ret", RD_THEME_RET},
    {"call", RD_THEME_CALL},
    {"jump", RD_THEME_JUMP},
    {"jump_c", RD_THEME_JUMP_COND},
    {"entry_fg", RD_THEME_ENTRY_FG}, {"entry_bg", RD_THEME_ENTRY_BG},
    {"graph_bg", RD_THEME_GRAPH_BG}, {"graph_edge", RD_THEME_GRAPH_EDGE}, 
    {"graph_edge_loop", RD_THEME_GRAPH_EDGE_LOOP}, {"graph_edge_loop_cond", RD_THEME_GRAPH_EDGE_LOOP_COND},
    {"success", RD_THEME_SUCCESS}, {"fail", RD_THEME_FAIL}, {"warning", RD_THEME_WARNING}};
// clang-format on

bool load_theme(const QString& theme) {
    if(!g_theme.isNull()) return true;

    QFile f(QString(":/res/themes/%1.json").arg(theme.toLower()));
    if(!f.open(QFile::ReadOnly)) return false;

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &err);

    if(err.error != QJsonParseError::NoError) return false;

    QPalette palette = qApp->palette();
    QJsonObject obj = doc.object();

    // Apply some missing builtins
    if(!obj.contains("selection_fg"))
        obj["selection_fg"] = palette.color(QPalette::HighlightedText).name();

    if(!obj.contains("selection_bg"))
        obj["selection_bg"] = palette.color(QPalette::Highlight).name();

    doc.setObject(obj);

    g_theme.swap(doc);
    return true;
}

QStringList read_themes(const QString& path) {
    QStringList themes = QDir(path).entryList({"*.json"});

    for(QString& theme : themes) {
        theme.remove(".json");
        theme[0] = theme[0].toUpper();
    }

    return themes;
}

void apply_listing_theme() {
    QJsonObject theme = g_theme.object();

    for(auto it = theme.begin(); it != theme.end(); it++) {
        auto thit = THEMES.find(it.key());
        if(thit == THEMES.end()) continue;
        rd_set_theme_color(thit.value(), qUtf8Printable(it.value().toString()));
    }
}

} // namespace

QStringList themes() { return themeprovider::read_themes(":/res/themes"); }

QString theme(const QString& name) {
    return QString(":/res/themes/%1.json").arg(name.toLower());
}

bool is_dark_theme() {
    QPalette p = qApp->palette();
    QColor c = p.window().color();

    double hsp = std::sqrt((0.299 * (c.red() * c.red())) +
                           (0.587 * (c.green() * c.green())) +
                           (0.114 * (c.blue() * c.blue())));

    return hsp <= 127.5;
}

QColor color(RDThemeKind kind) {
    const char* color = rd_get_theme_color(kind);
    return color ? QColor{color} : QColor{};
}

QIcon icon(const QString& name) {
    REDasmSettings settings;

    return QIcon(QString(":/res/%1/%2.png")
                     .arg(themeprovider::is_dark_theme() ? "dark" : "light")
                     .arg(name));

    return {};
}

void apply_theme() {
    REDasmSettings settings;
    if(!themeprovider::load_theme(settings.current_theme())) return;

    QJsonObject theme = g_theme.object();
    QPalette palette = qApp->palette();

    RD_THEME_UI_SET_COLOR(theme, palette, Shadow);
    RD_THEME_UI_SET_COLOR(theme, palette, Base);
    RD_THEME_UI_SET_COLOR(theme, palette, AlternateBase);
    RD_THEME_UI_SET_COLOR(theme, palette, Text);
    RD_THEME_UI_SET_COLOR(theme, palette, Window);
    RD_THEME_UI_SET_COLOR(theme, palette, WindowText);
    RD_THEME_UI_SET_COLOR(theme, palette, Button);
    RD_THEME_UI_SET_COLOR(theme, palette, ButtonText);
    RD_THEME_UI_SET_COLOR(theme, palette, Highlight);
    RD_THEME_UI_SET_COLOR(theme, palette, HighlightedText);
    RD_THEME_UI_SET_COLOR(theme, palette, ToolTipBase);
    RD_THEME_UI_SET_COLOR(theme, palette, ToolTipText);

    qApp->setPalette(palette);
    themeprovider::apply_listing_theme();
}

} // namespace themeprovider
