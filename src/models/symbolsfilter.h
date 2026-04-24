#pragma once

#include "symbols.h"
#include <QSortFilterProxyModel>
#include <redasm/redasm.h>

class SymbolsModel;

class SymbolsFilterModel: public QSortFilterProxyModel {
    Q_OBJECT

public:
    explicit SymbolsFilterModel(RDContext* ctx, bool autoalign = true,
                                QObject* parent = nullptr);
    explicit SymbolsFilterModel(RDContext* ctx, usize filter,
                                bool autoalign = true,
                                QObject* parent = nullptr);
    [[nodiscard]] RDAddress address(const QModelIndex& index) const;
    void resync();

    void set_type_filter(usize s) {
        m_typefilter = s;
        this->invalidate();
    }

    [[nodiscard]] const SymbolsModel* symbols_model() const {
        return qobject_cast<const SymbolsModel*>(this->sourceModel());
    }

    SymbolsModel* symbols_model() {
        return qobject_cast<SymbolsModel*>(this->sourceModel());
    }

protected:
    [[nodiscard]] bool filterAcceptsRow(int source_row,
                                        const QModelIndex&) const override;

private:
    usize m_typefilter{RD_SYMBOL_INVALID};
};
