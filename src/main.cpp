#include "mainwindow.h"
#include "support/themeprovider.h"
#include <QApplication>
#include <QString>
#include <QStyleFactory>

namespace {

const QString REDASM_VERSION = "4.0";
bool running = true;

} // namespace

int main(int argc, char** argv) {
    rd_init();
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    QApplication app{argc, argv};
    app.setApplicationName("redasm");
    app.setApplicationDisplayName("REDasm " + REDASM_VERSION);

    themeprovider::apply_theme();

    { // Scoping makes sure that widgets and context are freed before deinit
        MainWindow mw;
        QObject::connect(&mw, &MainWindow::closed, []() { running = false; });
        mw.show();

        while(running) {
            if(mw.loop())
                app.processEvents(QEventLoop::AllEvents);
            else
                app.processEvents(QEventLoop::WaitForMoreEvents);
        }
    }

    rd_deinit();
    return 0;
}
