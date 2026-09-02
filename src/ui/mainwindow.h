#pragma once

#include "support/actions.h"
#include "support/fontawesome.h"
#include "views/log.h"
#include <QMainWindow>
#include <QMenuBar>
#include <QSplitter>
#include <QStackedWidget>
#include <QStatusBar>
#include <QToolBar>
#include <QToolButton>

#if !defined(NDEBUG) && defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
#include <QProcessEnvironment>
#include <QString>
#include <QStringList>
#endif

namespace ui {

struct MainWindow {
    QStatusBar* statusbar;
    QStackedWidget* stackwidget;
    QMenu *mnufile, *mnuedit, *mnuview, *mnutools, *mnuwindow, *mnuhelp;
    QMenu *mnurecents, *mnuexport, *mnudev;
    QAction *act_fileopen, *act_filesave, *act_filesaveas, *act_fileexportdb,
        *act_fileexportinput, *act_fileexportpatch, *act_fileclose,
        *act_fileexit;
    QAction* act_winrestoredefault;
    QAction *act_edit, *act_view, *act_tools;
    QAction *act_toolsflc, *act_toolsproblems;
    QAction *act_devdecoder, *act_devgraphs;
    QAction *act_viewmemorymap, *act_viewsegments, *act_viewmappings,
        *act_viewsegmentregs, *act_viewstrings, *act_viewtypedefs,
        *act_viewimported, *act_viewexported;
    QAction *act_tbseparator1, *act_tbseparator2, *act_tbseparator3,
        *act_tbseparator4;
    QAction *act_copy, *act_goto;
    ::LogView* logview;

    explicit MainWindow(QMainWindow* self) {
        self->setAcceptDrops(true);
        self->resize(1500, 850);

#if !defined(NDEBUG) && defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
        if(this->is_tiling_wm()) self->setFixedSize(1500, 850);
#endif

        actions::init(self);

        auto* menubar = new QMenuBar(self);
        this->mnufile = menubar->addMenu("&File");
        this->mnuedit = menubar->addMenu("&Edit");
        this->mnuview = menubar->addMenu("&View");
        this->mnutools = menubar->addMenu("&Tools");
        this->mnuwindow = menubar->addMenu("&Window");
        this->mnuhelp = menubar->addMenu("&?");

        this->act_edit = this->mnuedit->menuAction();
        this->act_edit->setVisible(false);

        this->act_view = this->mnuview->menuAction();
        this->act_view->setVisible(false);

        this->act_tools = this->mnutools->menuAction();

        this->act_fileopen = this->mnufile->addAction(
            FA_ICON(0xf07c), "&Open", QKeySequence{Qt::CTRL | Qt::Key_O});

        this->act_filesave = this->mnufile->addAction(
            FA_ICON(0xf0c7), "Save", QKeySequence{Qt::CTRL | Qt::Key_S});
        this->act_filesave->setVisible(false);

        this->act_filesaveas = this->mnufile->addAction(
            "Save as…", QKeySequence{Qt::CTRL | Qt::SHIFT | Qt::Key_S});
        this->act_filesaveas->setVisible(false);

        this->mnuexport = this->mnufile->addMenu(FA_ICON(0xf30b), "Export…");
        this->mnuexport->setVisible(false);

        this->act_fileexportdb = this->mnuexport->addAction("Database");
        this->act_fileexportinput = this->mnuexport->addAction("Input");
        this->act_fileexportpatch = this->mnuexport->addAction("Patched Input");

        this->act_fileclose = this->mnufile->addAction("Close");
        this->act_fileclose->setVisible(false);

        this->mnufile->addSeparator();
        this->mnurecents = new QMenu("&Recent Files", menubar);
        this->mnufile->addMenu(this->mnurecents);
        this->mnufile->addAction(actions::create(actions::OPEN_SETTINGS, self));
        this->act_fileexit = this->mnufile->addAction("&Exit");

        this->act_winrestoredefault =
            this->mnuwindow->addAction("Restore Default");

        this->act_toolsproblems = this->mnutools->addAction("&Problems");
        this->act_toolsproblems->setVisible(false);

        this->act_toolsflc = this->mnutools->addAction(
            "&FLC", QKeySequence{Qt::CTRL | Qt::Key_L});
        this->act_toolsflc->setVisible(false);

        this->act_tbseparator1 = this->mnutools->addSeparator();

        this->mnudev = this->mnutools->addMenu("Dev");
        this->act_devdecoder = this->mnudev->addAction("&Decoder/Encoder");
        this->act_devgraphs = this->mnudev->addAction("&Graphs");

        // clang-format off
        this->mnuhelp->addAction(actions::create(actions::OPEN_HOME, self));
        this->mnuhelp->addAction(actions::create(actions::OPEN_GITHUB, self));
        this->mnuhelp->addAction(actions::create(actions::OPEN_FEEDBACK, self));
        this->mnuhelp->addSeparator();
        this->mnuhelp->addAction(actions::create(actions::OPEN_ABOUT, self));

        this->act_copy = actions::create(actions::COPY, self);
        this->mnuedit->addAction(this->act_copy);
        this->mnuedit->addAction(actions::create(actions::SELECT_ALL, self));
        this->mnuedit->addSeparator();
        this->mnuedit->addAction(actions::create(actions::REANALYZE, self));
        // clang-format on

        this->act_viewmemorymap = this->mnuview->addAction(
            "Memory Map", QKeySequence{Qt::SHIFT | Qt::Key_F1});

        this->act_viewmappings = this->mnuview->addAction(
            FA_ICON(0xe697), "Mappings", QKeySequence{Qt::SHIFT | Qt::Key_F2});

        this->act_viewsegments = this->mnuview->addAction(
            FA_ICON(0xf200), "Segments", QKeySequence{Qt::SHIFT | Qt::Key_F3});

        this->act_viewsegmentregs = this->mnuview->addAction(
            "Segment Registers", QKeySequence{Qt::SHIFT | Qt::Key_F4});

        this->act_viewstrings = this->mnuview->addAction(
            FA_ICON(0xf031), "&Strings", QKeySequence{Qt::SHIFT | Qt::Key_F5});

        this->act_viewtypedefs =
            this->mnuview->addAction(FA_ICON(0xf1b3), "&Type Definitions",
                                     QKeySequence{Qt::SHIFT | Qt::Key_F6});

        this->mnuview->addSeparator();

        this->act_viewexported = this->mnuview->addAction(
            FA_ICON(0xf56e), "&Exported", QKeySequence{Qt::SHIFT | Qt::Key_F7});

        this->act_viewimported = this->mnuview->addAction(
            FA_ICON(0xf56f), "&Imported", QKeySequence{Qt::SHIFT | Qt::Key_F8});

        auto* toolbar = new QToolBar(self);
        toolbar->setObjectName("MainToolBar");
        toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        toolbar->setFloatable(false);
        toolbar->setMovable(false);
        toolbar->addAction(this->act_fileopen);
        toolbar->addAction(this->act_filesave);
        this->act_tbseparator2 = toolbar->addSeparator();
        this->act_goto = actions::create(actions::GOTO, toolbar);
        toolbar->addAction(this->act_goto);
        this->act_tbseparator3 = toolbar->addSeparator();
        toolbar->addAction(this->act_viewsegments);
        toolbar->addAction(this->act_viewmappings);
        this->act_tbseparator4 = toolbar->addSeparator();
        toolbar->addAction(this->act_viewexported);
        toolbar->addAction(this->act_viewimported);
        toolbar->addAction(this->act_viewstrings);
        self->addToolBar(toolbar);

        self->setMenuBar(menubar);

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
