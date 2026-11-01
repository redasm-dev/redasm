#pragma once

#include <QAbstractListModel>
#include <redasm/redasm.h>

class ReferencesToModel: public QAbstractListModel {
    Q_OBJECT

public:
    explicit ReferencesToModel(RDContext* ctx, RDAddress address,
                               QObject* parent = nullptr);
    [[nodiscard]] RDAddress address(const QModelIndex& index) const;
    [[nodiscard]] const RDXRef& xref(const QModelIndex& index) const;

public:
    [[nodiscard]] int rowCount(const QModelIndex&) const override;
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation,
                                      int role) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index,
                                int role) const override;

private:
    [[nodiscard]] int columnCount(const QModelIndex&) const override;

private:
    RDContext* m_context;
    RDAddress m_address;
    RDXRefSlice m_refs;
};
