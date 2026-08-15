#include "log.h"
#include "support/themeprovider.h"
#include <QFontDatabase>
#include <QScrollBar>

LogView::LogView(QWidget* parent): QWidget{parent}, m_ui{this} {
    this->setVisible(false);

    this->add_log_level("ALL", INT_MAX);
    this->add_log_level(QString{}, INT_MIN);

#if !defined(NDEBUG)
    this->add_log_level("DEBUG", RD_LOGLEVEL_DEBUG);
#endif

    this->add_log_level("INFO", RD_LOGLEVEL_INFO);
    this->add_log_level("WARN", RD_LOGLEVEL_WARN);
    this->add_log_level("FAIL", RD_LOGLEVEL_FAIL);

    QFont f = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    m_ui.ptelogs->setFont(f);
    m_ui.ptelogs->document()->setDocumentMargin(0);

    this->update_log_levels();

    connect(m_ui.tbclear, &QToolButton::clicked, this, &LogView::clear);

    connect(m_ui.lefilter, &QLineEdit::textChanged, this,
            [&](const QString&) { this->update_log(); });
}

void LogView::clear() {
    this->setVisible(false);
    m_ui.lefilter->clear();
    m_ui.ptelogs->clear();
    m_entries.clear();
}

const LogView::LogEntry&
LogView::collect_log(RDLogLevel level, const QString& tag, const QString& msg) {
    m_entries.push_back(LogEntry{level, tag, msg});
    return m_entries.back();
}

void LogView::add_log_level(const QString& text, int ll) {
    QAction* act = nullptr;

    if(ll == INT_MIN) {
        act = new QAction(m_ui.tblevels);
        act->setSeparator(true);
        m_ui.tblevels->addAction(act);
        return;
    }

    if(ll == INT_MAX) {
        m_ui.tblevels->addAction(text, this, [&]() {
            QList<QAction*> actions = m_ui.tblevels->actions();

            for(QAction* a : actions) {
                if(a->data().toInt() == INT_MIN) continue;
                if(a->data().toInt() == INT_MAX) continue;

                if(!a->isChecked()) a->setChecked(true);
            }
        });

        return;
    }

    act = m_ui.tblevels->addAction(text);
    act->setCheckable(true);
    act->setChecked(true);

    m_levels[ll] = true;

    connect(act, &QAction::toggled, this, [&, ll, act]() {
        m_levels[ll] = act->isChecked();
        this->update_log_levels();
    });
}

void LogView::update_log_levels() {
    bool none = true, all = true;
    QString text;

    for(auto it = m_levels.begin(); it != m_levels.end(); it++) {
        if(!text.isEmpty() && it.value()) text.append(" | ");

        switch(it.key()) {
            case RD_LOGLEVEL_DEBUG:
                if(it.value()) text.append("DEBUG");
                break;

            case RD_LOGLEVEL_INFO:
                if(it.value()) text.append("INFO");
                break;

            case RD_LOGLEVEL_WARN:
                if(it.value()) text.append("WARN");
                break;

            case RD_LOGLEVEL_FAIL:
                if(it.value()) text.append("FAIL");
                break;

            default: qFatal("unknown log level"); break;
        }

        if(it.value()) none = false; // at least 1 item is checked
        if(!it.value()) all = false; // at least 1 item is unchecked
    }

    if(none)
        m_ui.tblevels->setText("NONE");
    else if(all)
        m_ui.tblevels->setText("ALL");
    else
        m_ui.tblevels->setText(text);

    this->update_log();
}

void LogView::update_log() {
    m_ui.ptelogs->clear();

    for(const LogEntry& e : m_entries) {
        if(this->accept_log(e)) this->append_log(e, false);
    }

    m_ui.ptelogs->moveCursor(QTextCursor::End);

    QScrollBar* sb = m_ui.ptelogs->verticalScrollBar();
    sb->setValue(sb->maximum());
}

bool LogView::accept_log(const LogEntry& e) const {
    if(!m_levels.contains(e.level) || !m_levels[e.level]) return false;

    QString s = m_ui.lefilter->text();
    if(s.isEmpty()) return true;

    return e.tag.contains(s, Qt::CaseInsensitive) ||
           e.msg.contains(s, Qt::CaseInsensitive);
}

void LogView::append_log(const LogEntry& e, bool update) {
    if(!this->isVisible()) this->setVisible(true);

    QTextCursor cursor = m_ui.ptelogs->textCursor();
    cursor.movePosition(QTextCursor::End);
    if(!m_ui.ptelogs->document()->isEmpty()) cursor.insertBlock();

    QTextCharFormat defaultfmt, tagfmt;
    tagfmt.setForeground(theme_provider::color(RD_THEME_COMMENT));
    cursor.insertText("[", defaultfmt);
    cursor.insertText(e.tag, tagfmt);
    cursor.insertText("] ", defaultfmt);

    QColor c;

    switch(e.level) {
        case RD_LOGLEVEL_DEBUG:
            c = theme_provider::color(RD_THEME_MUTED);
            break;
        case RD_LOGLEVEL_WARN:
            c = theme_provider::color(RD_THEME_WARNING);
            break;
        case RD_LOGLEVEL_FAIL: c = theme_provider::color(RD_THEME_FAIL); break;
        default: c = theme_provider::color(RD_THEME_FOREGROUND); break;
    }

    QTextCharFormat fmt;
    fmt.setForeground(c);
    cursor.insertText(e.msg, fmt);

    if(!update) return;

    m_ui.ptelogs->moveCursor(QTextCursor::End);

    QScrollBar* sb = m_ui.ptelogs->verticalScrollBar();
    sb->setValue(sb->maximum());
}

void LogView::log(RDLogLevel level, const QString& tag, const QString& msg) {
    const LogEntry& e = this->collect_log(level, tag, msg);
    if(this->accept_log(e)) this->append_log(e, true);
}
