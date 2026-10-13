#include "devgraphs.h"
#include "support/surfacerenderer.h"
#include "support/utils.h"
#include <QApplication>
#include <QClipboard>
#include <QHeaderView>

DevGraphsDialog::DevGraphsDialog(RDContext* ctx, QWidget* parent)
    : QDialog{parent}, m_ui{this}, m_context{ctx} {

    m_ui.ftbcopyhash->setEnabled(false);
    m_ui.ftbcopygraph->setEnabled(false);

    m_functionsmodel = new FunctionsModel(ctx, m_ui.tvfunctions);
    m_ui.tvfunctions->setModel(m_functionsmodel);

    m_ui.ptedot->setFont(surface_renderer::get_font());
    m_ui.ptedot->setTabStopDistance(4 * surface_renderer::cell_width());

    QHeaderView* hdrview = m_ui.tvfunctions->header();
    hdrview->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    hdrview->setSectionResizeMode(1, QHeaderView::Stretch);

    this->setFocus(); // don't focus function list

    connect(m_ui.tvfunctions, &QTreeView::clicked, this,
            &DevGraphsDialog::show_dot);

    connect(m_ui.ftbcopygraphhashes, &FeedbackPushButton::feedback, this,
            &DevGraphsDialog::copy_graph_hashes);

    connect(m_ui.ftbcopyfunctions, &FeedbackPushButton::feedback, this,
            &DevGraphsDialog::copy_functions);

    connect(m_ui.ftbcopygraph, &FeedbackPushButton::feedback, this,
            [&]() { qApp->clipboard()->setText(m_currentgraph); });

    connect(m_ui.ftbcopyhash, &FeedbackPushButton::feedback, this, [&]() {
        qApp->clipboard()->setText(
            QString{"0x%1"}.arg(utils::to_hex(m_currenthash)));
    });

    connect(m_ui.chklayoutdot, &QCheckBox::checkStateChanged, this,
            [&]() { this->show_dot(m_ui.tvfunctions->currentIndex()); });
}

void DevGraphsDialog::copy_graph_hashes() {
    QString s;

    for(int i = 0; i < m_functionsmodel->rowCount({}); i++) {
        QModelIndex index = m_functionsmodel->index(i);
        RDAddress address = m_functionsmodel->address(index);

        RDFunctionGraph* g = rd_functiongraph_create(m_context);
        bool done = rd_functiongraph_populate(g, address);
        Q_ASSERT(done);

        u32 hash = rd_functiongraph_get_hash(g);

        s.append(QString{"  {0x%1, 0x%2},\n"}
                     .arg(utils::to_hex(address))
                     .arg(utils::to_hex(hash)));

        rd_functiongraph_destroy(g);
    }

    qApp->clipboard()->setText(s);
}

void DevGraphsDialog::copy_functions() {
    QString s;

    for(int i = 0; i < m_functionsmodel->rowCount({}); i++) {
        QModelIndex index = m_functionsmodel->index(i);
        RDAddress address = m_functionsmodel->address(index);

        const char* name = rd_get_name(m_context, address);
        Q_ASSERT(name);

        s.append(QString{"  {0x%1, \"%2\"},\n"}
                     .arg(utils::to_hex(address))
                     .arg(QString::fromUtf8(name)));
    }

    qApp->clipboard()->setText(s);
}

void DevGraphsDialog::show_dot(const QModelIndex& index) {
    m_currentgraph = {};
    m_currenthash = {};
    m_ui.ptedot->clear();

    if(!index.isValid()) return;

    RDAddress address = m_functionsmodel->address(index);
    const char* dot = nullptr;

    RDFunctionGraph* g = rd_functiongraph_create(m_context);
    if(!rd_functiongraph_populate(g, address)) goto fail;

    dot = m_ui.chklayoutdot->isChecked()
              ? rd_functiongraph_generate_dot_layout(g)
              : rd_functiongraph_generate_dot(g);
    if(!dot) goto fail;

    m_currenthash = m_ui.chklayoutdot->isChecked()
                        ? rd_functiongraph_get_hash_layout(g)
                        : rd_functiongraph_get_hash(g);
    m_currentgraph = QString::fromUtf8(dot);

    m_ui.ptedot->setPlainText(m_currentgraph);
    m_ui.ftbcopyhash->setEnabled(true);
    m_ui.ftbcopygraph->setEnabled(true);
    rd_functiongraph_destroy(g);
    return;

fail:
    m_ui.ftbcopyhash->setEnabled(false);
    m_ui.ftbcopygraph->setEnabled(false);
    rd_functiongraph_destroy(g);
}
