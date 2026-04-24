#pragma once

#include <QDialog>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextBrowser>
#include <QVBoxLayout>

namespace ui {

struct AboutDialog {
    QDialogButtonBox* buttonbox;
    QLabel *lbllogo, *lbltitle;
    QTextBrowser* txbabout;

    explicit AboutDialog(QDialog* self) {
        self->resize(600, 500);
        self->setAttribute(Qt::WA_DeleteOnClose);

        this->lbllogo = new QLabel();
        this->lbltitle = new QLabel();
        this->txbabout = new QTextBrowser();

        this->buttonbox = new QDialogButtonBox(QDialogButtonBox::Close);
        QObject::connect(this->buttonbox, &QDialogButtonBox::rejected, self,
                         &QDialog::reject);

        auto* hbox = new QHBoxLayout();
        hbox->addWidget(this->lbllogo);
        hbox->addWidget(this->lbltitle, 1);

        auto* vbox = new QVBoxLayout(self);
        vbox->addLayout(hbox);
        vbox->addWidget(this->txbabout, 1);
        vbox->addWidget(this->buttonbox);
    }
};

} // namespace ui
