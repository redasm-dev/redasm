#pragma once

#include <QBoxLayout>
#include <QComboBox>
#include <QDialog>
#include <QHexView/qhexview.h>
#include <QVBoxLayout>

namespace ui {

struct MemoryMapDialog {
    QComboBox* cbsegments;
    QHexView* hexview;

    explicit MemoryMapDialog(QDialog* self) {
        self->setAttribute(Qt::WA_DeleteOnClose);
        self->setWindowTitle("Memory Map");
        self->setModal(true);
        self->resize(800, 600);

        this->cbsegments = new QComboBox();

        this->hexview = new QHexView();
        this->hexview->setReadOnly(true);

        auto* vboxlayout = new QVBoxLayout(self);
        vboxlayout->addWidget(this->cbsegments);
        vboxlayout->addWidget(this->hexview);
    }
};

} // namespace ui
