#pragma once

#include "symbolsfilter.h"
#include <redasm/redasm.h>

class StringsModel: public SymbolsFilterModel {
    Q_OBJECT

public:
    explicit StringsModel(RDContext* ctx, QObject* parent = nullptr);
    [[nodiscard]] RDAddress address(const QModelIndex& index) const override;

public:
    [[nodiscard]] QVariant data(const QModelIndex& index,
                                int role) const override;
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation,
                                      int role) const override;
    [[nodiscard]] int columnCount(const QModelIndex&) const override;
};
