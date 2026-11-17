#include "loaderoptions.h"
#include <QFont>
#include <algorithm>

namespace {
constexpr quintptr TOPLEVEL_GROUP = ~quintptr{0};
}

LoaderOptionsModel::LoaderOptionsModel(QObject* parent)
    : QAbstractItemModel{parent} {}

void LoaderOptionsModel::set_test_result(RDTestResult* tr) {
    this->beginResetModel();

    m_testresult = tr;
    m_options = rd_testresult_get_options(tr);
    m_nodes.clear();
    m_grouped = false;

    for(int i = 0; i < static_cast<int>(rd_slice_length(m_options)); i++) {
        const RDLoaderOption& opt = rd_slice_at(m_options, i);
        QString g = opt.group ? QString::fromUtf8(opt.group) : QString{};
        if(opt.group) m_grouped = true;

        auto it = std::find_if(m_nodes.begin(), m_nodes.end(),
                               [&](const Node& n) { return n.name == g; });

        if(it == m_nodes.end()) {
            m_nodes.push_back(Node{g, {i}});
            continue;
        }

        it->opts.push_back(i);
    }

    this->endResetModel();
}

void LoaderOptionsModel::reset_default() {
    if(!m_testresult) return;

    const RDLoaderOption* opt;
    rd_slice_each(opt, m_options)
        rd_testresult_set_option_bool(m_testresult, opt->id, opt->defvalue);

    // refresh items
    static const QVector<int> ROLES = {Qt::CheckStateRole, Qt::FontRole};

    if(!m_grouped) {
        int n = static_cast<int>(rd_slice_length(m_options));
        if(n)
            emit dataChanged(this->index(0, 0, {}), this->index(n - 1, 1, {}),
                             ROLES);
        return;
    }

    for(int g = 0; g < m_nodes.size(); g++) {
        QModelIndex parent = this->index(g, 0, {});
        int n = m_nodes.at(g).opts.size();

        if(n)
            emit dataChanged(this->index(0, 0, parent),
                             this->index(n - 1, 1, parent), ROLES);
    }
}

QVariant LoaderOptionsModel::headerData(int section,
                                        Qt::Orientation orientation,
                                        int role) const {
    if(orientation == Qt::Vertical || role != Qt::DisplayRole) return {};

    switch(section) {
        case 0: return "Name";
        case 1: return "Description";
        default: break;
    }

    return {};
}

int LoaderOptionsModel::columnCount(const QModelIndex&) const { return 2; }

bool LoaderOptionsModel::setData(const QModelIndex& index,
                                 const QVariant& value, int role) {
    if(role != Qt::CheckStateRole) return false;

    const RDLoaderOption* opt = this->option_at(index);
    if(!opt) return false;

    bool checked = value.value<Qt::CheckState>() == Qt::Checked;
    if(!rd_testresult_set_option_bool(m_testresult, opt->id, checked))
        return false;

    emit dataChanged(index, index.siblingAtColumn(1),
                     {Qt::CheckStateRole, Qt::FontRole});
    return true;
}

QVariant LoaderOptionsModel::data(const QModelIndex& index, int role) const {
    const RDLoaderOption* opt = this->option_at(index);

    if(!opt) { // a group row
        if(role == Qt::DisplayRole && index.column() == 0)
            return m_nodes.at(index.row()).name;

        if(role == Qt::FontRole && index.column() == 0) {
            QFont font;
            font.setBold(true);
            return font;
        }

        return {};
    }

    switch(role) {
        case Qt::DisplayRole:
            if(index.column() == 0) return QString::fromUtf8(opt->name);
            if(index.column() == 1)
                return opt->desc ? QString::fromUtf8(opt->desc) : QString{};
            break;

        case Qt::CheckStateRole:
            if(index.column() == 0)
                return opt->value ? Qt::Checked : Qt::Unchecked;
            break;

        case Qt::FontRole:
            // read live from the slice, so this stays right after setData
            if(index.column() == 0 && opt->value != opt->defvalue) {
                QFont font;
                font.setItalic(true);
                return font;
            }
            break;

        default: break;
    }

    return {};
}

int LoaderOptionsModel::rowCount(const QModelIndex& parent) const {
    if(!m_grouped)
        return parent.isValid() ? 0
                                : static_cast<int>(rd_slice_length(m_options));

    if(!parent.isValid()) return m_nodes.size();

    if(parent.internalId() != TOPLEVEL_GROUP)
        return 0; // options have no children

    return m_nodes.at(parent.row()).opts.size();
}

Qt::ItemFlags LoaderOptionsModel::flags(const QModelIndex& index) const {
    if(!index.isValid()) return Qt::NoItemFlags;

    // group rows are headings: selectable but not checkable
    if(!this->option_at(index)) return Qt::ItemIsEnabled;

    Qt::ItemFlags f = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if(index.column() == 0) f |= Qt::ItemIsUserCheckable;
    return f;
}

QModelIndex LoaderOptionsModel::index(int row, int col,
                                      const QModelIndex& parent) const {
    if(!this->hasIndex(row, col, parent)) return {};

    if(!m_grouped) return this->createIndex(row, col, TOPLEVEL_GROUP);
    if(!parent.isValid()) return this->createIndex(row, col, TOPLEVEL_GROUP);

    return this->createIndex(row, col, static_cast<quintptr>(parent.row()));
}

QModelIndex LoaderOptionsModel::parent(const QModelIndex& index) const {
    if(!index.isValid() || index.internalId() == TOPLEVEL_GROUP) return {};

    return this->createIndex(static_cast<int>(index.internalId()), 0,
                             TOPLEVEL_GROUP);
}

const RDLoaderOption*
LoaderOptionsModel::option_at(const QModelIndex& index) const {
    if(!index.isValid()) return nullptr;

    // flat: every row is an option
    if(!m_grouped) {
        if(index.row() >= static_cast<int>(rd_slice_length(m_options)))
            return nullptr;

        return &rd_slice_at(m_options, index.row());
    }

    if(index.internalId() == TOPLEVEL_GROUP) return nullptr; // a group row

    const Node& n = m_nodes.at(static_cast<int>(index.internalId()));
    if(index.row() >= n.opts.size()) return nullptr;

    return &rd_slice_at(m_options, n.opts.at(index.row()));
}
