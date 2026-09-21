#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFontComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace ui {

struct SettingsDialog {
    QComboBox *cbxthemes, *cbxfontsizes;
    QFontComboBox* fcbxfonts;
    QPushButton* pbfontdefault;
    QCheckBox* chknetwork;
    QLabel* lblpreview;

    explicit SettingsDialog(QDialog* self) {
        self->resize(450, 300);
        self->setAttribute(Qt::WA_DeleteOnClose);

        this->fcbxfonts = new QFontComboBox();
        this->cbxfontsizes = new QComboBox();
        this->cbxthemes = new QComboBox();
        this->pbfontdefault = new QPushButton("Default");

        this->lblpreview = new QLabel("Lorem ipsum dolor sit amet");
        this->lblpreview->setAlignment(Qt::AlignCenter);

        this->chknetwork = new QCheckBox("Enabled");

        auto* hbox_font = new QHBoxLayout();
        hbox_font->addWidget(this->fcbxfonts, 1);
        hbox_font->addWidget(this->cbxfontsizes);
        hbox_font->addWidget(this->pbfontdefault);

        auto* form = new QFormLayout();
        form->setLabelAlignment(Qt::AlignRight);
        form->addRow("Network:", this->chknetwork);
        form->addRow("Theme:", this->cbxthemes);
        form->addRow("Font:", hbox_font);

        auto* gb = new QGroupBox("Preview");
        gb->setFlat(false);

        auto* vbox2 = new QVBoxLayout(gb);
        vbox2->addWidget(this->lblpreview);

        auto* buttonbox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                               QDialogButtonBox::Cancel);

        auto* vbox1 = new QVBoxLayout(self);
        vbox1->addLayout(form);
        vbox1->addWidget(gb, 1);
        vbox1->addWidget(buttonbox);

        QObject::connect(buttonbox, &QDialogButtonBox::accepted, self,
                         &QDialog::accept);
        QObject::connect(buttonbox, &QDialogButtonBox::rejected, self,
                         &QDialog::reject);
    }
};

} // namespace ui
