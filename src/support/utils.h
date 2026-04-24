#pragma once

#include "views/surface/isurface.h"
#include <QString>
#include <QStringList>
#include <redasm/redasm.h>

class MainWindow;
class QPushButton;
class QKeyEvent;
class QWidget;
class QMenu;

namespace utils {

inline MainWindow* mainwindow{nullptr};
inline QStringList search_paths;

QString to_hex_addr(RDAddress address, const RDSegment* seg = nullptr);
QMenu* create_surface_menu(ISurface* surface);
QPushButton* create_screenshot_button(QWidget* w);
QPixmap get_logo();
bool handle_key_press(ISurface* surface, QKeyEvent* e);

} // namespace utils
