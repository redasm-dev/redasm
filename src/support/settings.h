#pragma once

#include <QMainWindow>
#include <QSettings>

class REDasmSettings: public QSettings {
    Q_OBJECT

public:
    static constexpr int MAX_RECENT_FILES = 10;

public:
    explicit REDasmSettings(QObject* parent = nullptr);
    [[nodiscard]] QStringList recent_files() const;
    [[nodiscard]] QString current_theme() const;
    [[nodiscard]] QFont current_font() const;
    [[nodiscard]] int current_font_size() const;
    [[nodiscard]] bool network_enabled() const;
    [[nodiscard]] bool screenshot_logo() const;
    bool restore_state(QMainWindow* mainwindow);
    void default_state(QMainWindow* mainwindow);
    void save_state(const QMainWindow* mainwindow);
    void clear_recent_files();
    void update_recent_files(const QString& s = QString{});
    void change_theme(const QString& theme);
    void change_font(const QFont& font);
    void change_font_size(int size);
    void network_enabled(bool b);
    void screenshot_logo(bool b);

public:
    static QFont load_font();

private:
    static QByteArray m_defaultstate;
};
