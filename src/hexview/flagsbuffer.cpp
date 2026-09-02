#include "flagsbuffer.h"

FlagsBuffer::FlagsBuffer(RDContext* ctx, const RDSegment* segment,
                         QObject* parent)
    : QHexBuffer{parent}, m_context{ctx}, m_segment{segment} {
    m_flags = rd_segment_get_flags(segment);
}

const RDFlagsBuffer* FlagsBuffer::flags() const { return m_flags; }
quint64 FlagsBuffer::base_address() const { return m_segment->start_address; }

qint64 FlagsBuffer::length() const {
    return static_cast<qint64>(rd_flagsbuffer_get_length(m_flags));
}

bool FlagsBuffer::accept(qint64 idx) const {
    return idx < this->length() &&
           rd_flagsbuffer_get_value(m_flags, static_cast<usize>(idx), nullptr);
}

void FlagsBuffer::insert(qint64 offset, const QByteArray& data) {
    this->replace(offset, data); // we cannot change the size of mapped data
}

void FlagsBuffer::remove(qint64 offset, int length) {
    RDAddress address = this->base_address() + offset;

    // same as insertion: we cannot change the size of mapped data
    // deletion means zero-filling
    QByteArray zero{length, 0};
    rd_patch(m_context, address, zero.constData(), zero.size());
}

void FlagsBuffer::replace(qint64 offset, const QByteArray& data) {
    RDAddress address = this->base_address() + offset;
    rd_patch(m_context, address, data.constData(), data.size());
}

QByteArray FlagsBuffer::read(qint64 offset, int length) {
    QByteArray data;

    for(qint64 i = 0; i < qMin<qint64>(length, this->length()); i++) {
        u8 b;

        if(rd_flagsbuffer_get_value(m_flags, static_cast<usize>(offset + i),
                                    &b))
            data.push_back(static_cast<char>(b));
        else
            data.push_back(u8{0});
    }

    return data;
}

bool FlagsBuffer::read(QIODevice* /*iodevice*/) { return false; }

void FlagsBuffer::write(QIODevice* /*iodevice*/) {}

qint64 FlagsBuffer::indexOf(const QByteArray& /*ba*/, qint64 /*from*/) {
    return -1;
}

qint64 FlagsBuffer::lastIndexOf(const QByteArray& /*ba*/, qint64 /*from*/) {
    return -1;
}
