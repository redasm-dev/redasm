#pragma once

#include <QBoxLayout>
#include <QComboBox>
#include <QDialog>
#include <QTreeWidget>

namespace ui {

struct SegmentRegsDialog {
    QComboBox* cbregisters;
    QTreeView* tvtable;

    explicit SegmentRegsDialog(QDialog* self) {
        self->setAttribute(Qt::WA_DeleteOnClose);
        self->setWindowTitle("Segment Registers");
        self->resize(800, 600);

        this->cbregisters = new QComboBox();
        this->tvtable = new QTreeView();

        auto* vbox = new QVBoxLayout(self);
        vbox->addWidget(this->cbregisters);
        vbox->addWidget(this->tvtable);
    }
};

} // namespace ui
