#include "mainwindow.h"
#include "dialogs/analyzer.h"
#include "dialogs/decoder.h"
#include "dialogs/flc.h"
#include "dialogs/loader.h"
#include "dialogs/memorymap.h"
#include "dialogs/table.h"
#include "models/exported.h"
#include "models/imported.h"
#include "models/mappings.h"
#include "models/problems.h"
#include "models/registers.h"
#include "models/segments.h"
#include "models/symbols.h"
#include "models/symbolsfilter.h"
#include "support/actions.h"
// #include "rdui/qtui.h"
#include "statusbar.h"
#include "support/settings.h"
#include "support/utils.h"
#include "views/welcome.h"
#include <QDirIterator>
#include <QDragMoveEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QMimeData>
#include <QStandardPaths>
#include <QTimer>

MainWindow::MainWindow(QWidget* parent): QMainWindow{parent}, m_ui{this} {
    utils::mainwindow = this;

    this->setWindowIcon(utils::get_logo());
    this->load_window_state();
    this->load_recents();
    this->update_menubar();

    m_ui.statusbar->addPermanentWidget(statusbar::create_status_label(this),
                                       60);

    m_ui.statusbar->addPermanentWidget(
        statusbar::create_problems_button(m_ui.statusbar->height(), this));

    m_ui.statusbar->addPermanentWidget(
        statusbar::create_status_button(m_ui.statusbar->height(), this));

    this->show_welcome_view();

    connect(m_ui.actfileexit, &QAction::triggered, this, &MainWindow::close);
    connect(m_ui.actfileopen, &QAction::triggered, this,
            &MainWindow::select_file);
    connect(m_ui.actfileclose, &QAction::triggered, this,
            &MainWindow::show_welcome_view);
    connect(m_ui.actviewsegments, &QAction::triggered, this,
            &MainWindow::show_segments);
    connect(m_ui.actviewmappings, &QAction::triggered, this,
            &MainWindow::show_mappings);
    connect(m_ui.actviewtrackedregisters, &QAction::triggered, this,
            &MainWindow::show_tracked_registers);
    connect(m_ui.actviewstrings, &QAction::triggered, this,
            &MainWindow::show_strings);
    connect(m_ui.actviewexported, &QAction::triggered, this,
            &MainWindow::show_exported);
    connect(m_ui.actviewimported, &QAction::triggered, this,
            &MainWindow::show_imported);

    connect(m_ui.actviewmemorymap, &QAction::triggered, this, [&]() {
        ContextView* ctxview = this->context_view();
        if(!ctxview) return;

        auto* dlgmemorymap = new MemoryMapDialog(ctxview->context(), this);
        dlgmemorymap->show();
    });

    connect(m_ui.acttoolsflc, &QAction::triggered, this, [&]() {
        ContextView* ctxview = this->context_view();
        if(!ctxview) return;
        auto* dlgflc = new FLCDialog(ctxview->context(), this);
        dlgflc->show();
    });

    connect(m_ui.acttoolsdecoder, &QAction::triggered, this, [&]() {
        auto* dlgdecoder = new DecoderDialog(this);
        dlgdecoder->show();
    });

    connect(m_ui.acttoolsproblems, &QAction::triggered, this,
            &MainWindow::show_problems);

    connect(m_ui.actwinrestoredefault, &QAction::triggered, this,
            [&]() { REDasmSettings{}.restore_state(this); });

    connect(statusbar::problems_button(), &QPushButton::clicked, this,
            &MainWindow::show_problems);

    connect(statusbar::status_button(), &QPushButton::clicked, this, [&]() {
        ContextView* ctxview = this->context_view();
        if(ctxview) ctxview->toggle_active();
    });
}

void MainWindow::dragEnterEvent(QDragEnterEvent* e) {
    if(!e->mimeData()->hasUrls()) return;
    e->acceptProposedAction();
}

void MainWindow::dragMoveEvent(QDragMoveEvent* e) {
    if(!e->mimeData()->hasUrls()) return;
    e->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent* e) {
    const QMimeData* mimedata = e->mimeData();
    if(!mimedata->hasUrls()) return;

    QList<QUrl> urllist = mimedata->urls();
    QString filepath = urllist.first().toLocalFile();

    QFileInfo fi(filepath);
    if(!fi.isFile()) return;

    this->open_file(filepath);
    e->acceptProposedAction();
}

void MainWindow::load_window_state() {
    if(REDasmSettings{}.restore_state(this)) return;

    QRect position = this->frameGeometry();
    position.moveCenter(qApp->primaryScreen()->availableGeometry().center());
    this->move(position.topLeft());
}

void MainWindow::load_recents() {
    m_ui.mnurecents->clear();

    REDasmSettings settings;
    QStringList recents = settings.recent_files();
    m_ui.mnurecents->setEnabled(!recents.empty());

    for(int i = 0; i < REDasmSettings::MAX_RECENT_FILES; i++) {
        if(i >= recents.length()) {
            QAction* action = m_ui.mnurecents->addAction(QString{});
            action->setVisible(false);
            continue;
        }

        if(!QFileInfo().exists(recents[i])) continue;

        QAction* action = m_ui.mnurecents->addAction(
            QString("%1 - %2").arg(i).arg(recents[i]));
        action->setData(recents[i]);

        connect(action, &QAction::triggered, this,
                [=]() { this->open_file(action->data().toString()); });
    }

    if(recents.empty()) return;

    m_ui.mnurecents->addSeparator();
    QAction* action = m_ui.mnurecents->addAction(tr("Clear"));

    connect(action, &QAction::triggered, this,
            [=]() { this->clear_recents(); });
}

void MainWindow::replace_view(QWidget* w) { // NOLINT
    w->setParent(m_ui.stackwidget);
    QWidget* oldwidget = m_ui.stackwidget->currentWidget();
    m_ui.stackwidget->addWidget(w);

    if(oldwidget) {
        m_ui.stackwidget->removeWidget(oldwidget);
        oldwidget->deleteLater();
    }
}

void MainWindow::show_welcome_view() { // NOLINT
    m_filepath.clear();
    this->setWindowTitle({});

    auto* welcomeview = new WelcomeView(m_ui.stackwidget);

    connect(welcomeview, &WelcomeView::open_requested, this,
            &MainWindow::select_file);
    connect(welcomeview, &WelcomeView::file_selected, this,
            &MainWindow::open_file);

    this->replace_view(welcomeview);
    this->enable_context_actions(false);
}

void MainWindow::show_problems() {
    ContextView* ctxview = this->context_view();
    if(!ctxview) return;

    auto* dlg = new TableDialog("Problems", this);

    QObject::connect(dlg, &TableDialog::double_clicked, this,
                     [&, dlg](const QModelIndex& index) {
                         if(ContextView* cv = this->context_view(); cv) {
                             auto* m =
                                 static_cast<ProblemsModel*>(dlg->model());
                             cv->surface()->jump_to(m->address(index));
                             dlg->accept();
                         }
                     });

    dlg->set_model(new ProblemsModel(ctxview->context(), dlg));
    dlg->show();
}

void MainWindow::show_context_view(RDContext* ctx) {
    QFileInfo fi{m_filepath};
    QDir::setCurrent(fi.path());
    this->setWindowTitle(fi.fileName());

    this->replace_view(new ContextView(ctx));
    this->enable_context_actions(true);
}

void MainWindow::update_menubar() {
    QAction* actcopy = actions::get(actions::COPY);
    m_ui.mnuedit->addAction(actcopy);

    connect(m_ui.mnuedit, &QMenu::aboutToShow, this, [&, actcopy]() {
        ContextView* cv = this->context_view();
        actcopy->setVisible(cv && cv->surface()->has_selection());
    });
}

void MainWindow::clear_recents() {
    REDasmSettings settings;
    settings.clear_recent_files();
    this->load_recents();

    if(!this->context_view()) this->show_welcome_view(); // Recreate Welcome Tab
}

ContextView* MainWindow::context_view() const {
    return qobject_cast<ContextView*>(m_ui.stackwidget->currentWidget());
}

bool MainWindow::loop() { // NOLINT
    ContextView* cv = this->context_view();
    return cv && cv->loop();
}

bool MainWindow::can_close() const {
    if(this->context_view()) {
        QMessageBox msgbox{const_cast<MainWindow*>(this)};
        msgbox.setWindowTitle(tr("Closing"));
        msgbox.setText(tr("Are you sure?"));
        msgbox.setStandardButtons(QMessageBox::Yes | QMessageBox::No |
                                  QMessageBox::Cancel);

        if(msgbox.exec() != QMessageBox::Yes) return false;
    }

    return true;
}

void MainWindow::enable_context_actions(bool e) { // NOLINT
    m_ui.actfileclose->setVisible(e);
    m_ui.actedit->setVisible(e);
    m_ui.actview->setVisible(e);

    m_ui.acttbseparator->setVisible(e);
    m_ui.acttoolsflc->setVisible(e);
    m_ui.acttoolsproblems->setVisible(e);
    m_ui.actviewexported->setVisible(e);
    m_ui.actviewimported->setVisible(e);
    m_ui.actviewstrings->setVisible(e);

    if(!e) {
        statusbar::set_status_text(QString{});
        statusbar::problems_button()->hide();
        statusbar::status_button()->hide();
        m_ui.logview->clear();
    }
}

void MainWindow::open_file(const QString& filepath) {
    if(filepath.isEmpty()) return;

    m_filepath = filepath;

    RDContextSlice ctxslice = rd_test(qUtf8Printable(m_filepath));
    if(rd_slice_is_empty(ctxslice)) return;

    REDasmSettings settings;
    settings.update_recent_files(m_filepath);
    this->load_recents();

    auto* dlgloader = new LoaderDialog(ctxslice, this);

    connect(dlgloader, &LoaderDialog::accepted, this, [&, dlgloader]() {
        if(rd_accept(dlgloader->context, dlgloader->processorplugin,
                     &dlgloader->addressing))
            this->select_analyzers(dlgloader->context);
        else
            QMessageBox::information(this, "Loader",
                                     "Loading failed or aborted");
    });

    connect(dlgloader, &LoaderDialog::rejected, this, []() { rd_reject(); });
    dlgloader->open();
}

void MainWindow::select_file() {
    QString s = QFileDialog::getOpenFileName(this, "Disassemble file...");
    if(!s.isEmpty()) this->open_file(s);
}

void MainWindow::log(RDLogLevel level, const QString& tag, // NOLINT
                     const QString& msg) {
    m_ui.logview->log(level, tag, msg);
}

void MainWindow::init_searchpaths() {
    const char* appdir = std::getenv("APPDIR");
    bool isappimage = appdir && std::getenv("APPIMAGE");

    utils::search_paths =
        QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);

    if(isappimage) utils::search_paths.prepend(QString::fromUtf8(appdir));

    utils::search_paths.prepend(qApp->applicationDirPath() + "/../processors");
    utils::search_paths.prepend(qApp->applicationDirPath() + "/../loaders");
    utils::search_paths.prepend(qApp->applicationDirPath() + "/../commands");
    utils::search_paths.prepend(qApp->applicationDirPath());

    for(const QString& searchpath : utils::search_paths) {
        QDirIterator it{searchpath, QDirIterator::Subdirectories};

        while(it.hasNext()) {
            QFileInfo fi = it.nextFileInfo();

            if(fi.isFile() && ("." + fi.suffix() == SHARED_OBJECT_EXT))
                rd_module_load(qUtf8Printable(fi.filePath()));
        }
    }
}

void MainWindow::select_analyzers(RDContext* ctx) {
    if(rd_get_analyzer_plugins(ctx).length) {
        auto* dlganalyzers = new AnalyzerDialog(ctx, this);
        connect(dlganalyzers, &AnalyzerDialog::accepted, this,
                [&, ctx]() { this->show_context_view(ctx); });
        connect(dlganalyzers, &AnalyzerDialog::rejected, this,
                [&, ctx]() { rd_destroy(ctx); });
        dlganalyzers->show();
    }
    else
        this->show_context_view(ctx);
}

void MainWindow::show_segments() {
    ContextView* ctxview = this->context_view();
    if(!ctxview) return;

    auto* dlg = new TableDialog("Segments", this);

    connect(dlg, &TableDialog::double_clicked, this,
            [&, ctxview, dlg](const QModelIndex& index) {
                auto* m = static_cast<SegmentsModel*>(dlg->model());
                ctxview->surface()->jump_to(m->address(index));
                dlg->accept();
            });

    dlg->set_model(new SegmentsModel(ctxview->context(), dlg));
    dlg->resize_column(0, QHeaderView::Stretch);
    dlg->show();
}

void MainWindow::show_mappings() {
    ContextView* ctxview = this->context_view();
    if(!ctxview) return;

    auto* dlg = new TableDialog("Mappings", this);

    connect(dlg, &TableDialog::double_clicked, this,
            [&, ctxview, dlg](const QModelIndex& index) {
                auto* m = static_cast<MappingsModel*>(dlg->model());
                ctxview->surface()->jump_to(m->address(index));
                dlg->accept();
            });

    dlg->set_model(new MappingsModel(ctxview->context(), dlg));
    dlg->show();
}

void MainWindow::show_tracked_registers() {
    ContextView* ctxview = this->context_view();
    if(!ctxview) return;

    static TableDialog* dlg = nullptr;

    if(dlg) {
        dlg->activateWindow();
        return;
    }

    dlg = new TableDialog("Tracked Registers", this);

    connect(dlg, &TableDialog::double_clicked, this,
            [&, ctxview](const QModelIndex& index) {
                auto* m = static_cast<RegistersModel*>(dlg->model());
                ctxview->surface()->jump_to(m->address(index));
            });

    connect(dlg, &TableDialog::closed, this, []() { dlg = nullptr; });

    dlg->set_model(new RegistersModel(ctxview->context(), dlg));
    dlg->show();
}

void MainWindow::show_strings() {
    ContextView* ctxview = this->context_view();
    if(!ctxview) return;

    auto* dlg = new TableDialog("Strings", this);

    connect(dlg, &TableDialog::double_clicked, this,
            [&, dlg](const QModelIndex& index) {
                ContextView* ctxview = this->context_view();
                if(!ctxview) return;

                auto* m = static_cast<SymbolsModel*>(dlg->model());
                ctxview->surface()->jump_to(m->address(index));
                dlg->accept();
            });

    auto* symbolsmodel =
        new SymbolsFilterModel(ctxview->context(), RD_SYMBOL_STRING, true, dlg);

    symbolsmodel->set_symbol_column_text("String");

    dlg->set_model(symbolsmodel);
    dlg->set_stretch_last_column(false);
    dlg->resize_column(2, QHeaderView::Stretch);
    dlg->resize_column(3, QHeaderView::ResizeToContents);
    dlg->hide_column(1);
    dlg->show();
}

void MainWindow::show_exported() {
    ContextView* ctxview = this->context_view();
    if(!ctxview) return;

    auto* dlg = new TableDialog("Exported", this);

    connect(dlg, &TableDialog::double_clicked, this,
            [&, dlg](const QModelIndex& index) {
                ContextView* ctxview = this->context_view();
                if(!ctxview) return;

                auto* m = static_cast<ExportedModel*>(dlg->model());
                ctxview->surface()->jump_to(m->address(index));
                dlg->accept();
            });

    dlg->set_model(new ExportedModel(ctxview->context(), dlg));
    dlg->set_stretch_last_column(false);
    dlg->resize_column(1, QHeaderView::Stretch);
    dlg->show();
}

void MainWindow::show_imported() {
    ContextView* ctxview = this->context_view();
    if(!ctxview) return;

    auto* dlg = new TableDialog("Imported", this);

    connect(dlg, &TableDialog::double_clicked, this,
            [&, dlg](const QModelIndex& index) {
                ContextView* ctxview = this->context_view();
                if(!ctxview) return;

                auto* m = static_cast<ImportedModel*>(dlg->model());
                ctxview->surface()->jump_to(m->address(index));
                dlg->accept();
            });

    dlg->set_model(new ImportedModel(ctxview->context(), dlg));
    dlg->set_stretch_last_column(false);
    dlg->resize_column(2, QHeaderView::Stretch);
    dlg->show();
}

void MainWindow::closeEvent(QCloseEvent* e) {
    if(this->can_close()) {
        REDasmSettings{}.save_state(this);
        QWidget::closeEvent(e);
        Q_EMIT closed();
    }
    else
        e->ignore();
}
