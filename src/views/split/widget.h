#pragma once

#include <QToolBar>
#include <QWidget>

class SplitView;

class SplitWidget: public QWidget {
    Q_OBJECT

private:
    explicit SplitWidget(SplitView* view);

public:
    ~SplitWidget() override;
    [[nodiscard]] QAction* action(int idx) const;
    [[nodiscard]] QWidget* widget() const { return m_widget; }
    QAction* add_button(const QIcon& icon);
    QAction* add_button(QAction* action);

public Q_SLOTS:
    void split_horizontally();
    void split_vertically();
    void open_in_dialog();
    void close_widget();

protected:
    void focusInEvent(QFocusEvent* event) override;

private Q_SLOTS:
    void update_close_button();

private:
    [[nodiscard]] QToolBar* create_toolbar();
    void split(Qt::Orientation orientation);
    void create_default_buttons();

private:
    SplitView* m_view;
    QWidget* m_widget;
    QAction *m_actfirstdefault{nullptr}, *m_actclose{nullptr};
    QToolBar* m_tbactions;

    friend class SplitView;
};
