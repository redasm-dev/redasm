#pragma once

#include <QAction>

class QMainWindow;

namespace actions {

enum Type : quint8 {
    GOTO,
    COPY,
    REFS_TO,
    RENAME,
    COMMENT,

    OPEN_DETAILS,

    OPEN_HOME,
    OPEN_TELEGRAM,
    OPEN_REDDIT,
    OPEN_X,
    OPEN_GITHUB,

    OPEN_SETTINGS,
    OPEN_ABOUT,
};

void init(QMainWindow* mw);
QAction* get(Type t);

} // namespace actions
