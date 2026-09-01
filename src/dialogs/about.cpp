#include "about.h"
#include "support/themeprovider.h"
#include "support/utils.h"
#include <redasm/redasm.h>

namespace {

void compile_config(QTextBrowser* txb) {
    const QString VERSION_CONTENT = R"(
<div><b>Qt Version:</b> %1</div>
<div><b>Core Version:</b> %2</div>
<div><b>RDAPI Version:</b> %3.%4</div><br>
)";

    const QString SEARCH_PATHS_CONTENT = R"(
<div><b>Search Paths</b></div>
<div>%1</div>
)";

    txb->setLineWrapMode(QTextBrowser::NoWrap);
    txb->insertHtml(VERSION_CONTENT.arg(QT_VERSION_STR)
                        .arg(rd_version_build())
                        .arg(RD_API_VERSION_MAJOR(RD_API_VERSION))
                        .arg(RD_API_VERSION_MINOR(RD_API_VERSION)));

    if(!utils::search_paths.isEmpty()) {
        QString lines;

        for(const QString& sp : utils::search_paths)
            lines.append(QString{"<div>- %1</div>"}.arg(sp));

        txb->insertHtml(SEARCH_PATHS_CONTENT.arg(lines));
    }
    else {
        // clang-format off
        txb->insertHtml(SEARCH_PATHS_CONTENT.arg(QString{R"(
                <font color="%1">
                    <b>No Search paths set</b>
                </font>
        )"}.arg(theme_provider::color(RD_THEME_FAIL).name())));
        // clang-format on
    }
}

void compile_modules(QTextBrowser* txb) {
    const QString CONTENT = R"(
<table>
    %1
</table>
)";

    QString html;
    RDModuleSlice modules = rd_get_all_modules();

    const RDModule** it;
    rd_slice_each(it, modules) {
        const RDModule* m = *it;
        QString row = R"(<tr><td>%1</td></tr>)";
        html.append(row.arg(m->path));
    }

    txb->setLineWrapMode(QTextBrowser::NoWrap);
    txb->setHtml(CONTENT.arg(html));
}

void compile_loaders(QTextBrowser* txb) {
    RDPluginSlice loaders = rd_get_all_loader_plugins();
    QString html;

    const RDPlugin** it;
    rd_slice_each(it, loaders) {
        const RDPlugin* p = *it;
        html.append(QString{"<div>- %1</div>"}.arg(p->loader->id));
    }

    txb->setLineWrapMode(QTextBrowser::NoWrap);
    txb->setHtml(html);
}

void compile_processors(QTextBrowser* txb) {
    const QString CONTENT = R"(
<table>
    <tr>
        <th valign="middle" width="50%">Name</th>
        <th valign="middle" width="50%">Id</th>
    </tr>
    %1
</table>
)";

    RDPluginSlice processors = rd_get_all_processor_plugins();
    QString html;

    const RDPlugin** it;
    rd_slice_each(it, processors) {
        const RDPlugin* p = *it;

        QString row = R"(
                <tr>
                    <td align="center" valign="middle">%1</td>
                    <td align="center" valign="middle">%2</td>
                </tr>
            )";

        html.append(row.arg(p->processor->name).arg(p->processor->id));
    }

    txb->setLineWrapMode(QTextBrowser::NoWrap);
    txb->setHtml(CONTENT.arg(html));
}

void compile_analyzers(QTextBrowser* txb) {
    const QString CONTENT = R"(
<table>
    <tr>
        <th valign="middle" width="50%">Name</th>
        <th valign="middle" width="50%">Id</th>
        <th valign="middle" width="50%">Order</th>
    </tr>
    %1
</table>
)";

    RDPluginSlice analyzers = rd_get_all_analyzer_plugins();
    QString html;

    const RDPlugin** it;
    rd_slice_each(it, analyzers) {
        const RDPlugin* p = *it;

        QString row = R"(
                <tr>
                    <td align="center" valign="middle">%1</td>
                    <td align="center" valign="middle">%2</td>
                    <td align="center" valign="middle">%3</td>
                </tr>
            )";

        html.append(row.arg(p->analyzer->name)
                        .arg(p->analyzer->id)
                        .arg(p->analyzer->order));
    }

    txb->setLineWrapMode(QTextBrowser::NoWrap);
    txb->setHtml(CONTENT.arg(html));
}

void compile_commands(QTextBrowser* txb) {
    RDPluginSlice commands = rd_get_all_command_plugins();
    QString html;

    const RDPlugin** it;
    rd_slice_each(it, commands) {
        const RDPlugin* p = *it;
        QString params;
        const RDCommandParam* cp = p->command->params;

        while(cp->name && cp->kind) {
            if(!params.isEmpty()) params.append(", ");

            params.append(rd_command_valuekind_str(cp->kind))
                .append(" ")
                .append(cp->name);
            cp++;
        }

        html.append(QString("- %1(%2)").arg(p->command->id).arg(params));
    }

    txb->setLineWrapMode(QTextBrowser::NoWrap);
    txb->setHtml(html);
}

} // namespace

AboutDialog::AboutDialog(QWidget* parent): QDialog{parent}, m_ui{this} {
    this->setWindowTitle("About REDasm");

    m_ui.lblaboutlogo->setAlignment(Qt::AlignCenter);
    m_ui.lblaboutlogo->setPixmap(utils::get_about_logo());

    compile_config(m_ui.txbconfig);
    compile_modules(m_ui.txbmodules);
    compile_loaders(m_ui.txbloaders);
    compile_processors(m_ui.txbprocessors);
    compile_analyzers(m_ui.txbanalyzers);
    compile_commands(m_ui.txbcommands);
}
