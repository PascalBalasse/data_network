/**
 * @file ConnectorFactory.h
 * @brief Factory for creating data connectors
 *
 * Uses registration pattern for extensibility.
 * New connectors can be registered without modifying this file.
 */

#ifndef CONNECTOR_FACORY_H
#define CONNECTOR_FACORY_H_

#include "DataConnector.h"
#include "../core/enums.h"
#include <memory>
#include <functional>
#include <QMap>

namespace dn::connectors {

    class ConnectorFactory
    {
    public:
        using CreatorFunc = std::function<std::unique_ptr<DataConnector>(QObject* parent)>;

        // Register a connector type
        static void registerConnector(ConnectorType type, CreatorFunc creator);

        // Create a connector by type
        static std::unique_ptr<DataConnector> create(ConnectorType type, QObject* parent = nullptr);

    private:
        static QMap<ConnectorType, CreatorFunc>& getRegistry();
    };

}

#endif // CONNECTOR_FACORY_H_
