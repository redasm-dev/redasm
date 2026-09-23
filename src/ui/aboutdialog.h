#pragma once

#include <QDialog>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTabWidget>
#include <QTextBrowser>
#include <QVBoxLayout>

namespace ui {

struct AboutDialog {
    QDialogButtonBox* buttonbox;
    QLabel* lblaboutlogo;
    QTextBrowser *txbconfig, *txbmodules, *txbloaders, *txbprocessors,
        *txbanalyzers, *txbcommands;
    QTabWidget* tabwidget;
    QPushButton* pbsettings;

    explicit AboutDialog(QDialog* self) {
        self->resize(800, 600);
        self->setAttribute(Qt::WA_DeleteOnClose);

        this->lblaboutlogo = new QLabel();

        this->tabwidget = new QTabWidget(self);
        this->txbconfig = this->add_tab("Config");
        this->txbmodules = this->add_tab("Modules");
        this->txbloaders = this->add_tab("Loaders");
        this->txbprocessors = this->add_tab("Processors");
        this->txbanalyzers = this->add_tab("Analyzers");
        this->txbcommands = this->add_tab("Commands");

        this->pbsettings = new QPushButton("Edit Settings");
        this->buttonbox = new QDialogButtonBox(QDialogButtonBox::Close);

        auto* hbox = new QHBoxLayout();
        hbox->addWidget(this->pbsettings);
        hbox->addStretch();
        hbox->addWidget(this->buttonbox);

        auto* vbox = new QVBoxLayout(self);
        vbox->addWidget(this->lblaboutlogo);
        vbox->addWidget(this->tabwidget, 1);
        vbox->addLayout(hbox);

        QObject::connect(this->buttonbox, &QDialogButtonBox::rejected, self,
                         &QDialog::reject);
    }

private:
    [[nodiscard]] QTextBrowser* add_tab(const QString& title) const {
        auto* txb = new QTextBrowser();
        this->tabwidget->addTab(txb, title);
        return txb;
    }
};

} // namespace ui
