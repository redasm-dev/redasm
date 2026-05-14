#include "mainwindow.h"
#include "support/surfacerenderer.h"
#include "support/themeprovider.h"
#include "support/utils.h"
#include <QApplication>
#include <QDirIterator>
#include <QFileInfo>
#include <QStandardPaths>
#include <QString>
#include <QStyleFactory>

namespace {

const QString REDASM_VERSION = "4.0";

void on_log(RDLogLevel level, const char* tag, const char* msg,
            void* userdata) {
    auto* mw = reinterpret_cast<MainWindow*>(userdata);
    mw->log(level, QString::fromUtf8(tag), QString::fromUtf8(msg));
}

void configure_searchpaths() {
    const char* appdir = std::getenv("APPDIR");
    bool isappimage = appdir && std::getenv("APPIMAGE");

    utils::search_paths =
        QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);

    if(isappimage) utils::search_paths.prepend(QString::fromUtf8(appdir));

    for(const QString& sp : utils::search_paths)
        utils::kb_search_paths.prepend((sp + "/kb").toUtf8());

    // TODO: davide - review this hardcoded path
    utils::kb_search_paths.prepend(
        (qApp->applicationDirPath() + "/../../redasm-dev/kb").toUtf8());

    utils::search_paths.prepend(qApp->applicationDirPath() + "/../processors");
    utils::search_paths.prepend(qApp->applicationDirPath() + "/../loaders");
    utils::search_paths.prepend(qApp->applicationDirPath() + "/../commands");
    utils::search_paths.prepend(qApp->applicationDirPath());
}

QVector<const char*> get_kb_searchpaths() {
    QVector<const char*> v;

    for(const QByteArray& ba : utils::kb_search_paths)
        v.push_back(ba.constData());

    v.push_back(nullptr);
    return v;
}

void load_modules() {
    for(const QString& searchpath : utils::search_paths) {
        QDirIterator it{searchpath, QDirIterator::Subdirectories};

        while(it.hasNext()) {
            QFileInfo fi = it.nextFileInfo();

            if(fi.isFile() && ("." + fi.suffix() == SHARED_OBJECT_EXT))
                rd_module_load(qUtf8Printable(fi.filePath()));
        }
    }
}

} // namespace

int main(int argc, char** argv) {
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    QApplication app{argc, argv};
    app.setApplicationName("redasm");
    app.setApplicationDisplayName("REDasm " + REDASM_VERSION);

    theme_provider::apply_theme();
    surface_renderer::init();
    int res = 0;

    { // Scoping makes sure that widgets and context are freed before deinit
        MainWindow mw;
        mw.show();

        configure_searchpaths();
        rd_set_log_callback(on_log, &mw);

        QVector<const char*> kb_paths = get_kb_searchpaths();

        RDInitParams params = {};
        params.kb_paths = kb_paths.data();
        rd_init(&params);

        load_modules();

        res = app.exec();
        rd_set_log_callback(nullptr, nullptr);
    }

    rd_deinit();
    return res;
}
