#pragma once

#include "support/changetracker.h"
#include "support/utils.h"
#include <QAbstractListModel>
#include <redasm/redasm.h>

class ReferencesFromModel: public QAbstractListModel {
    Q_OBJECT

public:
    using ChangeType = ChangeTracker<RDXRef, utils::XRefEqual>;
    using Change = ChangeType::Change;

public:
    explicit ReferencesFromModel(RDContext* ctx, RDAddress address,
                                 QObject* parent = nullptr);
    [[nodiscard]] RDAddress address(const QModelIndex& index) const;
    [[nodiscard]] RDXRef xref(const QModelIndex& index) const;
    [[nodiscard]] QList<Change> changes() const { return m_refs.changes(); }

public:
    [[nodiscard]] int rowCount(const QModelIndex&) const override;
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation,
                                      int role) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index,
                                int role) const override;
    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool setData(const QModelIndex& index, const QVariant& value,
                 int role) override;
    bool insertRows(int row, int count, const QModelIndex& parent) override;
    bool removeRows(int row, int count, const QModelIndex& parent) override;

private:
    [[nodiscard]] int columnCount(const QModelIndex&) const override;

private:
    RDContext* m_context;
    RDAddress m_address;
    ChangeType m_refs;
};
