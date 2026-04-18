/**
 * @file ZipReader.h
 * @brief Lecteur de fichiers ZIP (utilisé pour Excel .xlsx)
 *
 * Lit le contenu de fichiers ZIP sans dépendance externe.
 * Supporte la décompression Deflate.
 * Utilisé par ExcelConnector pour lire les fichiers .xlsx.
 */

#ifndef ZIPREADER_H
#define ZIPREADER_H

#include <QString>
#include <QByteArray>
#include <QMap>
#include <QVector>

class ZipEntry {
public:
    QString name;
    QByteArray data;
    quint32 compressedSize;
    quint32 uncompressedSize;
    quint16 compressionMethod;
    quint32 localHeaderOffset;
};

class ZipReader {
public:
    explicit ZipReader(const QString& filePath);
    ~ZipReader();

    /// @brief Retourne true si le fichier est valide
    bool isValid() const { return m_valid; }

    /// @brief Retourne le message d'erreur
    QString errorString() const { return m_errorString; }

    /// @brief Retourne la liste des noms d'entrées
    QStringList entryNames() const;

    /// @brief Retourne les données d'une entrée par son nom
    QByteArray entryData(const QString& name) const;

    /// @brief Vérifie si une entrée existe
    bool contains(const QString& name) const;

private:
    bool parse();
    QByteArray decompressDeflate(const QByteArray& data, quint32 expectedSize);
    bool readLocalFileHeader(const ZipEntry& entry, QByteArray& data);
    quint32 readUint32(const QByteArray& arr, int offset);
    quint16 readUint16(const QByteArray& arr, int offset);

    QMap<QString, ZipEntry> m_entries;
    QByteArray m_zipData;
    bool m_valid = false;
    QString m_errorString;
};

#endif
