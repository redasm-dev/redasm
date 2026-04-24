#pragma once

#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableView>
#include <QVBoxLayout>

namespace ui {

struct AnalyzerDialog {
    QTableView* tblanalyzers;
    QPushButton *pbselectall, *pbunselectall, *pbrestoredefaults;
    QCheckBox* chkshowdetails;

    explicit AnalyzerDialog(QDialog* self) {
        self->setAttribute(Qt::WA_DeleteOnClose);
        self->setWindowTitle("Select Analyzers");
        self->setFixedSize(640, 430);
        self->setModal(true);

        this->tblanalyzers = new QTableView();
        this->tblanalyzers->setEditTriggers(QAbstractItemView::NoEditTriggers);
        this->tblanalyzers->setSelectionMode(
            QAbstractItemView::SingleSelection);
        this->tblanalyzers->setSelectionBehavior(QAbstractItemView::SelectRows);
        this->tblanalyzers->setShowGrid(false);
        this->tblanalyzers->setWordWrap(false);
        this->tblanalyzers->verticalHeader()->setVisible(false);

        this->pbselectall = new QPushButton("Select All");
        this->pbunselectall = new QPushButton("Unselect All");
        this->pbrestoredefaults = new QPushButton("Restore Defaults");
        this->chkshowdetails = new QCheckBox("Show Details");

        auto* hbox = new QHBoxLayout();
        hbox->addWidget(this->pbselectall);
        hbox->addWidget(this->pbunselectall);
        hbox->addWidget(this->pbrestoredefaults);
        hbox->addStretch();
        hbox->addWidget(this->chkshowdetails);

        auto* vbox = new QVBoxLayout(self);
        vbox->addWidget(new QLabel("Analyzers (sorted by execution order)"));
        vbox->addWidget(this->tblanalyzers);
        vbox->addLayout(hbox);

        auto* buttonbox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                               QDialogButtonBox::Cancel);
        QObject::connect(buttonbox, &QDialogButtonBox::accepted, self,
                         &QDialog::accept);
        QObject::connect(buttonbox, &QDialogButtonBox::rejected, self,
                         &QDialog::reject);

        vbox->addWidget(buttonbox);
    }
};

} // namespace ui
