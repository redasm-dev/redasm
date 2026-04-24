#pragma once

#include <QApplication>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListView>
#include <QPushButton>
#include <QSpacerItem>
#include <QVBoxLayout>
#include <QWidget>

namespace ui {

struct WelcomeView {
    QListView* lvrecents;
    QPushButton* pbopen;
    QPushButton* pbsettings;
    QPushButton* pbabout;
    QPushButton* pbopenreddit;
    QPushButton* pbopenhome;
    QPushButton* pbopengithub;
    QPushButton* pbopentelegram;
    QPushButton* pbopenx;
    QLabel* lblrecents;
    QLabel* lblbrand;
    QLabel* lblversion;

    explicit WelcomeView(QWidget* self) {
        self->setWindowTitle("Welcome");

        QFont font1, font2;
        font1.setPointSize(60);
        font1.setBold(true);
        font2.setPointSize(16);
        font2.setBold(true);

        this->lblbrand = new QLabel(self);
        this->lblbrand->setFont(font1);
        this->lblbrand->setTextFormat(Qt::AutoText);
        this->lblbrand->setScaledContents(false);
        this->lblbrand->setAlignment(Qt::AlignCenter);

        this->pbopen = new QPushButton(self);

        QSizePolicy sizepolicy{QSizePolicy::Minimum, QSizePolicy::Fixed};
        sizepolicy.setHorizontalStretch(0);
        sizepolicy.setVerticalStretch(0);
        sizepolicy.setHeightForWidth(
            this->pbopen->sizePolicy().hasHeightForWidth());
        this->pbopen->setSizePolicy(sizepolicy);
        this->pbopen->setMinimumSize(QSize{0, 40});
        this->pbopen->setText("Open");

        this->pbsettings = new QPushButton(self);
        sizepolicy.setHeightForWidth(
            this->pbsettings->sizePolicy().hasHeightForWidth());
        this->pbsettings->setSizePolicy(sizepolicy);
        this->pbsettings->setMinimumSize(QSize{0, 40});

        this->pbabout = new QPushButton(self);
        sizepolicy.setHeightForWidth(
            this->pbabout->sizePolicy().hasHeightForWidth());
        this->pbabout->setSizePolicy(sizepolicy);
        this->pbabout->setMinimumSize(QSize{0, 40});

        auto* vbox4 = new QVBoxLayout(self);
        vbox4->setContentsMargins(16, 16, 16, 16);

        auto* hbox2 = new QHBoxLayout();
        hbox2->setSpacing(16);
        hbox2->setContentsMargins(0, 0, 0, 0);

        auto* vbox2 = new QVBoxLayout();
        vbox2->addWidget(this->lblbrand);

        auto* vbox1 = new QVBoxLayout();
        vbox1->setSpacing(15);
        vbox1->setContentsMargins(15, -1, 15, -1);
        vbox1->addWidget(this->pbopen);
        vbox1->addWidget(this->pbsettings);
        vbox1->addWidget(this->pbabout);

        vbox2->addLayout(vbox1);
        vbox2->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum,
                                       QSizePolicy::Expanding));

        auto* hbox1 = new QHBoxLayout();
        hbox1->setContentsMargins(-1, -1, -1, 16);
        hbox1->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding,
                                       QSizePolicy::Minimum));

        auto* grid = new QGridLayout();
        grid->setSpacing(0);

        this->pbopenhome = new QPushButton(self);
        grid->addWidget(this->pbopenhome, 0, 0, 1, 1);

        this->pbopengithub = new QPushButton(self);
        grid->addWidget(this->pbopengithub, 1, 0, 1, 1);

        this->pbopentelegram = new QPushButton(self);
        grid->addWidget(this->pbopentelegram, 2, 0, 1, 1);

        this->pbopenreddit = new QPushButton(self);
        grid->addWidget(this->pbopenreddit, 3, 0, 1, 1);

        this->pbopenx = new QPushButton(self);
        grid->addWidget(this->pbopenx, 4, 0, 1, 1);

        hbox1->addLayout(grid);
        hbox1->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding,
                                       QSizePolicy::Minimum));

        vbox2->addLayout(hbox1);
        vbox2->setStretch(2, 1);

        hbox2->addLayout(vbox2);

        auto* line1 = new QFrame(self);
        line1->setFrameShadow(QFrame::Plain);
        line1->setFrameShape(QFrame::VLine);
        hbox2->addWidget(line1);

        auto* vbox3 = new QVBoxLayout();
        vbox3->setSpacing(10);
        vbox3->setContentsMargins(0, 10, -1, 0);
        this->lblrecents = new QLabel(self);
        this->lblrecents->setFont(font2);
        this->lblrecents->setText("Recent Files:");
        this->lblrecents->setAlignment(Qt::AlignLeading | Qt::AlignLeft |
                                       Qt::AlignVCenter);

        vbox3->addWidget(this->lblrecents);

        auto* line2 = new QFrame(self);
        line2->setFrameShadow(QFrame::Plain);
        line2->setFrameShape(QFrame::HLine);
        vbox3->addWidget(line2);

        this->lvrecents = new QListView(self);
        this->lvrecents->setFrameShape(QFrame::NoFrame);
        this->lvrecents->setEditTriggers(QAbstractItemView::NoEditTriggers);
        this->lvrecents->setSelectionMode(QAbstractItemView::NoSelection);
        this->lvrecents->setSelectionBehavior(QAbstractItemView::SelectRows);
        this->lvrecents->setUniformItemSizes(true);
        vbox3->addWidget(this->lvrecents);

        this->lblversion = new QLabel(self);
        this->lblversion->setAlignment(Qt::AlignBottom | Qt::AlignRight |
                                       Qt::AlignTrailing);

        vbox3->addWidget(this->lblversion);
        vbox3->setStretch(2, 1);
        hbox2->addLayout(vbox3);
        hbox2->setStretch(2, 1);
        vbox4->addLayout(hbox2);
    }
};

} // namespace ui
