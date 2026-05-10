#pragma once

#include "views/surface/isurface.h"
#include <QString>
#include <QStringList>
#include <redasm/redasm.h>

class MainWindow;
class QToolButton;
class QKeyEvent;
class QLineEdit;
class QWidget;
class QMenu;

namespace utils {

inline MainWindow* mainwindow{nullptr};
inline QStringList search_paths;

QString to_hex_addr(RDAddress address, const RDSegment* seg = nullptr);
QString confidence_text(RDConfidence c);
QMenu* create_surface_menu(ISurface* surface);
QToolButton* create_screenshot_button(QWidget* w);
QPixmap get_logo();
bool handle_key_press(ISurface* surface, QKeyEvent* e);
void configure_hex_input(QLineEdit* le);

} // namespace utils
