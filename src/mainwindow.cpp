#include "mainwindow.h"
#include "dialogs/analyzer.h"
#include "dialogs/decoder.h"
#include "dialogs/devgraphs.h"
#include "dialogs/flc.h"
#include "dialogs/loader.h"
#include "dialogs/memorymap.h"
#include "dialogs/table.h"
#include "dialogs/typedefs.h"
#include "models/externals.h"
#include "models/mappings.h"
#include "models/problems.h"
#include "models/segments.h"
#include "models/strings.h"
#include "support/actions.h"
#include "support/surfacerenderer.h"
// #include "rdui/qtui.h"
#include "statusbar.h"
#include "support/settings.h"
#include "support/utils.h"
#include "views/welcome.h"
#include <QDragMoveEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QMimeData>

namespace {

void on_log(RDLogLevel level, const char* tag, const char* msg,
            void* userdata) {
    auto* mw = reinterpret_cast<MainWindow*>(userdata);
    mw->log(level, QString::fromUtf8(tag), QString::fromUtf8(msg));
}

} // namespace

MainWindow::MainWindow(const RDInitParams& params, QWidget* parent)
    : QMainWindow{parent}, m_ui{this} {
    utils::mainwindow = this;

    rd_set_log_callback(on_log, this);
    surface_renderer::init();
    rd_init(&params);

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
    connect(m_ui.actfilesave, &QAction::triggered, this,
            &MainWindow::save_project);
    connect(m_ui.actfilesaveas, &QAction::triggered, this,
            &MainWindow::save_project_as);
    connect(m_ui.actfileclose, &QAction::triggered, this,
            &MainWindow::show_welcome_view);
    connect(m_ui.actfileexportdb, &QAction::triggered, this,
            &MainWindow::export_db);
    connect(m_ui.actfileexportinput, &QAction::triggered, this,
            &MainWindow::export_input);
    connect(m_ui.actfileexportpatch, &QAction::triggered, this,
            &MainWindow::export_input_patch);
    connect(m_ui.actviewsegments, &QAction::triggered, this,
            &MainWindow::show_segments);
    connect(m_ui.actviewmappings, &QAction::triggered, this,
            &MainWindow::show_mappings);
    connect(m_ui.actviewsegmentregs, &QAction::triggered, this,
            &MainWindow::show_segment_regs);
    connect(m_ui.actviewstrings, &QAction::triggered, this,
            &MainWindow::show_strings);
    connect(m_ui.actviewtypedefs, &QAction::triggered, this,
            &MainWindow::show_typedefs);
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

    connect(m_ui.actdevdecoder, &QAction::triggered, this, [&]() {
        auto* dlgdecoder = new DecoderDialog(this);
        dlgdecoder->show();
    });

    connect(m_ui.actdevgraphs, &QAction::triggered, this, [&]() {
        ContextView* ctxview = this->context_view();
        if(!ctxview) return;
        auto* dlggraphdots = new DevGraphsDialog(ctxview->context(), this);
        dlggraphdots->show();
    });

    connect(m_ui.acttoolsproblems, &QAction::triggered, this,
            &MainWindow::show_problems);

    connect(m_ui.actwinrestoredefault, &QAction::triggered, this,
            [&]() { REDasmSettings{}.restore_state(this); });

    connect(statusbar::problems_button(), &QPushButton::clicked, this,
            &MainWindow::show_problems);

    connect(statusbar::status_button(), &QPushButton::clicked, this, [&]() {
        ContextView* ctxview = this->context_view();
        if(ctxview) ctxview->toggle_pause();
    });
}

MainWindow::~MainWindow() {
    delete this->context_view(); // delete immediately
    rd_deinit();
    rd_set_log_callback(nullptr, nullptr);
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
                             auto* m = static_cast<ProblemsModel*>(
                                 dlg->source_model());
                             cv->surface()->jump_to(m->address(index));
                             dlg->accept();
                         }
                     });

    dlg->set_model(new ProblemsModel(ctxview->context(), dlg));
    dlg->show();
}

void MainWindow::show_context_view(RDContext* ctx) {
    REDasmSettings settings;
    settings.update_recent_files(m_filepath);
    this->load_recents(); // reload

    QDir::setCurrent(QString::fromUtf8(rd_get_working_dir(ctx)));
    this->setWindowTitle(QString::fromUtf8(rd_get_file_name(ctx)));

    auto* cv = new ContextView(ctx);
    this->replace_view(cv);
    cv->schedule_step();

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
    m_ui.mnuexport->menuAction()->setVisible(e);

    m_ui.actfilesave->setVisible(e);
    m_ui.actfilesaveas->setVisible(e);
    m_ui.actfileclose->setVisible(e);
    m_ui.actedit->setVisible(e);
    m_ui.actview->setVisible(e);

    m_ui.acttbseparator1->setVisible(e);
    m_ui.acttbseparator2->setVisible(e);
    m_ui.acttbseparator3->setVisible(e);
    m_ui.acttbseparator4->setVisible(e);

    m_ui.acttoolsflc->setVisible(e);
    m_ui.acttoolsproblems->setVisible(e);
    m_ui.actdevgraphs->setVisible(e);
    m_ui.actviewsegmentregs->setVisible(e);
    m_ui.actviewsegments->setVisible(e);
    m_ui.actviewmappings->setVisible(e);
    m_ui.actviewstrings->setVisible(e);
    m_ui.actviewtypedefs->setVisible(e);
    m_ui.actviewimported->setVisible(e);
    m_ui.actviewexported->setVisible(e);

    actions::get(actions::GOTO)->setVisible(e);

    if(!e) {
        statusbar::set_status_text(QString{});
        statusbar::problems_button()->hide();
        statusbar::status_button()->hide();
        m_ui.logview->clear();
    }
}

void MainWindow::open_project(const QString& filepath) {
    if(filepath.isEmpty()) return;

    m_filepath = filepath;
    QByteArray workingdir;

    while(true) {
        RDAcceptResult res = rd_project_load(
            qUtf8Printable(m_filepath),
            workingdir.isEmpty() ? nullptr : workingdir.constData());

        switch(res.status) {
            case RD_ACCEPT_OK: this->show_context_view(res.context); return;

            case RD_ACCEPT_FAIL:
                QMessageBox::warning(this, "Project",
                                     "Project loading aborted");
                return;

            case RD_ACCEPT_FAIL_WRITE: {
                QString newpath = QFileDialog::getExistingDirectory(
                    this,
                    tr("destination is not writable, select another "
                       "directory…"),
                    QString{},
                    QFileDialog::ShowDirsOnly |
                        QFileDialog::DontUseNativeDialog);

                if(newpath.isEmpty()) {
                    rd_reject();
                    return;
                }

                workingdir = newpath.toUtf8();
                break;
            }
        }
    }
}

void MainWindow::open_file(const QString& filepath) {
    if(filepath.isEmpty()) return;

    if(QFileInfo{filepath}.suffix() == "rdx") {
        this->open_project(filepath);
        return;
    }

    m_filepath = filepath;
    RDTestResultSlice ctxslice = rd_test(qUtf8Printable(m_filepath));
    if(rd_slice_is_empty(ctxslice)) return;

    auto* dlgloader = new LoaderDialog(ctxslice, this);

    connect(dlgloader, &LoaderDialog::accepted, this, [&, dlgloader]() {
        QByteArray workingdir;

        while(true) {
            RDAcceptResult res =
                rd_accept(dlgloader->sel_test, &dlgloader->accept_params);

            switch(res.status) {
                case RD_ACCEPT_OK:
                    this->select_analyzers(res.context);
                    dlgloader->deleteLater();
                    return;

                case RD_ACCEPT_FAIL:
                    QMessageBox::warning(this, "Loader", "Loading aborted");
                    dlgloader->deleteLater();
                    return;

                case RD_ACCEPT_FAIL_WRITE: {
                    QString newpath = QFileDialog::getExistingDirectory(
                        this,
                        tr("destination is not writable, select another "
                           "directory…"),
                        QString{},
                        QFileDialog::ShowDirsOnly |
                            QFileDialog::DontUseNativeDialog);

                    if(newpath.isEmpty()) {
                        rd_reject();
                        dlgloader->deleteLater();
                        return;
                    }

                    workingdir = newpath.toUtf8();
                    dlgloader->accept_params.working_dir =
                        workingdir.constData();
                    break;
                }
            }
        }
    });

    connect(dlgloader, &LoaderDialog::rejected, this, [dlgloader]() {
        rd_reject();
        dlgloader->deleteLater();
    });

    dlgloader->open();
}

void MainWindow::select_file() {
    QString s = QFileDialog::getOpenFileName(this, "Disassemble file…");
    if(!s.isEmpty()) this->open_file(s);
}

void MainWindow::save_project() {
    ContextView* cv = this->context_view();
    if(rd_project_save(cv->context(), nullptr)) return;

    QMessageBox::warning(this, "Project ",
                         "Failed to save project\nSee log for details.");
}

void MainWindow::save_project_as() {
    QString s = QFileDialog::getSaveFileName(this, "REDasm Project…", {},
                                             "Database File (*.rdx)");
    if(s.isEmpty()) return;

    if(!s.endsWith(".rdx")) s.append(".rdx");

    ContextView* cv = this->context_view();
    if(rd_project_save(cv->context(), qUtf8Printable(s))) return;

    QMessageBox::warning(
        this, "Project ",
        QString{"Failed to save project to '%1'\nSee log for details."}.arg(s));
}

void MainWindow::export_db() {
    QString s = QFileDialog::getSaveFileName(this, "Export database…", {},
                                             "Database File (*.db)");
    if(s.isEmpty()) return;

    if(!s.endsWith(".db")) s.append(".db");

    ContextView* cv = this->context_view();
    if(rd_export(cv->context(), qUtf8Printable(s), RD_EXPORT_DB)) return;

    QMessageBox::warning(
        this, "DB export",
        QString{"Database export to '%1' failed\nSee log for details."}.arg(s));
}

void MainWindow::export_input() {
    QString s = QFileDialog::getSaveFileName(this, "Export input…", {},
                                             "Any File (*.*)");
    if(s.isEmpty()) return;

    ContextView* cv = this->context_view();
    if(rd_export(cv->context(), qUtf8Printable(s), RD_EXPORT_INPUT)) return;

    QMessageBox::warning(
        this, "Input export",
        QString{"Input export to '%1' failed\nSee log for details."}.arg(s));
}

void MainWindow::export_input_patch() {
    QString s = QFileDialog::getSaveFileName(this, "Export patched input…", {},
                                             "Any File (*.*)");
    if(s.isEmpty()) return;

    ContextView* cv = this->context_view();
    if(rd_export(cv->context(), qUtf8Printable(s), RD_EXPORT_INPUT_PATCH))
        return;

    QMessageBox::warning(
        this, "Patched input export",
        QString{"Patched input export to '%1' failed\nSee log for details."}
            .arg(s));
}

void MainWindow::log(RDLogLevel level, const QString& tag, // NOLINT
                     const QString& msg) {
    m_ui.logview->log(level, tag, msg);
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
                auto* m = static_cast<SegmentsModel*>(dlg->source_model());
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
                auto* m = static_cast<MappingsModel*>(dlg->source_model());
                ctxview->surface()->jump_to(m->address(index));
                dlg->accept();
            });

    dlg->set_model(new MappingsModel(ctxview->context(), dlg));
    dlg->show();
}

void MainWindow::show_segment_regs() { // NOLINT
    ContextView* ctxview = this->context_view();
    if(ctxview) ctxview->show_segment_regs();
}

void MainWindow::show_strings() {
    ContextView* ctxview = this->context_view();
    if(!ctxview) return;

    auto* dlg = new TableDialog("Strings", this);

    connect(dlg, &TableDialog::double_clicked, this,
            [&, dlg](const QModelIndex& index) {
                ContextView* cv = this->context_view();
                if(!cv) return;

                auto* m = static_cast<StringsModel*>(dlg->model());
                cv->surface()->jump_to(m->address(index));
                dlg->accept();
            });

    auto* stringsmodel = new StringsModel(ctxview->context(), dlg);

    dlg->set_model(stringsmodel);
    dlg->set_stretch_last_column(false);
    dlg->resize_column(3, QHeaderView::Stretch);
    dlg->show();
}

void MainWindow::show_typedefs() {
    ContextView* ctxview = this->context_view();
    if(!ctxview) return;

    auto* dlg = new TypeDefsDialog(ctxview->context(), this);
    dlg->show();
}

void MainWindow::show_exported() {
    ContextView* ctxview = this->context_view();
    if(!ctxview) return;

    auto* dlg = new TableDialog("Exported", this);

    connect(dlg, &TableDialog::double_clicked, this,
            [&, dlg](const QModelIndex& index) {
                ContextView* cv = this->context_view();
                if(!cv) return;

                auto* m = static_cast<ExternalsModel*>(dlg->source_model());
                cv->surface()->jump_to(m->address(index));
                dlg->accept();
            });

    dlg->set_model(
        new ExternalsModel(ctxview->context(), RD_EXT_EXPORTED, dlg));
    dlg->set_stretch_last_column(false);
    dlg->resize_column(2, QHeaderView::Stretch);
    dlg->show();
}

void MainWindow::show_imported() {
    ContextView* ctxview = this->context_view();
    if(!ctxview) return;

    auto* dlg = new TableDialog("Imported", this);

    connect(dlg, &TableDialog::double_clicked, this,
            [&, dlg](const QModelIndex& index) {
                ContextView* cv = this->context_view();
                if(!cv) return;

                auto* m = static_cast<ExternalsModel*>(dlg->source_model());
                cv->surface()->jump_to(m->address(index));
                dlg->accept();
            });

    dlg->set_model(
        new ExternalsModel(ctxview->context(), RD_EXT_IMPORTED, dlg));
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
