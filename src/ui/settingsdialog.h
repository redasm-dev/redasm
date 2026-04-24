#pragma once

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFontComboBox>
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

        auto* hbox1 = new QHBoxLayout();
        hbox1->addWidget(new QLabel("Theme"));
        hbox1->addWidget(this->cbxthemes, 1);

        auto* hbox2 = new QHBoxLayout();
        hbox2->addWidget(new QLabel("Font"));
        hbox2->addWidget(this->fcbxfonts, 1);
        hbox2->addWidget(this->cbxfontsizes);
        hbox2->addWidget(this->pbfontdefault);

        auto* gb = new QGroupBox("Preview");
        gb->setFlat(false);

        auto* vbox2 = new QVBoxLayout(gb);
        vbox2->addWidget(this->lblpreview);

        auto* buttonbox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                               QDialogButtonBox::Cancel);

        auto* vbox1 = new QVBoxLayout(self);
        vbox1->addLayout(hbox1);
        vbox1->addLayout(hbox2);
        vbox1->addWidget(gb, 1);
        vbox1->addWidget(buttonbox);

        QObject::connect(buttonbox, &QDialogButtonBox::accepted, self,
                         &QDialog::accept);
        QObject::connect(buttonbox, &QDialogButtonBox::rejected, self,
                         &QDialog::reject);
    }
};

} // namespace ui
