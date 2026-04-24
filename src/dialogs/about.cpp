#include "about.h"
#include "support/themeprovider.h"
#include "support/utils.h"
#include <redasm/redasm.h>

namespace {

constexpr int SCALE_LOGO = 128;
constexpr int SCALE_TEXT = 32;

const QString ABOUT_HTML = R"(
<div><b>Qt Version:</b> %1</div>
<div><b>Version:</b> %2</div>
<div><b>RDAPI Level:</b> %3</div>
<hr>
<div><b>Search Paths</b></div>
<div>%4</div>
<hr>
<table>
    <tr>
        <th align="center" valign="middle" width="50%">Loaders</th>
        <th align="center" valign="middle" width="50%">Processors</th>
    </tr>
    %5
</table>
)";

void compile_versions(QString& html) {
    html = html.arg(QT_VERSION_STR).arg(RD_VERSION_STR).arg(RD_API_LEVEL);
}

void compile_searchpaths(QString& html) {
    if(!utils::search_paths.isEmpty()) {
        QString lines;

        for(const QString& sp : utils::search_paths)
            lines.append(QString{"<div>- %1</div>"}.arg(sp));

        html = html.arg(lines);
    }
    else {
        // clang-format off
        html = html.arg(QString{R"(
                <font color="%1">
                    <b>No Search paths set</b>
                </font>
            )"}.arg(themeprovider::color(RD_THEME_FAIL).name()));
        // clang-format on
    }
}

void compile_plugins(QString& html) {
    RDPluginSlice pldr = rd_get_all_loader_plugins();
    RDPluginSlice pproc = rd_get_all_processor_plugins();
    QString plugins;

    for(usize i = 0; i < qMax(pldr.length, pproc.length); i++) {
        QString row = R"(
            <tr>
                <td align="center" valign="middle">%1</td>
                <td align="center" valign="middle">%2</td>
            </tr>
        )";

        if(i < rd_slice_length(pldr)) {
            row = row.arg(QString{"%1 [%2]"}
                              .arg(rd_slice_at(pldr, i)->loader->name)
                              .arg(rd_slice_at(pldr, i)->loader->id));
        }
        else
            row = row.arg(QString{});

        if(i < rd_slice_length(pproc)) {
            row = row.arg(QString{"%1 [%2]"}
                              .arg(rd_slice_at(pproc, i)->processor->name)
                              .arg(rd_slice_at(pproc, i)->processor->id));
        }
        else
            row = row.arg(QString{});

        plugins.append(row);
    }

    html = html.arg(plugins);
}

} // namespace

AboutDialog::AboutDialog(QWidget* parent): QDialog{parent}, m_ui{this} {
    this->setWindowTitle("About REDasm");

    m_ui.lbllogo->setPixmap(utils::get_logo().scaledToHeight(SCALE_LOGO));
    m_ui.lbltitle->setText("The OpenSource Disassembler");
    m_ui.lbltitle->setStyleSheet(QString{"font-size: %1px"}.arg(SCALE_TEXT));

    QString html = ABOUT_HTML;
    compile_versions(html);
    compile_searchpaths(html);
    compile_plugins(html);
    m_ui.txbabout->setHtml(html);
}
