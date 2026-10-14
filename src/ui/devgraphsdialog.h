#pragma once

#include "widgets/feedbackpushbutton.h"
#include <QButtonGroup>
#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QRadioButton>
#include <QSplitter>
#include <QTreeView>
#include <QVBoxLayout>

namespace ui {

struct DevGraphsDialog {
    FeedbackPushButton *ftbcopygraphhashes, *ftbcopyhash, *ftbcopyfunctions,
        *ftbcopygraph;
    QRadioButton *rbfuncgraph, *rbcallgraph;
    QTreeView* tvfunctions;
    QPlainTextEdit* ptedot;

    explicit DevGraphsDialog(QDialog* self) {
        self->setAttribute(Qt::WA_DeleteOnClose);
        self->setWindowTitle("Graph DOTs");
        self->setFixedSize(1000, 600);

        this->tvfunctions = new QTreeView();
        this->tvfunctions->setUniformRowHeights(true);
        this->tvfunctions->setRootIsDecorated(false);

        this->ftbcopygraphhashes = new FeedbackPushButton("Copy Graph Hashes");
        this->ftbcopygraphhashes->set_feedback_text("Copied");
        this->ftbcopyfunctions = new FeedbackPushButton("Copy Functions");
        this->ftbcopyfunctions->set_feedback_text("Copied");

        this->rbfuncgraph = new QRadioButton("Function");
        this->rbfuncgraph->setChecked(true);
        this->rbcallgraph = new QRadioButton("Call");

        auto* rbgroup = new QButtonGroup(self);
        rbgroup->addButton(this->rbfuncgraph);
        rbgroup->addButton(this->rbcallgraph);

        auto* hbox = new QHBoxLayout();
        hbox->addWidget(this->ftbcopygraphhashes);
        hbox->addWidget(this->ftbcopyfunctions);

        auto* vbox_left = new QVBoxLayout(new QWidget());
        vbox_left->addWidget(this->tvfunctions, 1);
        vbox_left->addLayout(hbox);

        this->ptedot = new QPlainTextEdit();
        this->ptedot->setLineWrapMode(QPlainTextEdit::NoWrap);
        this->ptedot->setUndoRedoEnabled(false);
        this->ptedot->setReadOnly(true);

        this->ftbcopyhash = new FeedbackPushButton("Copy Hash");
        this->ftbcopyhash->set_feedback_text("Copied");
        this->ftbcopygraph = new FeedbackPushButton("Copy Graph");
        this->ftbcopygraph->set_feedback_text("Copied");

        hbox = new QHBoxLayout();
        hbox->addWidget(new QLabel("Graph:"));
        hbox->addWidget(this->rbfuncgraph);
        hbox->addWidget(this->rbcallgraph);
        hbox->addStretch();
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
