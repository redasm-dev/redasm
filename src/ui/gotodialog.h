#pragma once

#include <QDialog>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLineEdit>
#include <QPushButton>
#include <QTreeView>
#include <QVBoxLayout>

namespace ui {

struct GotoDialog {
    QLineEdit* lesearch;
    QPushButton* pbgoto;
    QTreeView* tvsymbols;

    explicit GotoDialog(QDialog* self) {
        self->setAttribute(Qt::WA_DeleteOnClose);
        self->setWindowTitle("Goto");
        self->resize(640, 330);
        self->setModal(true);

        this->lesearch = new QLineEdit();
        this->pbgoto = new QPushButton("Goto");

        auto* hbox = new QHBoxLayout();
        hbox->addWidget(this->lesearch, 1);
        hbox->addWidget(this->pbgoto);

        this->lesearch->setPlaceholderText("Address or Symbol");

        this->tvsymbols = new QTreeView();
        this->tvsymbols->header()->setStretchLastSection(true);
        this->tvsymbols->setUniformRowHeights(true);
        this->tvsymbols->setRootIsDecorated(false);

        auto* vbox = new QVBoxLayout(self);
        vbox->addLayout(hbox);
        vbox->addWidget(this->tvsymbols, 1);

        auto* buttonbox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                               QDialogButtonBox::Cancel);
        QObject::connect(buttonbox, &QDialogButtonBox::accepted, self,
                         &QDialog::accept);
        QObject::connect(buttonbox, &QDialogButtonBox::rejected, self,
                         &QDialog::reject);

        vbox->addWidget(buttonbox);
    }
};

} // namespace ui
