#pragma once

#include <QMouseEvent>
#include <QObject>
#include <QPainter>
#include <QRect>
#include <redasm/redasm.h>

class GraphView;

class GraphViewNode: public QObject {
    Q_OBJECT

    friend class GraphView;

public:
    enum { NONE = 0, SELECTED, FOCUSED };

public:
    explicit GraphViewNode(RDGraphNode node, QWidget* parent = nullptr);
    [[nodiscard]] RDGraphNode node() const;
    [[nodiscard]] int x() const;
    [[nodiscard]] int y() const;
    [[nodiscard]] int width() const;
    [[nodiscard]] int height() const;
    [[nodiscard]] QRect rect() const;
    [[nodiscard]] bool contains(const QPoint& p) const;
    [[nodiscard]] QPoint position() const;
    [[nodiscard]] QWidget* parent_widget() const;
    void move(const QPoint& pos);

protected:
    [[nodiscard]] int adjusted_x(int x) const;
    [[nodiscard]] int adjusted_y(int y) const;
    [[nodiscard]] int adjusted_width(int w) const;
    [[nodiscard]] int adjusted_height(int h) const;
    void draw_chrome(QPainter* p, usize state, bool shadow) const;
    void draw_edge(QPainter* p, usize state) const;

protected:
    virtual void itemselection_changed(bool selected);
    virtual void mousedoubleclick_event(QMouseEvent* e);
    virtual void mousepress_event(QMouseEvent* e);
    virtual void mousemove_event(QMouseEvent* e);
    virtual void mouseenter_event(QMouseEvent* e);
    virtual void mouseleave_event(QMouseEvent* e);

public:
    [[nodiscard]] QPoint map_to_item(const QPoint& p) const;
    [[nodiscard]] virtual int current_row() const;
    [[nodiscard]] virtual QSize size() const = 0;
    virtual void render(QPainter* painter, usize state) = 0;
    virtual void invalidate(bool notify);

public Q_SLOTS:
    void invalidate() { this->invalidate(true); }

private:
    void draw_shadow(QPainter* p, usize state) const;

Q_SIGNALS:
    void invalidated();

private:
    QPoint m_pos;
    RDGraphNode m_node;
    const RDGraph* m_graph;
};
