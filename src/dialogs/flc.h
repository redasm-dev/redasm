#pragma once

#include "ui/flcdialog.h"
#include <QDialog>
#include <optional>
#include <redasm/redasm.h>

class FLCDialog: public QDialog {
    Q_OBJECT

public:
    explicit FLCDialog(RDContext* ctx, QWidget* parent = nullptr);

private Q_SLOTS:
    void on_address_edited(const QString& s);
    void on_rel_address_edited(const QString& s);
    void on_offset_edited(const QString& s);

private:
    void update_from(const QLineEdit* source,
                     const std::optional<RDAddress>& address);

private:
    ui::FLCDialog m_ui;
    RDContext* m_context{nullptr};
};
