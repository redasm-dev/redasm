#pragma once

#include <QMouseEvent>
#include <QObject>
#include <QPainter>
#include <QRect>
#include <redasm/redasm.h>

#define BLOCK_MARGIN 4
#define BLOCK_MARGINS -BLOCK_MARGIN, 0, BLOCK_MARGIN, 0

class GraphViewNode: public QObject {
    Q_OBJECT

    friend class GraphView;

public:
    enum { NONE = 0, SELECTED, FOCUSED };

public:
    explicit GraphViewNode(RDGraphNode node, QObject* parent = nullptr);
    [[nodiscard]] RDGraphNode node() const;
    [[nodiscard]] int x() const;
    [[nodiscard]] int y() const;
    [[nodiscard]] int width() const;
    [[nodiscard]] int height() const;
    [[nodiscard]] QRect rect() const;
    [[nodiscard]] bool contains(const QPoint& p) const;
    [[nodiscard]] const QPoint& position() const;
    void move(const QPoint& pos);

protected:
    virtual void itemselection_changed(bool selected);
    virtual void mousedoubleclick_event(QMouseEvent* e);
    virtual void mousepress_event(QMouseEvent* e);
    virtual void mousemove_event(QMouseEvent* e);

public:
    [[nodiscard]] QPoint map_to_item(const QPoint& p) const;
    [[nodiscard]] virtual int current_row() const;
    [[nodiscard]] virtual QSize size() const = 0;
    virtual void render(QPainter* painter, size_t state) = 0;
    virtual void invalidate(bool notify);

public Q_SLOTS:
    void invalidate() { this->invalidate(true); }

Q_SIGNALS:
    void invalidated();

private:
    QPoint m_pos;
    RDGraphNode m_node;
    const RDGraph* m_graph;
};
