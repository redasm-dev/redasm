#pragma once

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QSpinBox>
#include <QVBoxLayout>

namespace ui {

struct BranchDialog {
    QComboBox *cbxreftype, *cbxelemtype;
    QSpinBox* sbcount;

    explicit BranchDialog(QDialog* self) {
        self->setAttribute(Qt::WA_DeleteOnClose);

        this->cbxreftype = new QComboBox();
        this->cbxelemtype = new QComboBox();

        this->sbcount = new QSpinBox();

        auto* form = new QFormLayout();
        form->setLabelAlignment(Qt::AlignRight);
        form->addRow("Element Type", this->cbxelemtype);
        form->addRow("Count ", this->sbcount);
        form->addRow("Ref. Type", this->cbxreftype);

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
