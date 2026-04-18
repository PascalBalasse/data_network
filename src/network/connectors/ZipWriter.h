/**
 * @file ZipWriter.h
 * @brief Écrivain de fichiers ZIP (utilisé pour Excel .xlsx)
 *
 * Crée des fichiers ZIP sans dépendance externe.
 * Supporte la compression Deflate.
 * Utilisé par ExcelConnector pour écrire les fichiers .xlsx.
 */

#ifndef ZIPWRITER_H
#define ZIPWRITER_H

#include <QString>
#include <QByteArray>
#include <QMap>
#include <QVector>
#include <QFile>
#include <QDataStream>
#include <QBuffer>

class ZipWriter {
public:
    explicit ZipWriter(const QString& filePath);
    ~ZipWriter();

    /// @brief Retourne true si prêt pour l'écriture
    bool isValid() const { return m_valid; }

    /// @brief Retourne le message d'erreur
    QString errorString() const { return m_errorString; }

    /// @brief Ajoute une entrée au ZIP
    void addEntry(const QString& name, const QByteArray& data);

    /// @brief Écrit le fichier ZIP sur le disque
    bool write();

private:
    struct ZipEntryInfo {
        QString name;
        QByteArray data;
        quint32 crc32;
        quint32 compressedSize;
        quint32 uncompressedSize;
        quint32 localHeaderOffset;
    };

    QByteArray compressData(const QByteArray& data);
    quint32 calculateCRC32(const QByteArray& data);
    void writeUInt32(QByteArray& arr, quint32 value);
    void writeUInt16(QByteArray& arr, quint16 value);

    QString m_filePath;
    QVector<ZipEntryInfo> m_entries;
    bool m_valid = true;
    QString m_errorString;
};

#endif
