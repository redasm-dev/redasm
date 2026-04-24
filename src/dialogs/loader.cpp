#include "loader.h"
#include "support/utils.h"

LoaderDialog::LoaderDialog(RDContextSlice ctxslice, QWidget* parent)
    : QDialog{parent}, m_ui{this}, m_contextslice{ctxslice} {

    m_ui.gbaddressing->setEnabled(false);

    this->populate_processors();

    for(usize i = 0; i < rd_slice_length(m_contextslice); i++) {
        const RDLoaderPlugin* lp =
            rd_get_loader_plugin(rd_slice_at(m_contextslice, i));
        m_ui.lwloaders->addItem(lp->name);
    }

    connect(m_ui.lwloaders, &QListWidget::currentRowChanged, this,
            &LoaderDialog::on_loader_changed);

    connect(m_ui.cbprocessors, &QComboBox::currentIndexChanged, this,
            &LoaderDialog::on_processor_changed);

    // Trigger "on_loader_changed"
    if(!rd_slice_is_empty(m_contextslice)) m_ui.lwloaders->setCurrentRow(0);
}

void LoaderDialog::on_loader_changed(int currentrow) {
    if(currentrow != -1) {
        this->context = rd_slice_at(m_contextslice, currentrow);
        const RDProcessorPlugin* p = rd_get_processor_plugin(this->context);

        for(int i = 0; i < m_ui.cbprocessors->count(); i++) {
            if(m_ui.cbprocessors->itemData(i).toString() == p->id) {
                m_ui.cbprocessors->setCurrentIndex(i);
                return;
            }
        }

        m_ui.cbprocessors->setCurrentIndex(-1);
    }
    else
        this->context = nullptr;
}

void LoaderDialog::on_processor_changed(int currentrow) {
    int loaderidx = m_ui.lwloaders->currentRow();

    if(loaderidx != -1 && currentrow != -1) {
        QVariant data = m_ui.cbprocessors->itemData(currentrow);
        this->processorplugin =
            rd_processor_find(qUtf8Printable(data.toString()));
    }
    else
        this->processorplugin = nullptr;
}

void LoaderDialog::populate_processors() const {
    RDPluginSlice plugins = rd_get_all_processor_plugins();

    const RDPlugin** it;
    rd_slice_each(it, plugins) {
        m_ui.cbprocessors->addItem((*it)->processor->name,
                                   (*it)->processor->id);
    }
}
