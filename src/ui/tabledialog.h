#pragma once

#include "support/fontawesome.h"
#include "widgets/feedbacktoolbutton.h"
#include <QBoxLayout>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QTreeWidget>

namespace ui {

struct TableDialog {
    QDialogButtonBox* buttonbox;
    QLineEdit* lesearch;
    FeedbackToolButton* ftbcopy;
    QLabel* lbldescription;
    QTreeView* tvtable;

    explicit TableDialog(QDialog* self) {
        self->setAttribute(Qt::WA_DeleteOnClose);
        self->resize(800, 600);

        this->tvtable = new QTreeView();
        this->lbldescription = new QLabel();

        this->lesearch = new QLineEdit();
        this->lesearch->setPlaceholderText("Search…");
        this->ftbcopy = new FeedbackToolButton();
        this->ftbcopy->setIcon(FA_ICON(0xf0c5));

        auto* hbox = new QHBoxLayout();
        hbox->addWidget(this->lesearch, 1);
        hbox->addWidget(this->ftbcopy, 1);

        this->buttonbox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                               QDialogButtonBox::Cancel);
        QObject::connect(this->buttonbox, &QDialogButtonBox::accepted, self,
                         &QDialog::accept);
        QObject::connect(this->buttonbox, &QDialogButtonBox::rejected, self,
                         &QDialog::reject);

        auto* vbox = new QVBoxLayout(self);
        vbox->addLayout(hbox);
        vbox->addWidget(this->lbldescription);
        vbox->addWidget(this->tvtable);
        vbox->addWidget(this->buttonbox);
    }
};

} // namespace ui
