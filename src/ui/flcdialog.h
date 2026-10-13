#pragma once

#include "widgets/feedbackpushbutton.h"
#include <QDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace ui {

struct FLCDialog {
    QLineEdit *leaddress, *lereladdress, *leoffset, *lesegment;
    FeedbackPushButton *pbcopyaddress, *pbcopyreladdress, *pbcopyoffset,
        *pbcopysegment;

    explicit FLCDialog(QDialog* self) {
        self->setAttribute(Qt::WA_DeleteOnClose);
        self->setWindowTitle("FLC");
        self->setModal(true);

        this->leaddress = new QLineEdit();
        this->lereladdress = new QLineEdit();
        this->leoffset = new QLineEdit();
        this->lesegment = new QLineEdit();
        this->lesegment->setReadOnly(true);

        auto* form = new QFormLayout();
        form->addRow("Address", this->leaddress);
        form->addRow("Address (Rel)", this->lereladdress);
        form->addRow("Offset", this->leoffset);
        form->addRow("Segment", this->lesegment);

        this->pbcopyaddress = new FeedbackPushButton("Copy Address");
        this->pbcopyaddress->setEnabled(false);
        this->pbcopyaddress->set_feedback_text("Copied");
        this->pbcopyreladdress = new FeedbackPushButton("Copy Address (Rel)");
        this->pbcopyreladdress->setEnabled(false);
        this->pbcopyreladdress->set_feedback_text("Copied");
        this->pbcopyoffset = new FeedbackPushButton("Copy Offset");
        this->pbcopyoffset->setEnabled(false);
        this->pbcopyoffset->set_feedback_text("Copied");
        this->pbcopysegment = new FeedbackPushButton("Copy Segment");
        this->pbcopysegment->setEnabled(false);
        this->pbcopysegment->set_feedback_text("Copied");

        auto* hbox = new QHBoxLayout();
        hbox->addWidget(this->pbcopyaddress);
        hbox->addWidget(this->pbcopyreladdress);
        hbox->addWidget(this->pbcopyoffset);
        hbox->addWidget(this->pbcopysegment);

        auto* vbox = new QVBoxLayout(self);
        vbox->addLayout(form);
        vbox->addLayout(hbox);

        self->adjustSize();
    }
};

} // namespace ui
