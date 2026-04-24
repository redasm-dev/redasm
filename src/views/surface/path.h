#pragma once

#include <QWidget>
#include <redasm/redasm.h>

class SurfaceListing;

class SurfacePath: public QWidget {
    Q_OBJECT

public:
    explicit SurfacePath(SurfaceListing* surface, QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    [[nodiscard]] bool is_path_selected(const RDSurfacePathItem* p) const;
    void fill_arrow(QPainter* painter, int y);

private:
    SurfaceListing* m_surfacelisting;
};
