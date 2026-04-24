#ifndef CONNECTOR_FACTORY_H
#define CONNECTOR_FACTORY_H

#include "DataConnector.h"
#include "../core/enums.h"
#include <memory>

namespace dn::connectors {

class ConnectorFactory
{
public:
    static std::unique_ptr<DataConnector> create(dn::core::ConnectorType type);
    static std::unique_ptr<DataConnector> create(dn::core::ConnectorType type, QObject* parent);
};

}

#endif // CONNECTOR_FACTORY_H