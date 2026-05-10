#pragma once

#include <QAbstractListModel>
#include <redasm/redasm.h>

class SegmentRegsModel: public QAbstractListModel {
    Q_OBJECT

public:
    explicit SegmentRegsModel(RDContext* ctx, QObject* parent = nullptr);
    [[nodiscard]] RDAddress address(const QModelIndex& index) const;
    void select_register(const char* regname);

public:
    [[nodiscard]] int rowCount(const QModelIndex&) const override;
    [[nodiscard]] int columnCount(const QModelIndex&) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index,
                                int role) const override;
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation,
                                      int role) const override;

private:
    RDContext* m_context;
    RDSegmentRegSlice m_registers{};
};
