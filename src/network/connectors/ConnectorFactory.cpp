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

using namespace dn::core;
using namespace dn::connectors;

std::unique_ptr<DataConnector> ConnectorFactory::create(ConnectorType type)
{
    return create(type, nullptr);
}

std::unique_ptr<DataConnector> ConnectorFactory::create(ConnectorType type, QObject* parent)
{
    switch (type) {
    case ConnectorType::CSV:
        return std::make_unique<CSVConnector>(parent);
    case ConnectorType::TXT:
        return std::make_unique<TXTConnector>(parent);
    case ConnectorType::EXCEL:
        return std::make_unique<ExcelConnector>(parent);
    case ConnectorType::PDF:
        return std::make_unique<PDFConnector>(parent);
    case ConnectorType::FEC:
        return std::make_unique<FECConnector>(parent);
    case ConnectorType::JSON:
        return std::make_unique<JSONConnector>(parent);
    case ConnectorType::SQL:
        return std::make_unique<SQLConnector>(parent);
    case ConnectorType::XML:
        return std::make_unique<XMLConnector>(parent);
    case ConnectorType::WEB:
        return std::make_unique<WebConnector>(parent);
    case ConnectorType::DATABASE:
        return std::make_unique<SQLConnector>(parent);
    case ConnectorType::FOLDER:
    default:
        qWarning() << "ConnectorFactory: unsupported connector type" << static_cast<int>(type);
        return nullptr;
    }
}