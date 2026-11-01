#pragma once

#include <QAbstractListModel>
#include <redasm/redasm.h>

class TypedefsModel: public QAbstractListModel {
    Q_OBJECT

public:
    explicit TypedefsModel(RDContext* ctx, QObject* parent = nullptr);
    [[nodiscard]] const RDTypeDef* type_def(const QModelIndex& index) const;

public:
    [[nodiscard]] QVariant data(const QModelIndex& index,
                                int role) const override;
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation,
                                      int role) const override;
    [[nodiscard]] int rowCount(const QModelIndex&) const override;

private:
    [[nodiscard]] int columnCount(const QModelIndex&) const override;

private:
    RDContext* m_context;
    RDTypeDefSlice m_typedefs;
};
