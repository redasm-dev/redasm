#include "mainwindow.h"
#include "support/settings.h"
#include "support/themeprovider.h"
#include "support/utils.h"
#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDirIterator>
#include <QFileInfo>
#include <QStandardPaths>
#include <QString>
#include <QStyleFactory>

#if defined(_WIN32)
#include <io.h>
#define ACCESS _access
#else
#include <unistd.h>
#define ACCESS access
#endif

namespace {

void configure_search_paths() {
#if !defined(_WIN32)
    const char* appimage_dir = std::getenv("APPDIR");
    bool is_appimage = appimage_dir && std::getenv("APPIMAGE");
#else
    const char* appimage_dir = NULL;
    constexpr bool is_appimage = false;
#endif

    auto append_unique = [](auto& list, const auto& value) {
        if(!list.contains(value)) list.append(value);
    };

    // clang-format off
    if(is_appimage) {
        // 1. user plugins
        const QString USER_DIR = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        append_unique(utils::search_paths, USER_DIR + "/plugins");
        append_unique(utils::kb_search_paths, (USER_DIR + "/kb").toUtf8());

        // 2. bundled plugins
        const QString APP_DIR = QString::fromUtf8(appimage_dir);

#if defined(REDASM_PLUGIN_DIR)
        append_unique(utils::search_paths, APP_DIR + QString::fromUtf8(REDASM_PLUGIN_DIR));
#endif

        append_unique(utils::kb_search_paths, (APP_DIR + "/usr/share/redasm/kb").toUtf8());

        // 3. system plugins
        for(const QString& sp : QStandardPaths::standardLocations(QStandardPaths::AppDataLocation)) {
            if(sp == USER_DIR) continue; // already added, skip

            append_unique(utils::search_paths, sp + "/plugins");
            append_unique(utils::kb_search_paths, (sp + "/kb").toUtf8());
        }
    }
    else {
        const QString APP_DIR = qApp->applicationDirPath();

        append_unique(utils::search_paths, APP_DIR + "/plugins");
        append_unique(utils::kb_search_paths, (APP_DIR + "/kb").toUtf8());

#if defined(REDASM_PLUGIN_DIR)
        append_unique(utils::search_paths, QString::fromUtf8(REDASM_PLUGIN_DIR));
#endif

        for(const QString& sp : QStandardPaths::standardLocations(QStandardPaths::AppDataLocation)) {
            append_unique(utils::search_paths, sp + "/plugins");
            append_unique(utils::kb_search_paths, (sp + "/kb").toUtf8());
        }
    }
    // clang-format on
}

QVector<const char*> get_kb_search_paths() {
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

#if !defined(__has_feature)
#define __has_feature(x) 0
#endif

#if __has_feature(address_sanitizer) || defined(__SANITIZE_ADDRESS__)

extern "C" const char* __asan_default_options() { // NOLINT
    if(ACCESS(ASAN_SUPP_PATH, 0) == 0)
        return "suppressions=" ASAN_SUPP_PATH ":print_suppressions=0";

    return nullptr;
}

extern "C" const char* __lsan_default_options() { // NOLINT
    if(ACCESS(LSAN_SUPP_PATH, 0) == 0)
        return "suppressions=" LSAN_SUPP_PATH ":print_suppressions=0";

    return nullptr;
}

extern "C" const char* __lsan_default_suppressions() { // NOLINT
    return "";
}

#endif

int main(int argc, char** argv) {
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    QApplication app{argc, argv};
    app.setApplicationName("redasm");
    app.setApplicationDisplayName(
        QString{"REDasm %1"}.arg(rd_version_string()));
    app.setApplicationVersion(rd_version_string());

    QCommandLineParser parser;
    parser.setApplicationDescription("The OpenSource Disassembler");
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addPositionalArgument("file", "File to be analyzed");
    parser.process(app);

    int res = 0;

    { // Scoping makes sure that widgets and context are freed before deinit
        theme_provider::init();
        configure_search_paths();

        REDasmSettings settings;

        QVector<const char*> kb_paths = get_kb_search_paths();
        RDInitParams params = {};
        params.kb_paths = kb_paths.data();
        params.network_enabled = settings.network_enabled();

        MainWindow mw{params};
        load_modules();

        // process command line argument
        const QStringList ARGS = parser.positionalArguments();
        if(!ARGS.isEmpty() && QFileInfo{ARGS.first()}.isFile())
            mw.open_file(ARGS.first());

        mw.show();
        res = app.exec();
    }

    return res;
}
