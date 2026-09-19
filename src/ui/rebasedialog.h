#pragma once

#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>

namespace ui {

struct RebaseDialog {
    QLabel* lblbaseaddress;
    QLineEdit* lebaseaddress;
    QCheckBox* chkrelative;
    QDialogButtonBox* buttonbox;

    explicit RebaseDialog(QDialog* self) {
        self->setAttribute(Qt::WA_DeleteOnClose);
        self->setModal(true);

        this->lblbaseaddress = new QLabel();
        this->lebaseaddress = new QLineEdit();
        this->chkrelative = new QCheckBox("Relative");

        this->buttonbox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                               QDialogButtonBox::Cancel);

        QObject::connect(this->buttonbox, &QDialogButtonBox::accepted, self,
                         &QDialog::accept);
        QObject::connect(this->buttonbox, &QDialogButtonBox::rejected, self,
                         &QDialog::reject);

        auto* vbox = new QVBoxLayout(self);
        vbox->addWidget(this->lblbaseaddress);
        vbox->addWidget(this->lebaseaddress);
        vbox->addWidget(this->chkrelative);
        vbox->addWidget(this->buttonbox);

        self->adjustSize();
    }
};

} // namespace ui
