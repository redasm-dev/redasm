#pragma once

#include "views/surface/listing.h"
#include "views/surface/path.h"
#include <QSplitter>

class SurfaceView: public QSplitter {
    Q_OBJECT

public:
    explicit SurfaceView(RDContext* ctx, QWidget* parent = nullptr);
    SurfaceListing* listing() { return m_surfacelisting; }

Q_SIGNALS:
    void history_updated();
    void switch_view();

private:
    SurfacePath* m_surfacepath;
    SurfaceListing* m_surfacelisting;
};
