#pragma once

#include "widgets/feedbackpushbutton.h"
#include <QDialog>
#include <QHBoxLayout>
#include <QPlainTextEdit>
#include <QSplitter>
#include <QTreeView>
#include <QVBoxLayout>

namespace ui {

struct DevGraphsDialog {
    FeedbackPushButton *ftbcopygraphhashes, *ftbcopyhash, *ftbcopyfunctions,
        *ftbcopygraph;
    QTreeView* tvfunctions;
    QPlainTextEdit* ptedot;

    explicit DevGraphsDialog(QDialog* self) {
        self->setAttribute(Qt::WA_DeleteOnClose);
        self->setWindowTitle("Graph DOTs");
        self->setFixedSize(1000, 600);

        this->tvfunctions = new QTreeView();
        this->tvfunctions->setUniformRowHeights(true);
        this->tvfunctions->setRootIsDecorated(false);

        this->ftbcopygraphhashes = new FeedbackPushButton();
        this->ftbcopygraphhashes->setText("Copy Graph Hashes");
        this->ftbcopygraphhashes->set_feedback_text("Copied");
        this->ftbcopyfunctions = new FeedbackPushButton();
        this->ftbcopyfunctions->setText("Copy Functions");
        this->ftbcopyfunctions->set_feedback_text("Copied");

        auto* hbox = new QHBoxLayout();
        hbox->addWidget(this->ftbcopygraphhashes);
        hbox->addWidget(this->ftbcopyfunctions);

        auto* vbox_left = new QVBoxLayout(new QWidget());
        vbox_left->addWidget(this->tvfunctions, 1);
        vbox_left->addLayout(hbox);

        this->ptedot = new QPlainTextEdit();
        this->ptedot->setUndoRedoEnabled(false);
        this->ptedot->setReadOnly(true);

        this->ftbcopyhash = new FeedbackPushButton();
        this->ftbcopyhash->setText("Copy Hash");
        this->ftbcopyhash->set_feedback_text("Copied");
        this->ftbcopygraph = new FeedbackPushButton();
        this->ftbcopygraph->setText("Copy Graph");
        this->ftbcopygraph->set_feedback_text("Copied");

        hbox = new QHBoxLayout();
        hbox->addWidget(this->ftbcopyhash);
        hbox->addWidget(this->ftbcopygraph);

        auto* vbox_right = new QVBoxLayout(new QWidget());
        vbox_right->addWidget(this->ptedot, 1);
        vbox_right->addLayout(hbox);

        auto* splitter = new QSplitter(Qt::Horizontal);
        splitter->addWidget(vbox_left->parentWidget());
        splitter->addWidget(vbox_right->parentWidget());
        splitter->setStretchFactor(1, 1);

        auto* vbox = new QVBoxLayout(self);
        vbox->setContentsMargins(0, 0, 0, 0);
        vbox->addWidget(splitter);
    }
};

} // namespace ui
