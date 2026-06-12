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
}

void DevGraphsDialog::copy_graph_hashes() {
    QString s = "{\n";

    for(int i = 0; i < m_functionsmodel->rowCount({}); i++) {
        QModelIndex index = m_functionsmodel->index(i);
        RDAddress address = m_functionsmodel->address(index);
        const RDFunction* f = rd_find_function(m_context, address);
        Q_ASSERT(f);
        u32 hash = rd_function_get_hash(f);

        s.append(QString{"  {0x%1, 0x%2},\n"}
                     .arg(utils::to_hex(address))
                     .arg(utils::to_hex(hash)));
    }

    s.append("};");
    qApp->clipboard()->setText(s);
}

void DevGraphsDialog::copy_functions() {
    QString s = "{\n";

    for(int i = 0; i < m_functionsmodel->rowCount({}); i++) {
        QModelIndex index = m_functionsmodel->index(i);
        RDAddress address = m_functionsmodel->address(index);
        const RDFunction* f = rd_find_function(m_context, address);
        Q_ASSERT(f);
        const char* name = rd_get_name(m_context, address);
        Q_ASSERT(name);

        s.append(QString{"  {0x%1, \"%2\"},\n"}
                     .arg(utils::to_hex(address))
                     .arg(QString::fromUtf8(name)));
    }

    s.append("};");
    qApp->clipboard()->setText(s);
}

void DevGraphsDialog::show_dot(const QModelIndex& index) {
    m_currentgraph = {};
    m_currenthash = {};
    m_ui.ptedot->clear();

    RDAddress address = m_functionsmodel->address(index);
    const char* dot = nullptr;

    const RDFunction* f = rd_find_function(m_context, address);
    if(!f) goto fail;

    dot = rd_function_generate_dot(f);
    if(!dot) goto fail;

    m_currenthash = rd_function_get_hash(f);
    m_currentgraph = QString::fromUtf8(dot);

    m_ui.ptedot->setPlainText(m_currentgraph);
    m_ui.ftbcopyhash->setEnabled(true);
    m_ui.ftbcopygraph->setEnabled(true);
    return;

fail:
    m_ui.ftbcopyhash->setEnabled(false);
    m_ui.ftbcopygraph->setEnabled(false);
}
