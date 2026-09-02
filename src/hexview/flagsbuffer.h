#pragma once

#include <QHexView/model/buffer/qhexbuffer.h>
#include <redasm/redasm.h>

class FlagsBuffer: public QHexBuffer {
    Q_OBJECT

public:
    explicit FlagsBuffer(RDContext* ctx, const RDSegment* segment,
                         QObject* parent = nullptr);
    [[nodiscard]] const RDFlagsBuffer* flags() const;
    [[nodiscard]] quint64 base_address() const;

public:
    [[nodiscard]] qint64 length() const override;
    [[nodiscard]] bool accept(qint64 idx) const override;
    void insert(qint64 offset, const QByteArray& data) override;
    void remove(qint64 offset, int length) override;
    void replace(qint64 offset, const QByteArray& data) override;
    QByteArray read(qint64 offset, int length) override;
    bool read(QIODevice* iodevice) override;
    void write(QIODevice* iodevice) override;
    qint64 indexOf(const QByteArray& ba, qint64 from) override;
    qint64 lastIndexOf(const QByteArray& ba, qint64 from) override;

private:
    RDContext* m_context;
    const RDFlagsBuffer* m_flags;
    const RDSegment* m_segment;
};
