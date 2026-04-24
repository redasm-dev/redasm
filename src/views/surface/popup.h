#pragma once

#include <QWidget>
#include <redasm/redasm.h>

class SurfacePopup: public QWidget {
    Q_OBJECT

private:
    static constexpr int N_ROWS = 5;

public:
    explicit SurfacePopup(RDContext* ctx, QWidget* parent = nullptr);
    ~SurfacePopup() override;
    bool popup(RDAddress address, const QString& hlword);

private:
    void more_rows();
    void less_rows();
    void render();

protected:
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void paintEvent(QPaintEvent* e) override;

private:
    RDContext* m_context;
    RDSurface* m_surface;
    QPointF m_lastpos;
    int m_nrows{N_ROWS};
};
