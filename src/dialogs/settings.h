#pragma once

#include "ui/settingsdialog.h"

class SettingsDialog: public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);

private:
    void select_size(int size);
    void select_current_theme();
    void select_current_font();
    void select_current_size();
    void update_preview() const;

public Q_SLOTS:
    void accept() override;

private Q_SLOTS:
    void select_default_font();

private:
    ui::SettingsDialog m_ui;
};
