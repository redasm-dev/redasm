#pragma once

#include "support/actions.h"
#include "support/fontawesome.h"
#include "views/log.h"
#include <QHash>
#include <QList>
#include <QMainWindow>
#include <QMenuBar>
#include <QSplitter>
#include <QStackedWidget>
#include <QStatusBar>
#include <QToolBar>
#include <QToolButton>
#include <variant>

#if !defined(NDEBUG) && defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
#include <QProcessEnvironment>
#include <QString>
#include <QStringList>
#endif

#define MW_ACTION_ITEM(type, act_type, kind, icon, text, key, children)        \
    {                                                                          \
        MWActionType::type,                                                    \
        MWNodeKind::kind,                                                      \
        actions::Type::act_type,                                               \
        icon,                                                                  \
        text,                                                                  \
        key,                                                                   \
        children,                                                              \
    }

#define MW_ACTION_SEP MW_ACTION_ITEM(NONE, NONE, SEPARATOR, {}, {}, {}, {})

#define MW_ACTION_TREE(icon, text, children)                                   \
    MW_ACTION_ITEM(NONE, NONE, MENU, icon, text, {}, children)

#define MW_ACTION_TREE_ID(type, icon, text, children)                          \
    MW_ACTION_ITEM(type, NONE, MENU, icon, text, {}, children)

#define MW_ACTION_TREE_DYNAMIC(type, icon, text)                               \
    MW_ACTION_ITEM(type, NONE, MENU, icon, text, {}, {})

#define MW_ACTION_TYPE(type, act_type)                                         \
    MW_ACTION_ITEM(type, act_type, LEAF, {}, {}, {}, {})

#define MW_ACTION(type, icon, text, key)                                       \
    MW_ACTION_ITEM(type, NONE, LEAF, icon, text, key, {})

namespace ui {

enum class MWActionType {
    NONE = 0,

    VIEW,
    ANALYSIS,

    FILE_OPEN,
    FILE_SAVE,
    FILE_SAVE_AS,
    FILE_CLOSE,
    FILE_EXPORT,
    FILE_RECENTS,
    FILE_SETTINGS,
    FILE_EXIT,

    FILE_EXPORT_DATABASE,
    FILE_EXPORT_INPUT,
    FILE_EXPORT_PATCH,

    VIEW_MEMORY_MAP,
    VIEW_MAPPINGS,
    VIEW_SEGMENTS,
    VIEW_SEGMENT_REGS,
    VIEW_STRINGS,
    VIEW_TYPEDEFS,
    VIEW_EXPORTED,
    VIEW_IMPORTED,

    ANALYSIS_GOTO,
    ANALYSIS_REANALYZE,
    ANALYSIS_REBASE,
    ANALYSIS_PROBLEMS,

    TOOLS_FLC,

    TOOLS_DEV_DECODER,
    TOOLS_DEV_GRAPHS,

    WINDOW_RESTORE_DEFAULT,
};

namespace detail {

enum class MWNodeKind {
    LEAF,      // QAction
    MENU,      // QMenu
    SEPARATOR, // addSeparator()
};

struct MWMenuAction {
    MWActionType type;
    MWNodeKind kind;
    actions::Type action_type;

    QIcon icon;
    QString text;
    QKeySequence key;

    QList<MWMenuAction> actions;
};

using MWActionItem = std::variant<QMenu*, QAction*>;

class MWActions {
public:
    void insert(MWActionType t, MWActionItem item) { m_map.insert(t, item); }

    [[nodiscard]] QAction* action(MWActionType t) const {
        auto it = m_map.find(t);
        Q_ASSERT_X(it != m_map.end(), "MWActions::action", "unknown key");
        return std::get<QAction*>(
            *it); // throws std::bad_variant_access if it's a menu
    }

    [[nodiscard]] QMenu* menu(MWActionType t) const {
        auto it = m_map.find(t);
        Q_ASSERT_X(it != m_map.end(), "MWActions::menu", "unknown key");
        return std::get<QMenu*>(*it);
    }

private:
    QHash<MWActionType, MWActionItem> m_map;
};

inline const QList<MWMenuAction>& MENU_BAR() { // NOLINT
    static const QList<MWMenuAction> DATA = []() {
        // clang-format off
        const QList<MWMenuAction> MENU_FILE_EXPORT = {
            MW_ACTION(FILE_EXPORT_DATABASE, {}, "Database", {}),
            MW_ACTION(FILE_EXPORT_INPUT, {}, "Input", {}),
            MW_ACTION(FILE_EXPORT_PATCH, {}, "Patched Input", {}),
        };

        const QList<MWMenuAction> MENU_TOOLS_DEV = {
            MW_ACTION(TOOLS_DEV_DECODER, {}, "&Decoder/Encoder", {}),
            MW_ACTION(TOOLS_DEV_GRAPHS, {}, "&Graphs", {}),
        };

        const QList<MWMenuAction> MENU_FILE = {
            MW_ACTION(FILE_OPEN, FA_ICON(0xf07c), "&Open", QKeySequence{Qt::CTRL | Qt::Key_O}),
            MW_ACTION(FILE_SAVE, FA_ICON(0xf0c7), "Save", QKeySequence{Qt::CTRL | Qt::Key_S}),
            MW_ACTION(FILE_SAVE_AS, {}, "Save as…", QKeySequence{Qt::CTRL | Qt::SHIFT | Qt::Key_S}),
            MW_ACTION_TREE_ID(FILE_EXPORT, FA_ICON(0xf30b), "Export…", MENU_FILE_EXPORT),
            MW_ACTION(FILE_CLOSE, {}, "Close", {}),
            MW_ACTION_SEP,
            MW_ACTION_TREE_DYNAMIC(FILE_RECENTS, {}, "&Recent Files"),
            MW_ACTION_TYPE(NONE, OPEN_SETTINGS),
            MW_ACTION(FILE_EXIT, {}, "Exit", {}),
        };

        const QList<MWMenuAction> MENU_VIEW = {
            MW_ACTION(VIEW_MEMORY_MAP, {}, "Memory Map", QKeySequence{Qt::SHIFT | Qt::Key_F1}),
            MW_ACTION(VIEW_MAPPINGS, FA_ICON(0xe697), "Mappings", QKeySequence{Qt::SHIFT | Qt::Key_F2}),
            MW_ACTION(VIEW_SEGMENTS, FA_ICON(0xf200), "Segments", QKeySequence{Qt::SHIFT | Qt::Key_F3}),
            MW_ACTION(VIEW_SEGMENT_REGS, {}, "Segment Registers", QKeySequence{Qt::SHIFT | Qt::Key_F4}),
            MW_ACTION(VIEW_STRINGS, FA_ICON(0xf031), "&Strings", QKeySequence{Qt::SHIFT | Qt::Key_F5}),
            MW_ACTION(VIEW_TYPEDEFS, FA_ICON(0xf1b3), "&Type Definitions", QKeySequence{Qt::SHIFT | Qt::Key_F6}),
            MW_ACTION_SEP,
            MW_ACTION(VIEW_EXPORTED, FA_ICON(0xf56e), "&Exported", QKeySequence{Qt::SHIFT | Qt::Key_F7}),
            MW_ACTION(VIEW_IMPORTED, FA_ICON(0xf56f), "&Imported", QKeySequence{Qt::SHIFT | Qt::Key_F8}),
        };

        const QList<MWMenuAction> MENU_ANALYSIS = {
            MW_ACTION_TYPE(ANALYSIS_GOTO, GOTO),
            MW_ACTION(ANALYSIS_PROBLEMS, {}, "&Problems", {}),
            MW_ACTION_SEP,
            MW_ACTION(ANALYSIS_REBASE, {}, "Rebase", {}),
            MW_ACTION(ANALYSIS_REANALYZE, {}, "Reanalyze", {}),
        };

        const QList<MWMenuAction> MENU_TOOLS = {
            MW_ACTION(TOOLS_FLC, {}, "&FLC", {}),
            MW_ACTION_TREE({}, "Dev", MENU_TOOLS_DEV),
        };

        const QList<MWMenuAction> MENU_WINDOW = {
            MW_ACTION(WINDOW_RESTORE_DEFAULT, {}, "Restore Default", {}),
        };

        const QList<MWMenuAction> MENU_HELP = {
            MW_ACTION_TYPE(NONE, OPEN_HOME),
            MW_ACTION_TYPE(NONE, OPEN_GITHUB),
            MW_ACTION_TYPE(NONE, OPEN_FEEDBACK),
            MW_ACTION_TYPE(NONE, OPEN_ABOUT),
        };

        return QList<MWMenuAction>{
            MW_ACTION_TREE({}, "&File", MENU_FILE),
            MW_ACTION_TREE_ID(VIEW, {}, "&View", MENU_VIEW),
            MW_ACTION_TREE_ID(ANALYSIS, {}, "&Analysis", MENU_ANALYSIS),
            MW_ACTION_TREE({}, "&Tools", MENU_TOOLS),
            MW_ACTION_TREE({}, "&Window", MENU_WINDOW),
            MW_ACTION_TREE({}, "&?", MENU_HELP),
        };
    }();

    return DATA;
}
// clang-format on

template<typename QtContainer>
void build_actions_tree(QtContainer* parent, QWidget* owner,
                        const QList<MWMenuAction>& items,
                        MWActions& mw_actions) {
    for(const MWMenuAction& item : items) {
        switch(item.kind) {
            case MWNodeKind::SEPARATOR: parent->addSeparator(); break;

            case MWNodeKind::MENU: {
                QMenu* menu = parent->addMenu(item.icon, item.text);
                if(item.type != MWActionType::NONE)
                    mw_actions.insert(item.type, menu);
                if(!item.actions.isEmpty())
                    build_actions_tree(menu, owner, item.actions, mw_actions);
                break;
            }

            case MWNodeKind::LEAF: {
                QAction* action;

                if(item.action_type != actions::Type::NONE) {
                    Q_ASSERT_X(
                        item.icon.isNull() && item.text.isEmpty() &&
                            item.key.isEmpty(),
                        "buildActions",
                        "icon/text/key are ignored when action_type is set; "
                        "actions::create() supplies them");

                    action = actions::create(item.action_type, owner);
                    parent->addAction(action);
                }
                else {
                    action = new QAction(item.icon, item.text, owner);
                    if(!item.key.isEmpty()) action->setShortcut(item.key);
                    parent->addAction(action);
                }

                if(item.type != MWActionType::NONE)
                    mw_actions.insert(item.type, action);

                break;
            }

            default: qFatal("Invalid MWNodeKind");
        }
    }
}

}; // namespace detail

struct MainWindow {
    QStatusBar* statusbar;
    QStackedWidget* stackwidget;
    QAction *act_tbseparator1, *act_tbseparator2, *act_tbseparator3;
    ::LogView* logview;

    detail::MWActions mw_actions;

    explicit MainWindow(QMainWindow* self) {
        self->setAcceptDrops(true);
        self->resize(1500, 850);

#if !defined(NDEBUG) && defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
        if(this->is_tiling_wm()) self->setFixedSize(1500, 850);
#endif

        actions::init(self);

        auto* menubar = new QMenuBar(self);
        detail::build_actions_tree(menubar, self, detail::MENU_BAR(),
                                   this->mw_actions);
        self->setMenuBar(menubar);

        auto* toolbar = new QToolBar(self);
        toolbar->setObjectName("MainToolBar");
        toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        toolbar->setFloatable(false);
        toolbar->setMovable(false);
        toolbar->addAction(mw_actions.action(MWActionType::FILE_OPEN));
        toolbar->addAction(mw_actions.action(MWActionType::FILE_SAVE));
        this->act_tbseparator1 = toolbar->addSeparator();
        toolbar->addAction(mw_actions.action(MWActionType::ANALYSIS_GOTO));
        this->act_tbseparator2 = toolbar->addSeparator();
        toolbar->addAction(mw_actions.action(MWActionType::VIEW_SEGMENTS));
        toolbar->addAction(mw_actions.action(MWActionType::VIEW_MAPPINGS));
        this->act_tbseparator3 = toolbar->addSeparator();
        toolbar->addAction(mw_actions.action(MWActionType::VIEW_EXPORTED));
        toolbar->addAction(mw_actions.action(MWActionType::VIEW_IMPORTED));
        toolbar->addAction(mw_actions.action(MWActionType::VIEW_STRINGS));
        self->addToolBar(toolbar);

        this->statusbar = new QStatusBar(self);
        self->setStatusBar(this->statusbar);

        this->stackwidget = new QStackedWidget();
        this->logview = new ::LogView();

        auto* vsplit = new QSplitter(Qt::Vertical);
        vsplit->addWidget(this->stackwidget);
        vsplit->addWidget(this->logview);
        vsplit->setStretchFactor(0, 10);
        vsplit->setStretchFactor(1, 1);

        self->setCentralWidget(vsplit);
    }

#if !defined(NDEBUG) && defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
private:
    bool is_tiling_wm() {
        const QProcessEnvironment ENV =
            QProcessEnvironment::systemEnvironment();

        // Most tiling WMs set their own IPC socket env var
        // the most reliable signal
        static const QStringList TILING_ENV_HINTS = {
            "I3SOCK",                      // i3
            "SWAYSOCK",                    // sway
            "HYPRLAND_INSTANCE_SIGNATURE", // Hyprland
            "BSPWM_SOCKET",                // bspwm
            "QTILE_XEPHYR",                // qtile (partial)
        };

        for(const auto& var : TILING_ENV_HINTS) {
            if(ENV.contains(var)) return true;
        }

        // Fallback: match XDG_CURRENT_DESKTOP / DESKTOP_SESSION against known
        // tiling WM names (covers ones without a dedicated socket env var)
        static const QStringList TILING_WM_NAMES = {
            "i3",           "sway",  "bspwm",    "awesome", "dwm",  "xmonad",
            "herbstluftwm", "qtile", "spectrwm", "leftwm",  "river"};

        QString desktop = ENV.value("XDG_CURRENT_DESKTOP").toLower();
        QString session = ENV.value("DESKTOP_SESSION").toLower();

        for(const auto& name : TILING_WM_NAMES) { // NOLINT
            if(desktop.contains(name) || session.contains(name)) return true;
        }

        return false;
    }

#endif
};

} // namespace ui
