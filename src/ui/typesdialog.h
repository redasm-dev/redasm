#pragma once

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QSpinBox>
#include <QVBoxLayout>

namespace ui {

struct TypesDialog {
    QComboBox *cbxtypes, *cbxmod;
    QSpinBox* sbcount;

    explicit TypesDialog(QDialog* self) {
        self->setAttribute(Qt::WA_DeleteOnClose);

        this->cbxtypes = new QComboBox();
        this->cbxmod = new QComboBox();

        this->sbcount = new QSpinBox();
        this->sbcount->setMinimum(0);

        auto* form = new QFormLayout();
        form->setLabelAlignment(Qt::AlignRight);
        form->addRow("Type", this->cbxtypes);
        form->addRow("Count (0=no array)", this->sbcount);
        form->addRow("Modifier", this->cbxmod);

        auto* buttonbox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                               QDialogButtonBox::Cancel);

        auto* vbox = new QVBoxLayout(self);
        vbox->addLayout(form, 1);
        vbox->addWidget(buttonbox);

        self->adjustSize();
        self->resize(400, self->height());

        QObject::connect(buttonbox, &QDialogButtonBox::accepted, self,
                         &QDialog::accept);
        QObject::connect(buttonbox, &QDialogButtonBox::rejected, self,
                         &QDialog::reject);
    }
};

} // namespace ui
