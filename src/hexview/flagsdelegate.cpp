#include "flagsdelegate.h"
#include "support/themeprovider.h"

FlagsDelegate::FlagsDelegate(QObject* parent): QHexDelegate(parent) {}

void FlagsDelegate::set_flags_buffer(const RDFlagsBuffer* flags) {
    m_flags = flags;
}

QString FlagsDelegate::comment(quint64 offset, quint8 b,
                               const QHexView* hexview) const {
    Q_UNUSED(b);
    Q_UNUSED(hexview);

    if(!m_flags) return QString{};

    auto idx = static_cast<usize>(offset);

    if(rd_flagsbuffer_has_tail(m_flags, idx)) return QString{};

    static const struct {
        bool (*pred)(const RDFlagsBuffer*, usize);
        const char* name;
    } PREDS[] = {
        {rd_flagsbuffer_has_code, "CODE"},
        {rd_flagsbuffer_has_data, "DATA"},
        {rd_flagsbuffer_has_func, "FUNC"},
        {rd_flagsbuffer_has_call, "CALL"},
        {rd_flagsbuffer_has_jump, "JUMP"},
        {rd_flagsbuffer_has_cond, "COND"},
        {rd_flagsbuffer_has_dslot, "DSLOT"},
        {rd_flagsbuffer_has_flow, "FLOW"},
        {rd_flagsbuffer_has_noret, "NORET"},
        {rd_flagsbuffer_has_name, "NAME"},
        {rd_flagsbuffer_has_exported, "EXPORTED"},
        {rd_flagsbuffer_has_imported, "IMPORTED"},
    };

    QString s;

    for(const auto& p : PREDS) {
        if(!p.pred(m_flags, idx)) continue;
        if(!s.isEmpty()) s += " | ";
        s += p.name;
    }

    return s;
}

bool FlagsDelegate::renderByte(quint64 offset, quint8 b, QHexCharFormat& outcf,
                               const QHexView* hexview) const {
    Q_UNUSED(b);
    Q_UNUSED(hexview);

    if(!m_flags) return false;

    auto idx = static_cast<usize>(offset);

    if(rd_flagsbuffer_has_noret(m_flags, idx)) {
        outcf.foreground = theme_provider::color(RD_THEME_STOP);
        QColor bg = theme_provider::color(RD_THEME_STOP);
        bg.setAlpha(40);
        outcf.background = bg;
    }
    else if(rd_flagsbuffer_has_func(m_flags, idx)) {
        outcf.foreground = theme_provider::color(RD_THEME_FUNCTION);
        QColor bg = theme_provider::color(RD_THEME_FUNCTION);
        bg.setAlpha(40);
        outcf.background = bg;
    }
    else if(rd_flagsbuffer_has_code(m_flags, idx))
        outcf.foreground = theme_provider::color(RD_THEME_FLAG_CODE);
    else if(rd_flagsbuffer_has_data(m_flags, idx))
        outcf.foreground = theme_provider::color(RD_THEME_FLAG_DATA);
    else if(rd_flagsbuffer_has_tail(m_flags, idx))
        outcf.foreground = theme_provider::color(RD_THEME_MUTED);
    else
        return false;

    return true;
}
