#pragma once

#include <QDialog>
#include <QTextBrowser>
#include <QVBoxLayout>

namespace ui {

struct DetailDialog {
    QTextBrowser* tbdetail;

    explicit DetailDialog(QDialog* self) {
        self->setAttribute(Qt::WA_DeleteOnClose);
        self->resize(800, 600);
        self->setModal(true);

        this->tbdetail = new QTextBrowser();

        auto* vboxlayout = new QVBoxLayout(self);
        vboxlayout->addWidget(this->tbdetail);
    }
};

} // namespace ui
