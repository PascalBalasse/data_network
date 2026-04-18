/**
 * @file ZipReader.cpp
 * @brief Implémentation du lecteur ZIP
 *
 * Lit le contenu de fichiers ZIP sans dépendance externe.
 * Supporte la décompression Deflate.
 *
 * @see ZipReader.h
 */

#include "ZipReader.h"
#include <QFile>
#include <QDebug>
#include <zlib.h>

/// Constructeur - ouvre et parse le fichier ZIP
ZipReader::ZipReader(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        m_errorString = "Cannot open file: " + filePath;
        return;
    }

    m_zipData = file.readAll();
    file.close();

    if (m_zipData.size() < 22) {
        m_errorString = "File too small to be a valid zip";
        return;
    }

    QByteArray sig = m_zipData.mid(0, 4);
    if (sig != "PK\x03\x04" && sig != "PK\x05\x06" && sig != "PK\x07\x08") {
        m_errorString = "Not a valid ZIP file";
        return;
    }

    m_valid = parse();
}

ZipReader::~ZipReader()
{
}

/// Retourne la liste des noms d'entrées
QStringList ZipReader::entryNames() const
{
    return m_entries.keys();
}

/// Retourne les données d'une entrée par son nom
QByteArray ZipReader::entryData(const QString& name) const
{
    if (!m_entries.contains(name)) {
        return QByteArray();
    }
    return m_entries[name].data;
}

/// Vérifie si une entrée existe par son nom
bool ZipReader::contains(const QString& name) const
{
    return m_entries.contains(name);
}

/// Parse le fichier ZIP et lit les entrées
bool ZipReader::parse()
{
    qDebug() << "ZipReader: Starting parse, file size:" << m_zipData.size();

    int eocdOffset = -1;
    for (int i = m_zipData.size() - 22; i >= 0 && i >= m_zipData.size() - 65557; --i) {
        if (m_zipData.mid(i, 4) == "PK\x05\x06") {
            eocdOffset = i;
            break;
        }
    }

    if (eocdOffset < 0) {
        m_errorString = "Cannot find End of Central Directory";
        qDebug() << "ZipReader: EOCD not found";
        return false;
    }

    qDebug() << "ZipReader: EOCD found at" << eocdOffset;

    quint16 numEntries16 = readUint16(m_zipData, eocdOffset + 10);
    quint32 cdOffset = readUint32(m_zipData, eocdOffset + 16);

    qDebug() << "ZipReader: numEntries:" << numEntries16 << "cdOffset:" << cdOffset;

    if (cdOffset >= static_cast<quint32>(m_zipData.size())) {
        m_errorString = "Central directory offset out of bounds";
        return false;
    }

    int pos = static_cast<int>(cdOffset);
    for (quint16 i = 0; i < numEntries16 && pos < eocdOffset && pos >= 0; ++i) {
        if (m_zipData.mid(pos, 4) != "PK\x01\x02") {
            qDebug() << "ZipReader: Central directory signature not found at pos" << pos;
            break;
        }

        ZipEntry entry;
        entry.compressionMethod = readUint16(m_zipData, pos + 10);
        entry.compressedSize = readUint32(m_zipData, pos + 20);
        entry.uncompressedSize = readUint32(m_zipData, pos + 24);
        quint16 nameLen = readUint16(m_zipData, pos + 28);
        quint16 extraLen = readUint16(m_zipData, pos + 30);
        quint16 commentLen = readUint16(m_zipData, pos + 32);
        entry.localHeaderOffset = readUint32(m_zipData, pos + 42);

        if (pos + 46 + nameLen > m_zipData.size()) {
            break;
        }

        entry.name = QString::fromUtf8(m_zipData.mid(pos + 46, nameLen));

        qDebug() << "ZipReader: Found entry:" << entry.name << "compMethod:" << entry.compressionMethod << "compSize:" << entry.compressedSize << "uncompSize:" << entry.uncompressedSize;

        pos += 46 + nameLen + extraLen + commentLen;

        if (!entry.name.endsWith('/')) {
            m_entries[entry.name] = entry;
        }
    }

    qDebug() << "ZipReader: Total entries:" << m_entries.size();

    for (auto it = m_entries.begin(); it != m_entries.end(); ++it) {
        ZipEntry& entry = it.value();

        int localPos = static_cast<int>(entry.localHeaderOffset);
        if (localPos < 0 || localPos + 30 > m_zipData.size()) {
            qDebug() << "ZipReader: Invalid local header for" << entry.name;
            entry.data.clear();
            continue;
        }

        quint16 localNameLen = readUint16(m_zipData, localPos + 26);
        quint16 localExtraLen = readUint16(m_zipData, localPos + 28);
        int dataStart = localPos + 30 + localNameLen + localExtraLen;

        if (dataStart < 0 || dataStart > m_zipData.size()) {
            entry.data.clear();
            continue;
        }

        QByteArray compressedData = m_zipData.mid(dataStart, entry.compressedSize);

        if (entry.compressionMethod == 0) {
            entry.data = compressedData;
            qDebug() << "ZipReader: Entry" << entry.name << "stored, size:" << entry.data.size();
        } else if (entry.compressionMethod == 8) {
            entry.data = decompressDeflate(compressedData, entry.uncompressedSize);
            qDebug() << "ZipReader: Entry" << entry.name << "deflated, result size:" << entry.data.size();
        } else {
            entry.data.clear();
            qDebug() << "ZipReader: Entry" << entry.name << "unknown compression method:" << entry.compressionMethod;
        }
    }

    return true;
}

/// Décompresse les données Deflate
QByteArray ZipReader::decompressDeflate(const QByteArray& data, quint32 expectedSize)
{
    if (data.isEmpty()) {
        return QByteArray();
    }

    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = data.size();
    strm.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(data.data()));

    // -MAX_WBITS for raw deflate (no header)
    int ret = inflateInit2(&strm, -MAX_WBITS);
    if (ret != Z_OK) {
        qWarning() << "inflateInit2 failed:" << ret;
        return QByteArray();
    }

    QByteArray result;
    if (expectedSize > 0) {
        result.reserve(expectedSize);
    }

    do {
        char buffer[16384];
        strm.avail_out = sizeof(buffer);
        strm.next_out = reinterpret_cast<Bytef*>(buffer);

        ret = inflate(&strm, Z_NO_FLUSH);

        if (ret != Z_OK && ret != Z_STREAM_END && ret != Z_BUF_ERROR) {
            qWarning() << "inflate failed:" << ret << (strm.msg ? strm.msg : "");
            inflateEnd(&strm);
            return QByteArray();
        }

        int have = sizeof(buffer) - strm.avail_out;
        result.append(buffer, have);

    } while (ret != Z_STREAM_END);

    inflateEnd(&strm);
    return result;
}

quint32 ZipReader::readUint32(const QByteArray& arr, int offset)
{
    if (offset < 0 || offset + 4 > arr.size()) return 0;
    const uchar* data = reinterpret_cast<const uchar*>(arr.constData());
    return static_cast<quint32>(data[offset]) |
           (static_cast<quint32>(data[offset + 1]) << 8) |
           (static_cast<quint32>(data[offset + 2]) << 16) |
           (static_cast<quint32>(data[offset + 3]) << 24);
}

quint16 ZipReader::readUint16(const QByteArray& arr, int offset)
{
    if (offset < 0 || offset + 2 > arr.size()) return 0;
    const uchar* data = reinterpret_cast<const uchar*>(arr.constData());
    return static_cast<quint16>(data[offset]) |
           (static_cast<quint16>(data[offset + 1]) << 8);
}
