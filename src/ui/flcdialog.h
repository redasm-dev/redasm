#pragma once

#include <QDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace ui {

struct FLCDialog {
    QLineEdit *leaddress, *leoffset, *lesegment;
    QPushButton *pbcopyaddress, *pbcopyoffset, *pbcopysegment;

    explicit FLCDialog(QDialog* self) {
        self->setAttribute(Qt::WA_DeleteOnClose);
        self->setWindowTitle("FLC");
        self->setFixedSize(300, 150);
        self->setModal(true);

        this->leaddress = new QLineEdit();
        this->leoffset = new QLineEdit();
        this->lesegment = new QLineEdit();
        this->lesegment->setReadOnly(true);

        auto* form = new QFormLayout();
        form->addRow("Address", this->leaddress);
        form->addRow("Offset", this->leoffset);
        form->addRow("Segment", this->lesegment);

        this->pbcopyaddress = new QPushButton("Copy Address");
        this->pbcopyaddress->setEnabled(false);
        this->pbcopyoffset = new QPushButton("Copy Offset");
        this->pbcopyoffset->setEnabled(false);
        this->pbcopysegment = new QPushButton("Copy Segment");
        this->pbcopysegment->setEnabled(false);

        auto* hbox = new QHBoxLayout();
        hbox->addWidget(this->pbcopyaddress);
        hbox->addWidget(this->pbcopyoffset);
        hbox->addWidget(this->pbcopysegment);

        auto* vbox = new QVBoxLayout(self);
        vbox->addLayout(form);
        vbox->addLayout(hbox);
    }
};

} // namespace ui
