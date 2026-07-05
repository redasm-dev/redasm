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

    OP_AS_ADDRESS,
    OP_AS_IMMEDIATE,
    PATCH_INSTRUCTION,

    OPEN_DETAILS,

    OPEN_HOME,
    OPEN_DISCORD,
    OPEN_X,
    OPEN_MASTODON,
    OPEN_GITHUB,

    OPEN_SETTINGS,
    OPEN_ABOUT,
};

void init(QMainWindow* mw);
QAction* get(Type t);

} // namespace actions
