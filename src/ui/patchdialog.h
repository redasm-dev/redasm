#pragma once

#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QVBoxLayout>

namespace ui {

struct PatchDialog {
    QLineEdit* leinstruction;
    QCheckBox* chbxfillnop;
    QDialogButtonBox* buttonbox;

    explicit PatchDialog(QDialog* self) {
        self->setAttribute(Qt::WA_DeleteOnClose);
        self->setModal(true);

        this->leinstruction = new QLineEdit();
        this->chbxfillnop = new QCheckBox("Fill with NOPs");
        this->buttonbox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                               QDialogButtonBox::Cancel);

        QObject::connect(this->buttonbox, &QDialogButtonBox::accepted, self,
                         &QDialog::accept);
        QObject::connect(this->buttonbox, &QDialogButtonBox::rejected, self,
                         &QDialog::reject);

        auto* vbox = new QVBoxLayout(self);
        vbox->addWidget(this->leinstruction);
        vbox->addWidget(this->chbxfillnop);
        vbox->addWidget(this->buttonbox);

        self->resize(300, self->minimumHeight());
    }
};

} // namespace ui
