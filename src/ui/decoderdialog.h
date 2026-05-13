#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHexView/qhexview.h>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSplitter>
#include <QVBoxLayout>

namespace ui {

struct DecoderDialog {
    QHexView* hexview;
    QComboBox* cbprocessors;
    QLineEdit* leaddress;
    QCheckBox* cbxdetails;
    QPlainTextEdit* ptedecoded;
    QDialogButtonBox* buttonbox;
    QPushButton* pbclear;

    explicit DecoderDialog(QDialog* self) {
        self->setAttribute(Qt::WA_DeleteOnClose);
        self->setWindowTitle("Instructions Decoder");
        self->resize(700, 500);
        self->setModal(true);

        this->cbprocessors = new QComboBox();
        this->leaddress = new QLineEdit();
        this->pbclear = new QPushButton("Clear");
        this->cbxdetails = new QCheckBox("Show details");

        auto* grid = new QGridLayout();
        grid->setColumnStretch(1, 1);
        grid->addWidget(new QLabel("Processors:"), 0, 0, Qt::AlignRight);
        grid->addWidget(this->cbprocessors, 0, 1, 1, 2);
        grid->addWidget(this->pbclear, 0, 3);
        grid->addWidget(new QLabel("Base Address:"), 1, 0, Qt::AlignRight);
        grid->addWidget(this->leaddress, 1, 1, 1, 3);

        this->hexview = new QHexView();
        this->hexview->setFrameStyle(QFrame::NoFrame);

        this->ptedecoded = new QPlainTextEdit();
        this->ptedecoded->setFrameStyle(QFrame::NoFrame);
        this->ptedecoded->setWordWrapMode(QTextOption::NoWrap);
        this->ptedecoded->setUndoRedoEnabled(false);
        this->ptedecoded->setReadOnly(true);

        auto* splitter = new QSplitter(Qt::Vertical);
        splitter->addWidget(this->group_widget(this->hexview, "Input Data"));
        splitter->addWidget(
            this->group_widget(this->ptedecoded, "Decoded Instructions"));

        this->buttonbox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                               QDialogButtonBox::Close);
        this->buttonbox->button(QDialogButtonBox::Ok)->setText("Decode");

        QObject::connect(this->buttonbox, &QDialogButtonBox::rejected, self,
                         &QDialog::reject);

        auto* hbox = new QHBoxLayout();
        hbox->addWidget(this->cbxdetails);
        hbox->addStretch();
        hbox->addWidget(buttonbox);

        auto* vbox = new QVBoxLayout(self);
        vbox->addLayout(grid);
        vbox->addWidget(splitter);
        vbox->addLayout(hbox);
    }

private:
    QGroupBox* group_widget(QWidget* w, const QString& title) {
        auto* l = new QVBoxLayout();
        l->setContentsMargins(0, 0, 0, 0);
        l->addWidget(w);

        auto* gb = new QGroupBox(title);
        gb->setLayout(l);
        return gb;
    }
};

} // namespace ui
