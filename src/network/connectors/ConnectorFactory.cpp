#include "ConnectorFactory.h"
#include "CSVConnector.h"
#include "TXTConnector.h"
#include "ExcelConnector.h"
#include "PDFConnector.h"
#include "FECConnector.h"
#include "JSONConnector.h"
#include "SQLConnector.h"
#include "XMLConnector.h"
#include "WebConnector.h"

using namespace dn::connectors;

// Static registry storage
QMap<ConnectorType, ConnectorFactory::CreatorFunc>& ConnectorFactory::getRegistry()
{
    static QMap<ConnectorType, CreatorFunc> registry;
    return registry;
}

void ConnectorFactory::registerConnector(ConnectorType type, CreatorFunc creator)
{
    getRegistry()[type] = creator;
}

std::unique_ptr<DataConnector> ConnectorFactory::create(ConnectorType type, QObject* parent)
{
    auto& registry = getRegistry();
    
    // If registry is empty, initialize with defaults
    if (registry.isEmpty()) {
        registry[ConnectorType::CSV] = [](QObject* p) { return std::make_unique<CSVConnector>(p); };
        registry[ConnectorType::TXT] = [](QObject* p) { return std::make_unique<TXTConnector>(p); };
        registry[ConnectorType::EXCEL] = [](QObject* p) { return std::make_unique<ExcelConnector>(p); };
        registry[ConnectorType::PDF] = [](QObject* p) { return std::make_unique<PDFConnector>(p); };
        registry[ConnectorType::FEC] = [](QObject* p) { return std::make_unique<FECConnector>(p); };
        registry[ConnectorType::JSON] = [](QObject* p) { return std::make_unique<JSONConnector>(p); };
        registry[ConnectorType::SQL] = [](QObject* p) { return std::make_unique<SQLConnector>(p); };
        registry[ConnectorType::XML] = [](QObject* p) { return std::make_unique<XMLConnector>(p); };
        registry[ConnectorType::WEB] = [](QObject* p) { return std::make_unique<WebConnector>(p); };
    }
    
    auto it = registry.find(type);
    if (it != registry.end()) {
        return it.value()(parent);
    }
    
    qWarning() << "ConnectorFactory: unsupported connector type" << static_cast<int>(type);
    return nullptr;
}
