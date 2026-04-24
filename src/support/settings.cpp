#include "settings.h"
#include <QApplication>
#include <QDir>
#include <QFontDatabase>
#include <QStandardPaths>

namespace {

const QString CURRENT_WINDOW_GEOMETRY = "window_geometry";
const QString CURRENT_WINDOW_STATE = "window_state";
const QString RECENT_FILES = "recent_files";
const QString SELECTED_THEME = "selected_theme";
const QString SELECTED_FONT = "selected_font";
const QString SELECTED_FONT_SIZE = "selected_font_size";

} // namespace

QByteArray REDasmSettings::m_defaultstate;

REDasmSettings::REDasmSettings(QObject* parent)
    : QSettings{
          QDir(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation))
              .absoluteFilePath("redasm.conf"),
          QSettings::IniFormat, parent} {}

bool REDasmSettings::restore_state(QMainWindow* mainwindow) {
    m_defaultstate = mainwindow->saveState(); // Keep default state

    if(this->contains(CURRENT_WINDOW_GEOMETRY))
        mainwindow->restoreGeometry(
            this->value(CURRENT_WINDOW_GEOMETRY).toByteArray());

    if(this->contains(CURRENT_WINDOW_STATE)) {
        mainwindow->restoreState(
            this->value(CURRENT_WINDOW_STATE).toByteArray());
        return true;
    }

    return false;
}

void REDasmSettings::default_state(QMainWindow* mainwindow) {
    mainwindow->restoreState(m_defaultstate);
}

void REDasmSettings::save_state(const QMainWindow* mainwindow) {
    this->setValue(CURRENT_WINDOW_GEOMETRY, mainwindow->saveGeometry());
    this->setValue(CURRENT_WINDOW_STATE, mainwindow->saveState());
}

void REDasmSettings::clear_recent_files() {
    this->setValue(RECENT_FILES, QStringList());
}
QStringList REDasmSettings::recent_files() const {
    return this->value(RECENT_FILES).toStringList();
}

void REDasmSettings::update_recent_files(const QString& s) {
    QStringList recents = this->recent_files();
    recents.removeAll(s); // Remove duplicates
    recents.prepend(s);

    while(recents.length() > MAX_RECENT_FILES)
        recents.removeLast();

    this->setValue(RECENT_FILES, recents);
}

QString REDasmSettings::current_theme() const {
    return this->value(SELECTED_THEME, "light").toString();
}
QFont REDasmSettings::current_font() const {
    return this
        ->value(SELECTED_FONT,
                QFontDatabase::systemFont(QFontDatabase::FixedFont))
        .value<QFont>();
}

int REDasmSettings::current_font_size() const {
    int size = qApp->font().pixelSize();
    if(size == -1) size = QFontMetrics{qApp->font()}.height();
    return this->value(SELECTED_FONT_SIZE, size).toInt();
}

void REDasmSettings::change_theme(const QString& theme) {
    this->setValue(SELECTED_THEME, theme.toLower());
}
void REDasmSettings::change_font(const QFont& font) {
    this->setValue(SELECTED_FONT, font);
}
void REDasmSettings::change_font_size(int size) {
    this->setValue(SELECTED_FONT_SIZE, size);
}

QFont REDasmSettings::font() {
    REDasmSettings settings;
    QFont f = settings.current_font();

    if(f.fixedPitch() || f.family().contains("mono", Qt::CaseInsensitive) ||
       f.styleHint() == QFont::Monospace ||
       f.styleHint() == QFont::TypeWriter) {
        f.setFamily("Monospace");
        f.setFixedPitch(true);
        f.setStyleHint(QFont::TypeWriter);
    }

    f.setPixelSize(settings.current_font_size());
    return f;
}
