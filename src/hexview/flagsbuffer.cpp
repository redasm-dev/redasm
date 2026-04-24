#include "flagsbuffer.h"

FlagsBuffer::FlagsBuffer(const RDSegment* segment, QObject* parent)
    : QHexBuffer{parent}, m_segment{segment} {
    m_flags = rd_segment_get_flags(segment);
}

quint64 FlagsBuffer::base_address() const { return m_segment->start_address; }

qint64 FlagsBuffer::length() const {
    return rd_flagsbuffer_get_length(m_flags);
}

bool FlagsBuffer::accept(qint64 idx) const {
    return idx < this->length() &&
           rd_flagsbuffer_get_value(m_flags, idx, nullptr);
}

void FlagsBuffer::insert(qint64 /*offset*/, const QByteArray& /*data*/) {}

void FlagsBuffer::remove(qint64 /*offset*/, int /*length*/) {}

QByteArray FlagsBuffer::read(qint64 offset, int length) {
    QByteArray data;

    for(qint64 i = 0; i < qMin<qint64>(length, this->length()); i++) {
        u8 b;

        if(rd_flagsbuffer_get_value(m_flags, offset + i, &b))
            data.push_back(b);
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
