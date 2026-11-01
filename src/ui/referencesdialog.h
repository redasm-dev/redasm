#pragma once

#include <QDialog>
#include <QDialogButtonBox>
#include <QTreeView>
#include <QVBoxLayout>

namespace ui {

struct ReferencesFromDialog {
    QTreeView* elvrefs;
    QDialogButtonBox* buttonbox;

    explicit ReferencesFromDialog(QDialog* self) {
        self->setAttribute(Qt::WA_DeleteOnClose);
        self->setWindowTitle("References");
        self->resize(640, 330);
        self->setModal(true);

        this->elvrefs = new QTreeView();
        this->elvrefs->setUniformRowHeights(true);
        this->elvrefs->setRootIsDecorated(false);

        this->buttonbox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                               QDialogButtonBox::Cancel);
        QObject::connect(this->buttonbox, &QDialogButtonBox::accepted, self,
                         &QDialog::accept);
        QObject::connect(this->buttonbox, &QDialogButtonBox::rejected, self,
                         &QDialog::reject);

        auto* vbox = new QVBoxLayout(self);
        vbox->addWidget(this->elvrefs, 1);
        vbox->addWidget(this->buttonbox);
    }
};

} // namespace ui
