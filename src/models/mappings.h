#pragma once

#include <QAbstractListModel>
#include <redasm/redasm.h>

class MappingsModel: public QAbstractListModel {
    Q_OBJECT

public:
    explicit MappingsModel(const RDContext* ctx, QObject* parent = nullptr);
    [[nodiscard]] RDAddress address(const QModelIndex& index) const;

public:
    [[nodiscard]] QVariant data(const QModelIndex& index,
                                int role) const override;
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation,
                                      int role) const override;
    [[nodiscard]] int columnCount(const QModelIndex&) const override;
    [[nodiscard]] int rowCount(const QModelIndex&) const override;

private:
    RDInputMappingSlice m_mappings;
};
