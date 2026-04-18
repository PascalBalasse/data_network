/**
 * @file NetworkSerializer.h
 * @brief Sérialisation du réseau de données (JSON)
 *
 * Gère la sauvegarde et le chargement de la configuration
 * du réseau de données au format JSON.
 *
 * Namespace: dn::serialization
 */

#ifndef NETWORKSERIALIZER_H
#define NETWORKSERIALIZER_H

#include <QJsonObject>
#include <QJsonArray>
#include <QHash>
#include <QUuid>
#include <QPointF>

namespace dn::network {
class Network;
class RuntimeNode;
enum class NodeType;
}

namespace dn::serialization {

    class NetworkSerializer
    {
    public:
        /// Données d'un nœud sérialisé
        struct NodeData {
            QUuid id;
            QString name;
            network::NodeType type;
            QJsonObject parameters;
            QPointF position;
        };

        /// Données d'une connexion sérialisée
        struct ConnectionData {
            QUuid fromId;
            QUuid toId;
            int slot;
        };

        ///Sérialise le réseau vers JSON
        static QJsonObject serialize(const dn::network::Network& network,
                                     const QHash<QUuid, QPointF>& positions);

        ///Désérialise depuis JSON
        static bool deserialize(const QJsonObject& json,
                                QVector<NodeData>& nodes,
                                QVector<ConnectionData>& connections,
                                QString& version);

        /// Valide la versión du fichier
        static bool validateVersion(const QString& version);

        /// Retourne la version actuelle
        static QString currentVersion() { return "1.0"; }

        /// Convertit une position en JSON
        static QJsonObject positionToJson(const QPointF& pos);

        /// Convertit JSON en position
        static QPointF positionFromJson(const QJsonObject& json);
    };

}


#endif // NETWORKSERIALIZER_H
