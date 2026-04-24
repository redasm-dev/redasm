#pragma once

#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QWidget>

namespace ui {

struct LogView {
    QPlainTextEdit* ptelogs;

    explicit LogView(QWidget* self) {
        this->ptelogs = new QPlainTextEdit(self);
        this->ptelogs->setWordWrapMode(QTextOption::NoWrap);
        this->ptelogs->setUndoRedoEnabled(false);
        this->ptelogs->setReadOnly(true);

        auto* vbox = new QVBoxLayout(self);
        vbox->addWidget(this->ptelogs);
        vbox->setContentsMargins(0, 0, 0, 0);
    }
};

} // namespace ui
