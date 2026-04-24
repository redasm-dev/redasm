#include "settings.h"
#include "support/settings.h"
#include "support/themeprovider.h"
#include <QApplication>
#include <QMessageBox>

SettingsDialog::SettingsDialog(QWidget* parent): QDialog{parent}, m_ui{this} {
    m_ui.cbxthemes->addItems(themeprovider::themes());
    m_ui.fcbxfonts->setFontFilters(QFontComboBox::MonospacedFonts);

    auto sizes = QFontDatabase::standardSizes();

    for(int size : sizes)
        m_ui.cbxfontsizes->addItem(QString::number(size), size);

    this->select_current_theme();
    this->select_current_font();
    this->select_current_size();
    this->update_preview();

    connect(m_ui.fcbxfonts, &QFontComboBox::currentFontChanged, this,
            [&](const QFont&) { this->update_preview(); });
    connect(m_ui.cbxfontsizes, &QComboBox::currentTextChanged, this,
            [&](const QString&) { this->update_preview(); });
    connect(m_ui.pbfontdefault, &QPushButton::clicked, this,
            &SettingsDialog::select_default_font);
}

void SettingsDialog::select_size(int size) { // NOLINT
    for(int i = 0; i < m_ui.cbxfontsizes->count(); i++) {
        if(m_ui.cbxfontsizes->itemData(i).toInt() != size) continue;

        m_ui.cbxfontsizes->setCurrentIndex(i);
        break;
    }
}

void SettingsDialog::select_current_theme() { // NOLINT
    REDasmSettings settings;

    for(int i = 0; i < m_ui.cbxthemes->count(); i++) {
        if(QString::compare(m_ui.cbxthemes->itemText(i),
                            settings.current_theme(), Qt::CaseInsensitive))
            continue;

        m_ui.cbxthemes->setCurrentIndex(i);
        break;
    }
}

void SettingsDialog::select_current_font() { // NOLINT
    REDasmSettings settings;
    m_ui.fcbxfonts->setCurrentFont(settings.current_font());
}

void SettingsDialog::select_current_size() {
    REDasmSettings settings;
    this->select_size(settings.current_font_size());
}

void SettingsDialog::update_preview() const {
    QFont font = m_ui.fcbxfonts->currentFont();
    font.setPixelSize(m_ui.cbxfontsizes->currentData().toInt());
    m_ui.lblpreview->setFont(font);
}

void SettingsDialog::accept() {
    REDasmSettings settings;
    settings.change_theme(m_ui.cbxthemes->currentText());
    settings.change_font(m_ui.fcbxfonts->currentFont());
    settings.change_font_size(m_ui.cbxfontsizes->currentData().toInt());

    QMessageBox::information(this, "Settings Applied",
                             "Restart to apply settings");

    QDialog::accept();
}

void SettingsDialog::select_default_font() {
    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    m_ui.fcbxfonts->setCurrentFont(font);

    int size = qApp->font().pixelSize();
    if(size == -1) size = QFontMetrics{qApp->font()}.height();

    this->select_size(size);
}
