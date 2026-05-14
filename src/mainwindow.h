#pragma once

#include "ui/mainwindow.h"
#include "views/context.h"
#include <redasm/redasm.h>

class MainWindow: public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    [[nodiscard]] ContextView* context_view() const;
    void log(RDLogLevel level, const QString& tag, const QString& msg);

protected:
    void dragEnterEvent(QDragEnterEvent* e) override;
    void dragMoveEvent(QDragMoveEvent* e) override;
    void dropEvent(QDropEvent* e) override;
    void closeEvent(QCloseEvent* e) override;

private Q_SLOTS:
    void select_file();
    void show_segments();
    void show_mappings();
    void show_segment_regs();
    void show_strings();
    void show_exported();
    void show_imported();
    void show_welcome_view();
    void show_problems();
    void open_file(const QString& filepath);

private:
    [[nodiscard]] bool can_close() const;
    void select_analyzers(RDContext* ctx);
    void show_context_view(RDContext* ctx);
    void enable_context_actions(bool e);
    void replace_view(QWidget* w);
    void update_menubar();
    void clear_recents();
    void load_window_state();
    void load_recents();

Q_SIGNALS:
    void closed();

private:
    ui::MainWindow m_ui;
    QString m_filepath;
};
