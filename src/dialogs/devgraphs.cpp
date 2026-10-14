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

    connect(m_ui.rbfuncgraph, &QRadioButton::clicked, this, [&](bool checked) {
        if(checked) this->show_dot(m_ui.tvfunctions->currentIndex());
    });

    connect(m_ui.rbcallgraph, &QRadioButton::clicked, this, [&](bool checked) {
        if(checked) this->show_dot(m_ui.tvfunctions->currentIndex());
    });

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
}

void DevGraphsDialog::copy_graph_hashes() {
    QString s;

    for(int i = 0; i < m_functionsmodel->rowCount({}); i++) {
        QModelIndex index = m_functionsmodel->index(i);
        RDAddress address = m_functionsmodel->address(index);

        auto f_dot = this->get_function_dot(address);
        auto c_dot = this->get_call_dot(address);

        Q_ASSERT(f_dot.second);
        Q_ASSERT(c_dot.second);

        s.append(QString{"  {0x%1, 0x%2, 0x%3},\n"}
                     .arg(utils::to_hex(address))
                     .arg(utils::to_hex(f_dot.second))
                     .arg(utils::to_hex(c_dot.second)));
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

    auto [dot, hash] = m_ui.rbcallgraph->isChecked()
                           ? this->get_call_dot(address)
                           : this->get_function_dot(address);

    bool success = hash && !dot.isEmpty();

    if(success) {
        m_currenthash = hash;
        m_currentgraph = dot;
        m_ui.ptedot->setPlainText(m_currentgraph);
    }
    else {
        m_currenthash = {};
        m_currentgraph = {};
        m_ui.ptedot->clear();
    }

    m_ui.ftbcopyhash->setEnabled(success);
    m_ui.ftbcopygraph->setEnabled(success);
}

DevGraphsDialog::DotResult
DevGraphsDialog::get_function_dot(RDAddress address) {
    const char* dot = nullptr;
    u32 hash = 0;
    QString graph;

    RDFunctionGraph* g = rd_functiongraph_create(m_context);
    if(!rd_functiongraph_populate(g, address)) goto cleanup;

    dot = rd_functiongraph_generate_dot(g);
    if(!dot) goto cleanup;

    hash = rd_functiongraph_get_hash(g);
    graph = QString::fromUtf8(dot);

cleanup:
    rd_functiongraph_destroy(g);
    return {graph, hash};
}

DevGraphsDialog::DotResult DevGraphsDialog::get_call_dot(RDAddress address) {
    const char* dot = nullptr;
    u32 hash = 0;
    QString graph;

    RDCallGraph* g = rd_callgraph_create(m_context, address);
    if(!g) goto cleanup;

    dot = rd_callgraph_generate_dot(g);
    if(!dot) goto cleanup;

    hash = rd_callgraph_get_hash(g);
    graph = QString::fromUtf8(dot);

cleanup:
    rd_callgraph_destroy(g);
    return {graph, hash};
}
