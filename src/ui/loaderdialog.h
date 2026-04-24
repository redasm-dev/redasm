#pragma once

#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QSpinBox>

namespace ui {

struct LoaderDialog {
    QListWidget* lwloaders;
    QGroupBox *gboptions, *gbloader, *gbaddressing;
    QComboBox* cbprocessors;
    QSpinBox* sbminstring;
    QLineEdit *leentrypoint, *leoffset, *lebaseaddress;

    explicit LoaderDialog(QDialog* self) {
        self->setAttribute(Qt::WA_DeleteOnClose);
        self->setWindowTitle("Loader");
        self->resize(540, 550);
        self->setModal(true);

        auto* vbox = new QVBoxLayout(self);
        this->setup_top(vbox);
        this->setup_bottom(vbox);

        auto* buttonbox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                               QDialogButtonBox::Cancel);

        QObject::connect(buttonbox, &QDialogButtonBox::accepted, self,
                         &QDialog::accept);
        QObject::connect(buttonbox, &QDialogButtonBox::rejected, self,
                         &QDialog::reject);

        vbox->addWidget(buttonbox);
    }

private:
    void setup_top(QVBoxLayout* l) {
        l->addWidget(new QLabel("Select a Loader from the list below"));

        this->lwloaders = new QListWidget();
        this->lwloaders->setEditTriggers(QAbstractItemView::NoEditTriggers);
        this->lwloaders->setUniformItemSizes(true);
        l->addWidget(this->lwloaders);
    }

    void setup_bottom(QVBoxLayout* l) {
        auto* vbox = new QVBoxLayout();
        this->setup_loader_part(vbox);
        this->setup_addressing_part(vbox);
        l->addLayout(vbox);
    }

    void setup_loader_part(QVBoxLayout* l) {
        this->gbloader = new QGroupBox();
        this->gbloader->setTitle("Loader");

        auto* grid = new QGridLayout(this->gbloader);
        grid->setColumnStretch(1, 1);
        grid->addWidget(new QLabel("Processor:"), 0, 0, 1, 1);
        this->cbprocessors = new QComboBox();
        grid->addWidget(this->cbprocessors, 0, 1, 1, 1);

        grid->addWidget(new QLabel("Min String:"), 1, 0, 1, 1);
        this->sbminstring = new QSpinBox();
        this->sbminstring->setMinimum(1);
        this->sbminstring->setMaximum(100);
        grid->addWidget(this->sbminstring, 1, 1, 1, 1);

        l->addWidget(this->gbloader);
    }

    void setup_addressing_part(QVBoxLayout* l) {
        this->gbaddressing = new QGroupBox();
        this->gbaddressing->setTitle("Addressing");

        auto* grid = new QGridLayout(this->gbaddressing);
        grid->setColumnStretch(1, 1);
        grid->addWidget(new QLabel("Entry Point (Relative):"), 0, 0, 1, 1);
        this->leentrypoint = new QLineEdit();
        grid->addWidget(this->leentrypoint, 0, 1, 1, 1);
        grid->addWidget(new QLabel("Offset:"), 1, 0, 1, 1);
        grid->addWidget(new QLineEdit(), 1, 1, 1, 1);

        this->lebaseaddress = new QLineEdit();
        grid->addWidget(this->lebaseaddress, 2, 1, 1, 1);
        grid->addWidget(new QLabel("Base Address:"), 2, 0, 1, 1);
        l->addWidget(this->gbaddressing);
    }
};

} // namespace ui
