#pragma once

#include <QToolBar>
#include <QWidget>

class SplitView;

class SplitWidget: public QWidget {
    Q_OBJECT

private:
    explicit SplitWidget(SplitView* view, SplitWidget* splitfrom = nullptr);

public:
    ~SplitWidget() override;
    [[nodiscard]] QAction* action(int idx) const;
    [[nodiscard]] QWidget* widget() const { return m_widget; }
    QAction* add_button(const QString& text);
    QAction* add_button(const QIcon& icon);
    QAction* add_button(QAction* action);
    QAction* add_widget(QWidget* w);
    void add_spacer();
    void create_default_buttons();
    void create_title();

public Q_SLOTS:
    void split_horizontally();
    void split_vertically();
    void open_in_dialog();
    void close_widget();

private Q_SLOTS:
    void update_close_button();

private:
    [[nodiscard]] SplitWidget* find_sibling_to_focus() const;
    void split(Qt::Orientation orientation);

private:
    SplitView* m_view;
    QWidget* m_widget{nullptr};
    QAction* m_actclose{nullptr};
    QToolBar* m_tbactions;

    friend class SplitView;
};
