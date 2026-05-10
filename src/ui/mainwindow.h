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

namespace ui {

struct MainWindow {
    QStatusBar* statusbar;
    QStackedWidget* stackwidget;
    QMenu *mnufile, *mnuedit, *mnuview, *mnutools, *mnuwindow, *mnuhelp;
    QMenu* mnurecents;
    QAction *actfileopen, *actfileclose, *actfileexit;
    QAction* actwinrestoredefault;
    QAction *acttoolsflc, *acttoolsdecoder, *acttoolsproblems;
    QAction *actedit, *actview, *acttools;
    QAction *actviewmemorymap, *actviewsegments, *actviewmappings,
        *actviewsegmentregs, *actviewstrings, *actviewimported,
        *actviewexported;
    QAction* acttbseparator;
    ::LogView* logview;

    explicit MainWindow(QMainWindow* self) {
        self->setAcceptDrops(true);
        self->resize(1500, 850);

#if !defined(NDEBUG)
        self->setFixedSize(1500, 850);
#endif

        actions::init(self);

        auto* menubar = new QMenuBar(self);
        this->mnufile = menubar->addMenu("&File");
        this->mnuedit = menubar->addMenu("&Edit");
        this->mnuview = menubar->addMenu("&View");
        this->mnutools = menubar->addMenu("&Tools");
        this->mnuwindow = menubar->addMenu("&Window");
        this->mnuhelp = menubar->addMenu("&?");

        this->actedit = this->mnuedit->menuAction();
        this->actedit->setVisible(false);

        this->actview = this->mnuview->menuAction();
        this->actview->setVisible(false);

        this->acttools = this->mnutools->menuAction();

        this->actfileopen = this->mnufile->addAction(FA_ICON(0xf07c), "&Open");
        this->mnurecents = new QMenu("&Recent Files", menubar);
        this->mnufile->addMenu(this->mnurecents);

        this->actfileclose = this->mnufile->addAction("Close");
        this->actfileclose->setVisible(false);

        this->mnufile->addSeparator();
        this->mnufile->addAction(actions::get(actions::OPEN_SETTINGS));
        this->actfileexit = this->mnufile->addAction("&Exit");

        this->actwinrestoredefault =
            this->mnuwindow->addAction("Restore Default");

        this->acttoolsflc = this->mnutools->addAction(
            "&FLC", QKeySequence{Qt::CTRL | Qt::Key_L});
        this->acttoolsflc->setVisible(false);

        this->mnuhelp->addAction(actions::get(actions::OPEN_ABOUT));
        this->mnuhelp->addSeparator();
        this->mnuhelp->addAction(actions::get(actions::OPEN_HOME));
        this->mnuhelp->addAction(actions::get(actions::OPEN_GITHUB));

        this->acttoolsproblems = this->mnutools->addAction("&Problems");
        this->acttoolsproblems->setVisible(false);

        this->acttoolsdecoder = this->mnutools->addAction("&Decoder");

        this->actviewmemorymap = this->mnuview->addAction(
            "Memory Map", QKeySequence{Qt::SHIFT | Qt::Key_F1});

        this->actviewmappings = this->mnuview->addAction(
            "Mappings", QKeySequence{Qt::SHIFT | Qt::Key_F2});

        this->actviewsegments = this->mnuview->addAction(
            "Segments", QKeySequence{Qt::SHIFT | Qt::Key_F3});

        this->actviewsegmentregs = this->mnuview->addAction(
            "Segment Registers", QKeySequence{Qt::SHIFT | Qt::Key_F4});

        this->actviewstrings = this->mnuview->addAction(
            FA_ICON(0xf031), "&Strings", QKeySequence{Qt::SHIFT | Qt::Key_F5});

        this->mnuview->addSeparator();

        this->actviewexported = this->mnuview->addAction(
            FA_ICON(0xf56e), "&Exported", QKeySequence{Qt::SHIFT | Qt::Key_F6});

        this->actviewimported = this->mnuview->addAction(
            FA_ICON(0xf56f), "&Imported", QKeySequence{Qt::SHIFT | Qt::Key_F7});

        auto* toolbar = new QToolBar(self);
        toolbar->setObjectName("MainToolBar");
        toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        toolbar->setFloatable(false);
        toolbar->setMovable(false);
        toolbar->addAction(this->actfileopen);
        this->acttbseparator = toolbar->addSeparator();
        toolbar->addAction(this->actviewexported);
        toolbar->addAction(this->actviewimported);
        toolbar->addAction(this->actviewstrings);
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
};

} // namespace ui
