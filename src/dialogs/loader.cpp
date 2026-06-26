#include "loader.h"
#include "support/utils.h"
#include <QPushButton>

LoaderDialog::LoaderDialog(RDTestResultSlice ctxslice, QWidget* parent)
    : QDialog{parent}, m_ui{this}, m_testslice{ctxslice} {
    utils::configure_hex_input(m_ui.leentrypoint);
    utils::configure_hex_input(m_ui.leaddress);
    utils::configure_hex_input(m_ui.leoffset);

    this->accept_params.min_string = RD_MIN_STRING_LENGTH;

    m_ui.gbaddressing->setEnabled(false);
    m_ui.leentrypoint->setText(QString{"0"}.repeated(sizeof(u32) * 2));
    m_ui.leaddress->setText(QString{"0"}.repeated(sizeof(u32) * 2));
    m_ui.leoffset->setText(QString{"0"}.repeated(sizeof(u32) * 2));

    this->populate_processors();

    for(usize i = 0; i < rd_slice_length(m_testslice); i++) {
        const char* ldr_name =
            rd_testresult_get_loader_name(rd_slice_at(m_testslice, i));
        m_ui.lwloaders->addItem(QString::fromUtf8(ldr_name));
    }

    connect(m_ui.lwloaders, &QListWidget::currentRowChanged, this,
            &LoaderDialog::on_loader_changed);

    connect(m_ui.cbprocessors, &QComboBox::currentIndexChanged, this,
            &LoaderDialog::on_processor_changed);

    connect(m_ui.sbminstring, &QSpinBox::valueChanged, this,
            &LoaderDialog::update_min_string);

    connect(m_ui.rbnewanalysis, &QRadioButton::clicked, this,
            &LoaderDialog::update_open_mode);

    connect(m_ui.rbopenproject, &QRadioButton::clicked, this,
            &LoaderDialog::update_open_mode);

    connect(m_ui.leentrypoint, &QLineEdit::textChanged, this,
            &LoaderDialog::update_entry_point);

    connect(m_ui.leaddress, &QLineEdit::textChanged, this,
            &LoaderDialog::update_address);

    connect(m_ui.leoffset, &QLineEdit::textChanged, this,
            &LoaderDialog::update_offset);

    // Trigger "on_loader_changed"
    if(!rd_slice_is_empty(m_testslice)) m_ui.lwloaders->setCurrentRow(0);

    this->validate_fields();
    this->update_open_mode();
}

void LoaderDialog::on_loader_changed(int currentrow) {
    if(currentrow != -1) {
        this->sel_test = rd_slice_at(m_testslice, currentrow);
        m_ui.sbminstring->setValue(RD_MIN_STRING_LENGTH);

        const RDLoaderPlugin* l =
            rd_testresult_get_loader_plugin(this->sel_test);
        const RDProcessorPlugin* p =
            rd_testresult_get_processor_plugin(this->sel_test);

        m_ui.gbaddressing->setEnabled(m_ui.rbnewanalysis->isChecked() &&
                                      (l->flags & RD_LF_MANUAL));

        for(int i = 0; i < m_ui.cbprocessors->count(); i++) {
            if(m_ui.cbprocessors->itemData(i).toString() == p->id) {
                m_ui.cbprocessors->setCurrentIndex(i);
                return;
            }
        }

        m_ui.cbprocessors->setCurrentIndex(-1);
    }
    else {
        m_ui.gbaddressing->setEnabled(false);
        this->sel_test = nullptr;
    }

    this->validate_fields();
}

void LoaderDialog::on_processor_changed(int currentrow) {
    int loaderidx = m_ui.lwloaders->currentRow();

    if(loaderidx != -1 && currentrow != -1) {
        QVariant d = m_ui.cbprocessors->itemData(currentrow);
        this->accept_params.processorplugin =
            rd_processor_find(qUtf8Printable(d.toString()));
    }
    else
        this->accept_params.processorplugin = nullptr;

    this->validate_fields();
}

void LoaderDialog::populate_processors() const {
    RDPluginSlice plugins = rd_get_all_processor_plugins();

    const RDPlugin** it;
    rd_slice_each(it, plugins) {
        m_ui.cbprocessors->addItem((*it)->processor->name,
                                   (*it)->processor->id);
    }
}

void LoaderDialog::update_min_string() {
    this->accept_params.min_string = m_ui.sbminstring->value();
}

void LoaderDialog::update_open_mode() {
    if(m_ui.rbnewanalysis->isChecked())
        this->accept_params.mode = RD_AM_NEW;
    else if(m_ui.rbopenproject->isChecked())
        this->accept_params.mode = RD_AM_PROJECT;
    else
        qFatal("cannot set an open mode");

    m_ui.gbloader->setEnabled(m_ui.rbnewanalysis->isChecked());

    const RDLoaderPlugin* l = rd_testresult_get_loader_plugin(this->sel_test);

    m_ui.gbaddressing->setEnabled(m_ui.rbnewanalysis->isChecked() &&
                                  (l && l->flags & RD_LF_MANUAL));
}

void LoaderDialog::update_entry_point() {
    this->accept_params.addressing.entrypoint =
        m_ui.leentrypoint->text().toULongLong(nullptr, 16);
    this->validate_fields();
}

void LoaderDialog::update_address() {
    this->accept_params.addressing.address =
        m_ui.leentrypoint->text().toULongLong(nullptr, 16);
    this->validate_fields();
}

void LoaderDialog::update_offset() {
    this->accept_params.addressing.offset =
        m_ui.leoffset->text().toULongLong(nullptr, 16);
    this->validate_fields();
}

void LoaderDialog::validate_fields() { // NOLINT
    QPushButton* pb = m_ui.buttonbox->button(QDialogButtonBox::Ok);

    if(!this->sel_test || m_ui.cbprocessors->currentIndex() == -1) {
        pb->setEnabled(false);
        return;
    }

    const RDLoaderPlugin* l = rd_testresult_get_loader_plugin(this->sel_test);

    if(l->flags & RD_LF_MANUAL) {
        pb->setEnabled(!m_ui.leentrypoint->text().isEmpty() &&
                       !m_ui.leaddress->text().isEmpty() &&
                       !m_ui.leoffset->text().isEmpty());
    }
    else
        pb->setEnabled(true);
}
