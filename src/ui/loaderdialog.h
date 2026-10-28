#pragma once

#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QTreeView>

namespace ui {

struct LoaderDialog {
    QListWidget* lwloaders;
    QGroupBox *gbloader, *gbopenmode, *gbaddressing;
    QComboBox* cbprocessors;
    QSpinBox* sbminstring;
    QRadioButton *rbnewanalysis, *rbopenproject;
    QLineEdit *lebaseaddress, *leentrypoint, *leoffset, *leaddress;
    QTabWidget* tabs;
    QTreeView* tvoptions;
    QDialogButtonBox* buttonbox;
    QPushButton* pbresetoptions;

    explicit LoaderDialog(QDialog* self) {
        self->setWindowTitle("Loader");
        self->resize(540, 550);
        self->setModal(true);

        auto* vbox = new QVBoxLayout(self);
        this->setup_top(vbox);
        this->setup_bottom(vbox);

        this->pbresetoptions = new QPushButton();
        this->pbresetoptions->setText("Reset options");

        this->buttonbox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                               QDialogButtonBox::Cancel);

        QObject::connect(this->buttonbox, &QDialogButtonBox::accepted, self,
                         &QDialog::accept);
        QObject::connect(this->buttonbox, &QDialogButtonBox::rejected, self,
                         &QDialog::reject);

        auto* hbox = new QHBoxLayout();
        hbox->addWidget(this->pbresetoptions);
        hbox->addWidget(this->buttonbox, 1);

        vbox->addLayout(hbox);
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
        this->tabs = new QTabWidget();

        auto* vbox = new QVBoxLayout(new QWidget());
        this->setup_loader_part(vbox);
        this->setup_addressing_part(vbox);

        this->tvoptions = new QTreeView();
        this->tvoptions->setFrameShape(QFrame::NoFrame);
        this->tvoptions->setUniformRowHeights(true);
        this->tvoptions->setRootIsDecorated(false);
        this->tvoptions->setWordWrap(false);

        this->tabs->addTab(vbox->parentWidget(), "General");
        this->tabs->addTab(this->tvoptions, "Options");

        l->addWidget(this->tabs);
    }

    void setup_loader_part(QVBoxLayout* l) {
        auto* hbox = new QHBoxLayout();

        // left part
        this->gbloader = new QGroupBox();
        this->gbloader->setTitle("Loader");

        this->cbprocessors = new QComboBox();

        this->sbminstring = new QSpinBox();
        this->sbminstring->setMinimum(1);
        this->sbminstring->setMaximum(100);

        auto* form = new QFormLayout(this->gbloader);
        form->setLabelAlignment(Qt::AlignRight);
        form->addRow("Processor:", this->cbprocessors);
        form->addRow("Min String:", this->sbminstring);

        // right part
        this->gbopenmode = new QGroupBox();
        this->gbopenmode->setTitle("Open Mode");

        this->rbnewanalysis = new QRadioButton();
        this->rbnewanalysis->setText("New analysis");
        this->rbnewanalysis->setChecked(true);

        this->rbopenproject = new QRadioButton();
        this->rbopenproject->setText("Load project");

        auto* vbox = new QVBoxLayout(this->gbopenmode);
        vbox->addWidget(this->rbnewanalysis);
        vbox->addWidget(this->rbopenproject);
        vbox->addStretch();

        hbox->addWidget(this->gbloader, 1);
        hbox->addWidget(this->gbopenmode);

        l->addLayout(hbox);
    }

    void setup_addressing_part(QVBoxLayout* l) {
        this->gbaddressing = new QGroupBox();
        this->gbaddressing->setTitle("Addressing");

        this->lebaseaddress = new QLineEdit();
        this->leentrypoint = new QLineEdit();
        this->leaddress = new QLineEdit();
        this->leoffset = new QLineEdit();

        auto* form = new QFormLayout(this->gbaddressing);
        form->setLabelAlignment(Qt::AlignRight);
        form->addRow("Base Address:", this->lebaseaddress);
        form->addRow("Address:", this->leaddress);
        form->addRow("Offset:", this->leoffset);
        form->addRow("Entry Point:", this->leentrypoint);
        l->addWidget(this->gbaddressing);
    }
};

} // namespace ui
