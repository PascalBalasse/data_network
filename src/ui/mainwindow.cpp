#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "models/DataTableModel.h"
#include "dialogs/customcsvdialog.h"
#include "dialogs/customexceldialog.h"
#include "dialogs/customfilterdialog.h"
#include "dialogs/customselectcolumnsdialog.h"
#include "dialogs/customfecdialog.h"
#include "dialogs/customsqldialog.h"
#include "dialogs/customtxtdialog.h"
#include "dialogs/customjsondialog.h"
#include "dialogs/customxmldialog.h"
#include "dialogs/customwebdialog.h"
#include "dialogs/customcalculatedcolumndialog.h"
#include "dialogs/custompdfdialog.h"

#include "../network/connectors/CSVConnector.h"
#include "../network/connectors/TXTConnector.h"
#include "../network/connectors/ExcelConnector.h"
#include "../network/connectors/PDFConnector.h"
#include "../network/connectors/FECConnector.h"
#include "../network/connectors/JSONConnector.h"
#include "../network/connectors/SQLConnector.h"
#include "../network/connectors/XMLConnector.h"
#include "../network/connectors/WebConnector.h"
#include "../network/transformations/FilterTransformation.h"
#include "../network/transformations/SelectColumnsTransformation.h"
#include "../network/transformations/RenameColumnsTransformation.h"
#include "../network/transformations/CalculatedColumnTransformation.h"

#include <QInputDialog>
#include <QFileDialog>
#include <QSaveFile>
#include <QMessageBox>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QDebug>
#include <QUuid>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QMenu>
#include <QHeaderView>

using namespace dn::ui;
using namespace dn::dialogs;
using namespace dn::connectors;
using namespace dn::core;
using namespace dn::network;
using namespace dn::transformations;

//========================= CONSTRUCTEUR ET DESTRUCTEUR ======================================//

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_selectedNode(nullptr)
{
    ui->setupUi(this); // construction de l'interface à partir du fichier .ui

    showMaximized();

    QList<int> sizes = {width() / 2, width() / 2};
    ui->splitter->setSizes(sizes);

    setupConnections();

    //instanciation du model pour la TableView
    m_tableModel = std::make_unique<DataTableModel>(this);
    ui->tableView->setModel(m_tableModel.get());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupConnections(){

    // connection des signaux des menus aux slots
    connect(ui->actionNouveau_r_seau, &QAction::triggered, this, &MainWindow::onNewNetwork);
    connect(ui->actionOuvrir_un_r_seau, &QAction::triggered, this, &MainWindow::onLoadNetwork);
    connect(ui->actionEnregistrer,&QAction::triggered,this,&MainWindow::onSaveNetwork);
    connect(ui->actionEnregistrer_sous,&QAction::triggered,this,&MainWindow::onSaveNetworkAs);


    connect(ui->tableView->selectionModel(), &QItemSelectionModel::currentChanged,
           this, &MainWindow::onTableRowSelected);
    connect(ui->tableView, &dn::ui::TableView::renameColumnsRequested,
            this, &MainWindow::onRenameColumns);

    // Connexion des signaux émis par le réseau
    connect(&m_network, &Network::nodeAdded, this, &MainWindow::onNodeAdded);
    connect(&m_network, &Network::nodeRemoved, this, &MainWindow::onNodeRemoved);
    connect(&m_network, &Network::networkChanged, this, &MainWindow::onNetworkChanged);

    // Connection à une demande de source de données
    connect(ui->graphView, &GraphView::sourceNodeRequested, this, [this](const dn::core::ConnectorType& type){
        switch (type) {
            case dn::core::ConnectorType::CSV:
                onLoadCSVData();
                break;
            case dn::core::ConnectorType::TXT:
                onLoadTXTData();
                break;
            case dn::core::ConnectorType::EXCEL:
                onLoadExcelData();
                break;
            case dn::core::ConnectorType::PDF:
                onLoadPDFData();
                break;
            case dn::core::ConnectorType::FEC:
                onLoadFECData();
                break;
            case dn::core::ConnectorType::SQL:
                onLoadSQLData();
                break;
            case dn::core::ConnectorType::JSON:
                onLoadJSONData();
                break;
            case dn::core::ConnectorType::XML:
                onLoadXMLData();
                break;
            case dn::core::ConnectorType::WEB:
                onLoadWebData();
                break;
            default:
                break;
            }
    });

    // connection à une demande de transformation
    connect(ui->graphView, &GraphView::transformationRequested, this, [this](const dn::core::TransformationType& type){
        switch (type) {
        case dn::core::TransformationType::Filter:
            onAddFilter();
            break;
        case dn::core::TransformationType::SelectColumns:
            onSelectColumns();
            break;
        case dn::core::TransformationType::CalculatedColumn:
            onAddCalculatedColumn();
            break;

        default:
            break;
        }
    });

    // Connection à une demande de cible de données
    connect(ui->graphView, &GraphView::targetNodeRequested, this, [this](const dn::core::ConnectorType& type){
        switch (type) {
        case dn::core::ConnectorType::CSV:
            onSaveCSVData();
            break;
        case dn::core::ConnectorType::TXT:
            onSaveTXTData();
            break;
        case dn::core::ConnectorType::EXCEL:
            onSaveExcelData();
            break;
        case dn::core::ConnectorType::XML:
            onSaveXMLData();
            break;
        default:
            break;
        }
    });

    connect(ui->graphView, &GraphView::nodeSelected,
            this, &MainWindow::onGraphNodeSelected);
    connect(ui->graphView, &GraphView::clearNetworkRequested,
            this, &MainWindow::onClearNetwork);
    // Actions sur les nœuds
    connect(ui->graphView, &GraphView::deleteNodeRequested,
            this, &MainWindow::onDeleteNode);
    connect(ui->graphView, &GraphView::renameNodeRequested,
            this, &MainWindow::onRenameNode);
    connect(ui->graphView, &GraphView::duplicateNodeRequested,
            this, &MainWindow::onDuplicateNode);
    connect(ui->graphView, &GraphView::editNodeRequested,
            this, &MainWindow::onEditNode);
    connect(ui->graphView, &GraphView::computeFromNodeRequested,
            this, &MainWindow::onComputeFromNode);
}

void MainWindow::setUiEnabled(bool enabled)
{
    // Changer le curseur pour indiquer l'activité
    if (!enabled) {
        QApplication::setOverrideCursor(Qt::WaitCursor);
    } else {
        QApplication::restoreOverrideCursor();
    }
}

//========================= SEQUENCEMENT DES OPERATIONS ======================================//

// ajouter l'opération dans la file d'attente
void MainWindow::queueOperation(NetworkOperation operation)
{
    m_operationQueue.enqueue(operation);
    processNextOperation();
}

void MainWindow::processNextOperation()
{
    if (m_isProcessing || m_operationQueue.isEmpty())
        return;

    m_isProcessing = true;
    setUiEnabled(false);

    // Prendre la prochaine opération
    NetworkOperation operation = m_operationQueue.dequeue();

    // Exécuter l'opération
    operation();

    // L'opération doit appeler operationCompleted() quand elle est terminée
}


// ========== SLOTS L ==========

void MainWindow::onLoadCSVData()
{
    queueOperation([this]() { doReadCSV(); });
}

void MainWindow::onLoadExcelData()
{
    queueOperation([this]() { doReadExcel(); });
}

void MainWindow::onLoadPDFData()
{
    queueOperation([this]() { doReadPDF(); });
}

void MainWindow::onLoadFECData()
{
    queueOperation([this]() { doReadFEC(); });
}

void MainWindow::onLoadSQLData()
{
    queueOperation([this]() { doReadSQL(); });
}

void MainWindow::onLoadJSONData()
{
    queueOperation([this]() { doReadJSON(); });
}

void MainWindow::onLoadXMLData()
{
    queueOperation([this]() { doReadXML(); });
}

void MainWindow::onSaveXMLData()
{
    queueOperation([this]() { doWriteXML(); });
}

void MainWindow::onLoadWebData()
{
    queueOperation([this]() { doReadWeb(); });
}

void MainWindow::onSaveCSVData()
{
    queueOperation([this]() { doWriteCSV(); });
}

void MainWindow::onLoadTXTData()
{
    queueOperation([this]() { doReadTXT(); });
}

void MainWindow::onSaveTXTData()
{
    queueOperation([this]() { doWriteTXT(); });
}

void MainWindow::onSaveExcelData()
{
    queueOperation([this]() { doWriteExcel(); });
}

void MainWindow::onAddFilter()
{
    queueOperation([this]() { doAddFilter(); });
}

void MainWindow::onSelectColumns()
{
    queueOperation([this]() { doSelectColumns(); });
}

void MainWindow::onAddCalculatedColumn()
{
    queueOperation([this]() { doAddCalculatedColumn(); });
}

void MainWindow::onRenameColumns()
{
     queueOperation([this]() { doRenameColumns(); });
}

// effacer le graphique
void MainWindow::onClearNetwork(){
    queueOperation([this]() { doClearNetwork(); });
}


// ========== SLOTS DES SIGNAUX DU RESEAU ==========

void MainWindow::onNodeTableChanged(const dn::core::DataTable& table)
{
    auto* node = qobject_cast<dn::nodes::RuntimeNode*>(sender());
    if (node && node == m_selectedNode) {
        m_tableModel->setTable(&table);
    }
}

void MainWindow::onNodeAdded(RuntimeNode* node)
{
    // Ajouter au graphe visuel
    ui->graphView->addNode(node->getId(), node->getNodeType(), node->getName());
    ui->graphView->selectNewNode(node->getId());
    ui->graphView->requestLayout();
    // Connecter le signal de sélection du GraphView

}

void MainWindow::onGraphNodeSelected(const QUuid& nodeId)
{
    RuntimeNode* node = m_network.findNode(nodeId);
    if (node) {
        m_selectedNode = node;
        m_tableModel->setTable(&node->getCachedResult());
        statusBar()->showMessage(QString("Nœud sélectionné: %1 (%2 lignes, %3 colonnes)")
                                     .arg(node->getName())
                                     .arg(node->getCachedResult().rowCount())
                                     .arg(node->getCachedResult().columnCount()), 3000);
    }
}

void MainWindow::onNodeRemoved(RuntimeNode* node)
{
    qDebug() << "Node removed:" << node->getName();

    // Mettre à jour l'affichage
    if (m_selectedNode == node) {
        m_selectedNode = nullptr;
        m_tableModel->setTable(nullptr);
    }
}

void MainWindow::AddConnection(RuntimeNode* source, RuntimeNode* target)
{
    // AJOUTER CETTE LIGNE pour dessiner la connexion
    ui->graphView->addConnection(source->getId(), target->getId());
    ui->graphView->requestLayout();
}

void MainWindow::onNetworkChanged()
{
    qDebug() << "Network changed";
    // Mettre à jour l'interface si nécessaire
}

void MainWindow::onTableRowSelected(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous)

    if (!current.isValid() || !m_selectedNode)
        return;

    // Afficher la valeur sélectionnée dans la status bar
    QVariant value = m_tableModel->data(current, Qt::DisplayRole);
    statusBar()->showMessage(QString("Valeur: %1").arg(value.toString()), 2000);
}


QList<int> MainWindow::selectedTableColumns() const
{
    QList<int> columns;
    if (!ui->tableView->selectionModel())
        return columns;

    const QModelIndexList selectedIndexes = ui->tableView->selectionModel()->selectedColumns();
    for (const QModelIndex& index : selectedIndexes) {
        if (index.isValid()) {
            columns.append(index.column());
        }
    }
    return columns;
}


// Dialogue pour Excel (à implémenter)

// Dialogue pour JSON
QMap<QString, QVariant> MainWindow::showJSONDialog()
{
    return CustomJSONDialog::getJSONWriteParameters(this);
}

// Dialogue pour XML
QMap<QString, QVariant> MainWindow::showXMLReadDialog()
{
    return CustomXMLDialog::getXMLReadParameters(this);
}

QMap<QString, QVariant> MainWindow::showXMLWriteDialog()
{
    return CustomXMLDialog::getXMLWriteParameters(this);
}

QMap<QString, QVariant> MainWindow::showWebReadDialog()
{
    return CustomWebDialog::getWebReadParameters(this);
}


void MainWindow::onExportCompleted(bool success, const QString& message)
{
    if (success) {
        statusBar()->showMessage(message, 3000);
        QMessageBox::information(this, "Export", message);
    } else {
        QMessageBox::warning(this, "Erreur d'export", message);
    }
}

QMap<QString, QVariant> MainWindow::showCSVReadDialog()
{
    return CustomCSVDialog::getCSVReadParameters(this);
}

QMap<QString, QVariant> MainWindow::showCSVWriteDialog()
{
    return CustomCSVDialog::getCSVWriteParameters(this);
}

QMap<QString, QVariant> MainWindow::showTXTReadDialog()
{
    return CustomTXTDialog::getTXTReadParameters(this);
}

QMap<QString, QVariant> MainWindow::showTXTWriteDialog()
{
    return CustomTXTDialog::getTXTWriteParameters(this);
}

QMap<QString, QVariant> MainWindow::showExcelReadDialog()
{
    return CustomExcelDialog::getExcelReadParameters(this);
}

QMap<QString, QVariant> MainWindow::showExcelWriteDialog()
{
    return CustomExcelDialog::getExcelWriteParameters(this);
}

QMap<QString, QVariant> MainWindow::showPDFReadDialog()
{
    return CustomPDFDialog::getPDFReadParameters(this);
}


// ========== GESTION DU RÉSEAU ==========


void MainWindow::onNewNetwork()
{
    // Vérifier s'il faut sauvegarder les changements
    if (!confirmDiscardChanges()) {
        return;
    }

    // Nettoyer le réseau actuel
    doClearNetwork();

    // Réinitialiser le fichier courant
    m_currentNetworkFile.clear();
    updateWindowTitle();

    statusBar()->showMessage("Nouveau réseau créé", 2000);
}

void MainWindow::onLoadNetwork()
{
    // Vérifier s'il faut sauvegarder les changements
    if (!confirmDiscardChanges()) {
        return;
    }

    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Ouvrir un réseau",
                                                    QString(),
                                                    "Data Network Files (*.dnet);;JSON Files (*.json);;All Files (*)");

    if (fileName.isEmpty()) {
        return;
    }

    if (loadNetworkFromFile(fileName)) {
        m_currentNetworkFile = fileName;
        updateWindowTitle();
        statusBar()->showMessage(QString("Réseau chargé: %1").arg(fileName), 3000);
    }
}

void MainWindow::onSaveNetwork()
{
    if (m_currentNetworkFile.isEmpty()) {
        onSaveNetworkAs();
    } else {
        saveNetworkToFile(m_currentNetworkFile);
    }
}

void MainWindow::onSaveNetworkAs()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "Sauvegarder le réseau",
                                                    QString(),
                                                    "Data Network Files (*.dnet);;JSON Files (*.json)");

    if (fileName.isEmpty()) {
        return;
    }

    // Ajouter l'extension si nécessaire
    if (!fileName.endsWith(".dnet") && !fileName.endsWith(".json")) {
        fileName += ".dnet";
    }

    if (saveNetworkToFile(fileName)) {
        m_currentNetworkFile = fileName;
        updateWindowTitle();
        statusBar()->showMessage(QString("Réseau sauvegardé: %1").arg(fileName), 3000);
    }
}

// Supprimer tous mes noeuds du réseau
void MainWindow::doClearNetwork()
{
    // Supprimer tous les nœuds du réseau
    QVector<RuntimeNode*> nodes = m_network.getAllNodes();
    for (RuntimeNode* node : nodes) {
        m_network.removeNode(node);
    }

    m_selectedNode = nullptr;
    ui->graphView->clearGraph();
    m_tableModel->setTable(nullptr);

    m_isProcessing = false;
    setUiEnabled(true);
    processNextOperation();
}

bool MainWindow::saveNetworkToFile(const QString& fileName)
{
    // Valider le réseau avant sauvegarde
    if (!m_network.validate()) {
        QMessageBox::warning(this, "Erreur de validation",
                             "Le réseau contient des erreurs et ne peut pas être sauvegardé.\n"
                             "Vérifiez que tous les nœuds sont correctement connectés.");
        return false;
    }

    // Récupérer les positions des nœuds depuis GraphView
    QJsonObject json = m_network.toJson();

    // Ajouter les positions au JSON
    QJsonArray nodesArray = json["nodes"].toArray();
    QHash<QUuid, QPointF> positions = ui->graphView->getNodePositions();

    for (int i = 0; i < nodesArray.size(); ++i) {
        QJsonObject nodeObj = nodesArray[i].toObject();
        QUuid nodeId(nodeObj["id"].toString());

        if (positions.contains(nodeId)) {
            QPointF pos = positions[nodeId];
            QJsonObject posObj;
            posObj["x"] = pos.x();
            posObj["y"] = pos.y();
            nodeObj["position"] = posObj;
            nodesArray[i] = nodeObj;
        }
    }

    json["nodes"] = nodesArray;

    // Sauvegarder le fichier
    QSaveFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, "Erreur",
                              QString("Impossible d'ouvrir le fichier en écriture:\n%1").arg(file.errorString()));
        return false;
    }

    QJsonDocument doc(json);
    file.write(doc.toJson());

    if (!file.commit()) {
        QMessageBox::critical(this, "Erreur",
                              QString("Impossible d'écrire le fichier:\n%1").arg(file.errorString()));
        return false;
    }

    return true;
}

bool MainWindow::loadNetworkFromFile(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Erreur",
                              QString("Impossible d'ouvrir le fichier en lecture:\n%1").arg(file.errorString()));
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        QMessageBox::critical(this, "Erreur de parsing",
                              QString("Erreur JSON:\n%1").arg(parseError.errorString()));
        return false;
    }

    // Nettoyer le réseau actuel
    doClearNetwork();

    // Charger le nouveau réseau
    if (!m_network.fromJson(doc.object())) {
        QMessageBox::critical(this, "Erreur",
                              "Impossible de charger le réseau depuis le fichier.");
        return false;
    }

    // Restaurer les positions des nœuds
    QJsonObject json = doc.object();
    QJsonArray nodesArray = json["nodes"].toArray();

    for (const QJsonValue& nodeValue : nodesArray) {
        QJsonObject nodeObj = nodeValue.toObject();

        if (nodeObj.contains("position")) {
            QUuid nodeId(nodeObj["id"].toString());
            QJsonObject posObj = nodeObj["position"].toObject();
            QPointF position(posObj["x"].toDouble(), posObj["y"].toDouble());

            ui->graphView->setNodePosition(nodeId, position);
        }
    }

    // Mettre à jour l'affichage
    ui->graphView->refresh();

    // Sélectionner le premier nœud source par défaut
    QVector<RuntimeNode*> roots = m_network.findRootNodes();
    if (!roots.isEmpty()) {
        m_selectedNode = roots.first();
        m_tableModel->setTable(&m_selectedNode->getCachedResult());
    }

    return true;
}


// ========== GESTION DES NOEUDS ==========

void MainWindow::doReadCSV()
{
    // === ÉTAPE 1 : Dialogue pour obtenir les paramètres CSV ===
    QMap<QString, QVariant> params = showCSVReadDialog();

    if (params.isEmpty()) {
        // L'utilisateur a annulé
        m_isProcessing = false;
        setUiEnabled(true);
        processNextOperation();
        return;
    }

    // === ÉTAPE 2 : Créer et configurer le connector ===
    auto connector = std::make_unique<CSVConnector>(this);
    connector->configure(params);

    // === ÉTAPE 3 : Créer le SourceNode ===
    const QString nodeName = params.value("nodeName", "CSV Source").toString().trimmed();
    auto* sourceNode = m_network.createSourceNode(
        std::move(connector),
        nodeName.isEmpty() ? QString("CSV Source") : nodeName);

    connect(sourceNode, &RuntimeNode::tableChanged,
            this, &MainWindow::onNodeTableChanged);

    onNodeAdded(sourceNode);

    m_selectedNode = sourceNode;
    m_tableModel->setTable(&sourceNode->getCachedResult());

    statusBar()->showMessage(tr("CSV chargé: %1 lignes, %2 colonnes")
                                 .arg(sourceNode->getCachedResult().rowCount())
                                 .arg(sourceNode->getCachedResult().columnCount()), 3000);

    m_isProcessing = false;
    setUiEnabled(true);
    processNextOperation();
}

void MainWindow::doWriteCSV(){
    // === ÉTAPE 1 : Dialogue pour obtenir les paramètres CSV ===
    QMap<QString, QVariant> params = showCSVWriteDialog();

    if (params.isEmpty()) {
        // L'utilisateur a annulé
        m_isProcessing = false;
        setUiEnabled(true);
        processNextOperation();
        return;
    }

    // === ÉTAPE 2 : Créer et configurer le connector ===
    auto connector = std::make_unique<CSVConnector>(this);
    connector->configure(params);

    // === ÉTAPE 3 : Créer le SourceNode ===
    const QString nodeName = params.value("nodeName", "CSV Target").toString().trimmed();
    auto* targetNode = m_network.createTargetNode(
        std::move(connector),
        m_selectedNode,
        nodeName.isEmpty() ? QString("CSV Target") : nodeName);

    connect(targetNode, &RuntimeNode::tableChanged, this, &MainWindow::onNodeTableChanged);
    connect(targetNode, &TargetNode::exportCompleted, this, &MainWindow::onExportCompleted);

    ui->graphView->addNodeWithConnection(m_selectedNode->getId(), targetNode->getId(), targetNode->getNodeType(), targetNode->getName());
    ui->graphView->selectNewNode(targetNode->getId());
    ui->graphView->requestLayout();

    m_selectedNode = targetNode;
    m_tableModel->setTable(&targetNode->getCachedResult());

    statusBar()->showMessage(tr("CSV enregistré: %1 lignes, %2 colonnes")
                                 .arg(targetNode->getCachedResult().rowCount())
                                 .arg(targetNode->getCachedResult().columnCount()), 3000);

    m_isProcessing = false;
    setUiEnabled(true);
    processNextOperation();
}

void MainWindow::doReadTXT()
{
    QMap<QString, QVariant> params = showTXTReadDialog();

    if (params.isEmpty()) {
        m_isProcessing = false;
        setUiEnabled(true);
        processNextOperation();
        return;
    }

    auto connector = std::make_unique<TXTConnector>(this);
    connector->configure(params);

    const QString nodeName = params.value("nodeName", "TXT Source").toString().trimmed();
    auto* sourceNode = m_network.createSourceNode(std::move(connector),
                                                  nodeName.isEmpty() ? QString("TXT Source") : nodeName);

    connect(sourceNode, &RuntimeNode::tableChanged,
            this, &MainWindow::onNodeTableChanged);

    onNodeAdded(sourceNode);

    m_selectedNode = sourceNode;
    m_tableModel->setTable(&sourceNode->getCachedResult());

    statusBar()->showMessage(tr("TXT chargé: %1 lignes, %2 colonnes")
                                 .arg(sourceNode->getCachedResult().rowCount())
                                 .arg(sourceNode->getCachedResult().columnCount()), 3000);

    m_isProcessing = false;
    setUiEnabled(true);
    processNextOperation();
}

void MainWindow::doWriteTXT(){
    if (!checkNodeSelected()) {
        m_isProcessing = false;
        setUiEnabled(true);
        processNextOperation();
        return;
    }

    if (!checkNodeHasData()) {
        m_isProcessing = false;
        setUiEnabled(true);
        processNextOperation();
        return;
    }

    QMap<QString, QVariant> params = showTXTWriteDialog();

    if (params.isEmpty()) {
        m_isProcessing = false;
        setUiEnabled(true);
        processNextOperation();
        return;
    }

    auto connector = std::make_unique<TXTConnector>(this);
    connector->configure(params);

    const QString nodeName = params.value("nodeName", "TXT Target").toString().trimmed();
    auto* targetNode = m_network.createTargetNode(std::move(connector), m_selectedNode,
                                                  nodeName.isEmpty() ? QString("TXT Target") : nodeName);

    connect(targetNode, &RuntimeNode::tableChanged, this, &MainWindow::onNodeTableChanged);
    connect(targetNode, &TargetNode::exportCompleted, this, &MainWindow::onExportCompleted);

    ui->graphView->addNodeWithConnection(m_selectedNode->getId(), targetNode->getId(), targetNode->getNodeType(), targetNode->getName());
    ui->graphView->selectNewNode(targetNode->getId());
    ui->graphView->requestLayout();

    m_selectedNode = targetNode;
    m_tableModel->setTable(&targetNode->getCachedResult());

    statusBar()->showMessage(tr("TXT enregistré: %1 lignes, %2 colonnes")
                                 .arg(targetNode->getCachedResult().rowCount())
                                 .arg(targetNode->getCachedResult().columnCount()), 3000);

    m_isProcessing = false;
    setUiEnabled(true);
    processNextOperation();
}

void MainWindow::doWriteExcel(){
    if (!checkNodeSelected()) {
        m_isProcessing = false;
        setUiEnabled(true);
        processNextOperation();
        return;
    }

    if (!checkNodeHasData()) {
        m_isProcessing = false;
        setUiEnabled(true);
        processNextOperation();
        return;
    }

    QMap<QString, QVariant> params = showExcelWriteDialog();

    if (params.isEmpty()) {
        m_isProcessing = false;
        setUiEnabled(true);
        processNextOperation();
        return;
    }

    auto connector = std::make_unique<ExcelConnector>(this);
    connector->configure(params);

    const QString nodeName = params.value("nodeName", "Excel Target").toString().trimmed();
    auto* targetNode = m_network.createTargetNode(std::move(connector), m_selectedNode,
                                                  nodeName.isEmpty() ? QString("Excel Target") : nodeName);

    connect(targetNode, &RuntimeNode::tableChanged, this, &MainWindow::onNodeTableChanged);
    connect(targetNode, &TargetNode::exportCompleted, this, &MainWindow::onExportCompleted);
    ui->graphView->addNodeWithConnection(m_selectedNode->getId(), targetNode->getId(), targetNode->getNodeType(), "Excel Target");
    ui->graphView->selectNewNode(targetNode->getId());
    ui->graphView->requestLayout();

    m_selectedNode = targetNode;
    m_tableModel->setTable(&targetNode->getCachedResult());

    statusBar()->showMessage(tr("Excel enregistré: %1 lignes, %2 colonnes")
                                 .arg(targetNode->getCachedResult().rowCount())
                                 .arg(targetNode->getCachedResult().columnCount()), 3000);

    m_isProcessing = false;
    setUiEnabled(true);
    processNextOperation();
}

void MainWindow::doReadExcel()
{
    // === ÉTAPE 1 : Dialogue pour obtenir les paramètres Excel ===
    QMap<QString, QVariant> params = showExcelReadDialog();

    if (params.isEmpty()) {
        m_isProcessing = false;
        setUiEnabled(true);
        processNextOperation();
        return;
    }

    // === ÉTAPE 2 : Créer et configurer le connector ===
    auto connector = std::make_unique<ExcelConnector>(this);
    connector->configure(params);

    // === ÉTAPE 3 : Créer le SourceNode ===
    const QString nodeName = params.value("nodeName", "Excel Source").toString().trimmed();
    auto* sourceNode = m_network.createSourceNode(std::move(connector),
                                                  nodeName.isEmpty() ? QString("Excel Source") : nodeName);

    connect(sourceNode, &RuntimeNode::tableChanged,
            this, &MainWindow::onNodeTableChanged);

    onNodeAdded(sourceNode);

    m_selectedNode = sourceNode;
    m_tableModel->setTable(&sourceNode->getCachedResult());

    statusBar()->showMessage(tr("Excel chargé: %1 lignes, %2 colonnes")
                                 .arg(sourceNode->getCachedResult().rowCount())
                                 .arg(sourceNode->getCachedResult().columnCount()), 3000);

    m_isProcessing = false;
    setUiEnabled(true);
    processNextOperation();
}

void MainWindow::doReadFEC()
{
    QMap<QString, QVariant> params = CustomFECDialog::getFECReadParameters(this, "FEC Source");

    if (params.isEmpty()) {
        m_isProcessing = false;
        setUiEnabled(true);
        processNextOperation();
        return;
    }

    auto connector = std::make_unique<FECConnector>(this);
    connector->configure(params);

    const QString nodeName = params.value("nodeName", "FEC Source").toString().trimmed();
    auto* sourceNode = m_network.createSourceNode(std::move(connector),
                                                  nodeName.isEmpty() ? QString("FEC Source") : nodeName);

    connect(sourceNode, &RuntimeNode::tableChanged,
            this, &MainWindow::onNodeTableChanged);

    onNodeAdded(sourceNode);

    m_selectedNode = sourceNode;
    m_tableModel->setTable(&sourceNode->getCachedResult());

    statusBar()->showMessage(tr("FEC chargé: %1 lignes, %2 colonnes")
                                 .arg(sourceNode->getCachedResult().rowCount())
                                 .arg(sourceNode->getCachedResult().columnCount()), 3000);

    m_isProcessing = false;
    setUiEnabled(true);
    processNextOperation();
}

void MainWindow::doReadSQL()
{
    QMap<QString, QVariant> params = CustomSQLDialog::getSQLReadParameters(this, "SQL Source");

    if (params.isEmpty()) {
        m_isProcessing = false;
        setUiEnabled(true);
        processNextOperation();
        return;
    }

    auto connector = std::make_unique<SQLConnector>(this);
    connector->configure(params);

    const QString nodeName = params.value("nodeName", "SQL Source").toString().trimmed();
    auto* sourceNode = m_network.createSourceNode(std::move(connector),
                                                  nodeName.isEmpty() ? QString("SQL Source") : nodeName);

    connect(sourceNode, &RuntimeNode::tableChanged,
            this, &MainWindow::onNodeTableChanged);

    onNodeAdded(sourceNode);

    m_selectedNode = sourceNode;
    m_tableModel->setTable(&sourceNode->getCachedResult());

    statusBar()->showMessage(tr("SQL chargé: %1 lignes, %2 colonnes")
                                 .arg(sourceNode->getCachedResult().rowCount())
                                 .arg(sourceNode->getCachedResult().columnCount()), 3000);

    m_isProcessing = false;
    setUiEnabled(true);
    processNextOperation();
}

void MainWindow::doReadJSON()
{
    QMap<QString, QVariant> params = CustomJSONDialog::getJSONReadParameters(this, "JSON Source");

    if (params.isEmpty()) {
        m_isProcessing = false;
        setUiEnabled(true);
        processNextOperation();
        return;
    }

    auto connector = std::make_unique<JSONConnector>(this);
    connector->configure(params);

    const QString nodeName = params.value("nodeName", "JSON Source").toString().trimmed();
    auto* sourceNode = m_network.createSourceNode(std::move(connector),
                                                  nodeName.isEmpty() ? QString("JSON Source") : nodeName);

    connect(sourceNode, &RuntimeNode::tableChanged,
            this, &MainWindow::onNodeTableChanged);

    onNodeAdded(sourceNode);

    m_selectedNode = sourceNode;
    m_tableModel->setTable(&sourceNode->getCachedResult());

    statusBar()->showMessage(tr("JSON chargé: %1 lignes, %2 colonnes")
                                 .arg(sourceNode->getCachedResult().rowCount())
                                 .arg(sourceNode->getCachedResult().columnCount()), 3000);

    m_isProcessing = false;
    setUiEnabled(true);
    processNextOperation();
}

void MainWindow::doReadXML()
{
    QMap<QString, QVariant> params = showXMLReadDialog();

    if (params.isEmpty()) {
        m_isProcessing = false;
        setUiEnabled(true);
        processNextOperation();
        return;
    }

    auto connector = std::make_unique<XMLConnector>(this);
    connector->configure(params);

    const QString nodeName = params.value("nodeName", "XML Source").toString().trimmed();
    auto* sourceNode = m_network.createSourceNode(std::move(connector),
                                                  nodeName.isEmpty() ? QString("XML Source") : nodeName);

    connect(sourceNode, &RuntimeNode::tableChanged,
            this, &MainWindow::onNodeTableChanged);

    onNodeAdded(sourceNode);

    m_selectedNode = sourceNode;
    m_tableModel->setTable(&sourceNode->getCachedResult());

    statusBar()->showMessage(tr("XML chargé: %1 lignes, %2 colonnes")
                                 .arg(sourceNode->getCachedResult().rowCount())
                                 .arg(sourceNode->getCachedResult().columnCount()), 3000);

    m_isProcessing = false;
    setUiEnabled(true);
    processNextOperation();
}

void MainWindow::doWriteXML(){
    if (!checkNodeSelected()) {
        m_isProcessing = false;
        setUiEnabled(true);
        processNextOperation();
        return;
    }

    if (!checkNodeHasData()) {
        m_isProcessing = false;
        setUiEnabled(true);
        processNextOperation();
        return;
    }

    QMap<QString, QVariant> params = showXMLWriteDialog();

    if (params.isEmpty()) {
        m_isProcessing = false;
        setUiEnabled(true);
        processNextOperation();
        return;
    }

    auto connector = std::make_unique<XMLConnector>(this);
    connector->configure(params);

    const QString nodeName = params.value("nodeName", "XML Target").toString().trimmed();
    auto* targetNode = m_network.createTargetNode(std::move(connector), m_selectedNode,
                                                  nodeName.isEmpty() ? QString("XML Target") : nodeName);

    connect(targetNode, &RuntimeNode::tableChanged, this, &MainWindow::onNodeTableChanged);
    connect(targetNode, &TargetNode::exportCompleted, this, &MainWindow::onExportCompleted);

    ui->graphView->addNodeWithConnection(m_selectedNode->getId(), targetNode->getId(), targetNode->getNodeType(), targetNode->getName());
    ui->graphView->selectNewNode(targetNode->getId());
    ui->graphView->requestLayout();

    m_selectedNode = targetNode;
    m_tableModel->setTable(&targetNode->getCachedResult());

    statusBar()->showMessage(tr("XML enregistré: %1 lignes, %2 colonnes")
                                 .arg(targetNode->getCachedResult().rowCount())
                                 .arg(targetNode->getCachedResult().columnCount()), 3000);

    m_isProcessing = false;
    setUiEnabled(true);
    processNextOperation();
}

void MainWindow::doReadWeb()
{
    QMap<QString, QVariant> params = showWebReadDialog();

    if (params.isEmpty()) {
        m_isProcessing = false;
        setUiEnabled(true);
        processNextOperation();
        return;
    }

    auto connector = std::make_unique<WebConnector>(this);
    connector->configure(params);

    const QString nodeName = params.value("nodeName", "Web Source").toString().trimmed();
    auto* sourceNode = m_network.createSourceNode(std::move(connector),
                                                  nodeName.isEmpty() ? QString("Web Source") : nodeName);

    connect(sourceNode, &RuntimeNode::tableChanged,
            this, &MainWindow::onNodeTableChanged);

    onNodeAdded(sourceNode);

    m_selectedNode = sourceNode;
    m_tableModel->setTable(&sourceNode->getCachedResult());

    statusBar()->showMessage(tr("Données web chargées: %1 lignes, %2 colonnes")
                                 .arg(sourceNode->getCachedResult().rowCount())
                                 .arg(sourceNode->getCachedResult().columnCount()), 3000);

    m_isProcessing = false;
    setUiEnabled(true);
    processNextOperation();
}

void MainWindow::doReadPDF()
{
    QMap<QString, QVariant> params = showPDFReadDialog();

    if (params.isEmpty()) {
        m_isProcessing = false;
        setUiEnabled(true);
        processNextOperation();
        return;
    }

    auto connector = std::make_unique<PDFConnector>(this);
    connector->configure(params);

    const QString nodeName = params.value("nodeName", "PDF Source").toString().trimmed();
    auto* sourceNode = m_network.createSourceNode(std::move(connector),
                                                  nodeName.isEmpty() ? QString("PDF Source") : nodeName);

    connect(sourceNode, &RuntimeNode::tableChanged,
            this, &MainWindow::onNodeTableChanged);

    onNodeAdded(sourceNode);

    m_selectedNode = sourceNode;
    m_tableModel->setTable(&sourceNode->getCachedResult());

    statusBar()->showMessage(tr("PDF chargé: %1 lignes, %2 colonnes")
                                 .arg(sourceNode->getCachedResult().rowCount())
                                 .arg(sourceNode->getCachedResult().columnCount()), 3000);

    m_isProcessing = false;
    setUiEnabled(true);
    processNextOperation();
}

void MainWindow::doAddFilter()
{
    if (!checkNodeHasData("Veuillez d'abord charger des données ou sélectionner un nœud contenant des données."))
        return;

    const DataTable& table = m_selectedNode->getCachedResult();

    // Utiliser le dialogue avec le DataTable complet
    QString column, opStr, value, nodeName;

    if (!CustomFilterDialog::getFilterParameters(this, &table, column, opStr, value, &nodeName)) {
        m_isProcessing = false;
        setUiEnabled(true);
        processNextOperation();
        return;
    }

    // Utiliser la méthode statique de CustomFilterDialog pour convertir
    FilterTransformation::Operator op = CustomFilterDialog::convertOperatorString(opStr);

    // Créer le filtre avec l'enum
    auto* filter = new FilterTransformation(column, op, value, this);
    RuntimeNode* parentNode = m_selectedNode; // Save parent before onNodeAdded modifies m_selectedNode
    const QString safeNodeName = nodeName.trimmed().isEmpty() ? filter->description() : nodeName.trimmed();
    auto* filterNode = m_network.createTransformationNode(filter, m_selectedNode, safeNodeName);

    connect(filterNode, &RuntimeNode::tableChanged,this, &MainWindow::onNodeTableChanged);

    onNodeAdded(filterNode);
    AddConnection(parentNode, filterNode); // ajout de la connection sur le graphique
   // ui->graphView->selectNewNode(filterNode->getId()); // sélectiooner le nouveau noeud


    statusBar()->showMessage(tr("Filtre ajouté: %1").arg(filter->description()), 3000);

    m_isProcessing = false;
    setUiEnabled(true);
    processNextOperation();
}

void MainWindow::doSelectColumns()
{
    if (!checkNodeHasData("Veuillez d'abord charger des données ou sélectionner un nœud contenant des données."))
        return;

    const DataTable& table = m_selectedNode->getCachedResult();

    // Utiliser le nouveau dialogue
    QStringList columns;
    QString nodeName;

    if (!CustomSelectColumnsDialog::getSelectedColumns(this, &table, columns, &nodeName)) {
        // L'utilisateur a annulé
        m_isProcessing = false;
        setUiEnabled(true);
        processNextOperation();
        return;
    }

    // Vérifier qu'au moins une colonne est sélectionnée
    if (columns.isEmpty()) {
        QMessageBox::warning(this, "Erreur",
                             "Veuillez sélectionner au moins une colonne.");
        m_isProcessing = false;
        setUiEnabled(true);
        processNextOperation();
        return;
    }

    // Créer et ajouter la transformation
    auto* trans = new SelectColumnsTransformation(columns, this);
    RuntimeNode* parentNode = m_selectedNode; // Save parent before onNodeAdded modifies m_selectedNode
    const QString safeNodeName = nodeName.trimmed().isEmpty() ? trans->description() : nodeName.trimmed();
    auto* selectNode = m_network.createTransformationNode(trans,m_selectedNode, safeNodeName);

    connect(selectNode, &RuntimeNode::tableChanged,this, &MainWindow::onNodeTableChanged);

    onNodeAdded(selectNode); // ajout du nouveau noeud sur le graphique
    AddConnection(parentNode, selectNode); // ajout de la connection sur le graphique
    //ui->graphView->selectNewNode(selectNode->getId()); // sélectiooner le nouveau noeud

    statusBar()->showMessage(tr("Sélection de colonnes: %1 colonnes conservées sur %2")
                                 .arg(columns.size())
                                 .arg(table.columnCount()), 3000);

    m_isProcessing = false;
    setUiEnabled(true);
    processNextOperation();
}

void MainWindow::doAddCalculatedColumn()
{
    if (!checkNodeHasData("Veuillez d'abord charger des données ou sélectionner un nœud contenant des données."))
        return;

    const DataTable& table = m_selectedNode->getCachedResult();

    // Utiliser le dialogue pour créer une colonne calculée
    QString columnName;
    QString expression;
    dn::core::ColumnType columnType;

    if (!CustomCalculatedColumnDialog::getCalculatedColumnDetails(this, &table, columnName, expression, columnType)) {
        // L'utilisateur a annulé
        m_isProcessing = false;
        setUiEnabled(true);
        processNextOperation();
        return;
    }

    // Vérifier que le nom de colonne n'est pas vide
    if (columnName.isEmpty()) {
        QMessageBox::warning(this, "Erreur",
                             "Veuillez saisir un nom pour la colonne calculée.");
        m_isProcessing = false;
        setUiEnabled(true);
        processNextOperation();
        return;
    }

    // Vérifier que l'expression n'est pas vide
    if (expression.isEmpty()) {
        QMessageBox::warning(this, "Erreur",
                             "Veuillez saisir une expression pour la colonne calculée.");
        m_isProcessing = false;
        setUiEnabled(true);
        processNextOperation();
        return;
    }

    // Créer et ajouter la transformation de colonne calculée
    auto* calc = new CalculatedColumnTransformation(columnName, expression, columnType, this);
    RuntimeNode* parentNode = m_selectedNode; // Save parent before onNodeAdded modifies m_selectedNode
    const QString safeNodeName = columnName.trimmed().isEmpty() ? calc->description() : columnName.trimmed();
    auto* calcNode = m_network.createTransformationNode(calc, m_selectedNode, safeNodeName);

    connect(calcNode, &RuntimeNode::tableChanged, this, &MainWindow::onNodeTableChanged);

    onNodeAdded(calcNode); // ajout du nouveau noeud sur le graphique
    AddConnection(parentNode, calcNode); // ajout de la connection sur le graphique

    statusBar()->showMessage(tr("Colonne calculée ajoutée: %1 (%2)")
                                 .arg(columnName)
                                 .arg(expression), 3000);

    m_isProcessing = false;
    setUiEnabled(true);
    processNextOperation();
}

void MainWindow::doRenameColumns()
{
    if (!m_selectedNode) {
        QMessageBox::information(this, "Info", "Veuillez d'abord charger des données ou sélectionner un nœud.");
        m_isProcessing = false;
        setUiEnabled(true);
        processNextOperation();
        return;
    }

    bool ok;
    QString text = QInputDialog::getText(
        this,
        "Rename columns",
        "Format: ancien:nouveau, ancien2:nouveau2",
        QLineEdit::Normal,
        "",
        &ok
        );

    if (!ok || text.isEmpty()) {
        m_isProcessing = false;
        setUiEnabled(true);
        processNextOperation();
        return;
    }

    QHash<QString, QString> renames;
    QStringList pairs = text.split(",", Qt::SkipEmptyParts);

    for (QString pair : pairs) {
        QStringList parts = pair.split(":");
        if (parts.size() != 2)
            continue;

        QString oldName = parts[0].trimmed();
        QString newName = parts[1].trimmed();

        if (!oldName.isEmpty() && !newName.isEmpty())
            renames[oldName] = newName;
    }

    if (renames.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Format invalide. Utilisez 'ancien:nouveau'.");
        m_isProcessing = false;
        setUiEnabled(true);
        processNextOperation();
        return;
    }

    auto* trans = new RenameColumnsTransformation(renames, this);
    RuntimeNode* parentNode = m_selectedNode; // Save parent before onNodeAdded modifies m_selectedNode
    auto* renameNode = m_network.createTransformationNode(trans,m_selectedNode, trans->description());

    connect(renameNode, &RuntimeNode::tableChanged,this, &MainWindow::onNodeTableChanged);

    onNodeAdded(renameNode); // ajout du nouveau noeud sur le graphique
    AddConnection(parentNode, renameNode); // ajout de la connection sur le graphique
  //  ui->graphView->selectNewNode(renameNode->getId()); // sélectiooner le nouveau noeud


    statusBar()->showMessage(tr("Renommage de colonnes effectué"), 2000);

    m_isProcessing = false;
    setUiEnabled(true);
    processNextOperation();
}


// ========== FONCTIONS UTILITAIRES ==========

bool MainWindow::checkNodeSelected(const QString& errorMessage)
{
    if (!m_selectedNode) {
        QMessageBox::information(this, "Info", errorMessage);
        m_isProcessing = false;
        setUiEnabled(true);
        processNextOperation();
        return false;
    }
    return true;
}

bool MainWindow::checkNodeHasData(const QString& errorMessage)
{
    if (!m_selectedNode) {
        return checkNodeSelected();
    }

    const DataTable& table = m_selectedNode->getCachedResult();
    if (table.rowCount() == 0) {
        QMessageBox::information(this, "Info", errorMessage);
        m_isProcessing = false;
        setUiEnabled(true);
        processNextOperation();
        return false;
    }
    return true;
}

bool MainWindow::confirmDiscardChanges()
{
    // Vérifier si le réseau a des nœuds
    if (m_network.getAllNodes().isEmpty()) {
        return true;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(this,
                                                              "Nouveau réseau",
                                                              "Voulez-vous sauvegarder les changements avant de continuer ?",
                                                              QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    if (reply == QMessageBox::Save) {
        onSaveNetwork();
        // Vérifier si l'utilisateur a annulé la sauvegarde
        return !m_currentNetworkFile.isEmpty() || QFile::exists(m_currentNetworkFile);
    }

    return reply != QMessageBox::Cancel;
}


void MainWindow::updateWindowTitle()
{
    QString title = "Data Network";

    if (!m_currentNetworkFile.isEmpty()) {
        QFileInfo fileInfo(m_currentNetworkFile);
        title += QString(" - [%1]").arg(fileInfo.fileName());
    }

    if (!m_network.validate()) {
        title += " [modifié]";
    }

    setWindowTitle(title);
}

void MainWindow::onDeleteNode(const QUuid& nodeId)
{
    RuntimeNode* node = m_network.findNode(nodeId);
    if (!node) return;

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Supprimer le nœud",
        QString("Voulez-vous vraiment supprimer le nœud '%1' ?\n"
                "Toutes les connexions seront également supprimées.")
            .arg(node->getName()),
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        m_network.removeNode(node);
        ui->graphView->removeNode(nodeId);
        if (m_selectedNode == node) {
            m_selectedNode = nullptr;
            m_tableModel->setTable(nullptr);
        }
        statusBar()->showMessage(QString("Nœud supprimé: %1").arg(node->getName()), 2000);
    }
}

void MainWindow::onRenameNode(const QUuid& nodeId)
{
    RuntimeNode* node = m_network.findNode(nodeId);
    if (!node) return;

    bool ok;
    QString newName = QInputDialog::getText(
        this,
        "Renommer le nœud",
        "Nouveau nom:",
        QLineEdit::Normal,
        node->getName(),
        &ok
        );

    if (ok && !newName.isEmpty()) {
        node->setName(newName);
        ui->graphView->updateNodeLabel(nodeId, newName);
        statusBar()->showMessage(QString("Nœud renommé: %1").arg(newName), 2000);
    }
}

void MainWindow::onDuplicateNode(const QUuid& nodeId)
{
    RuntimeNode* original = m_network.findNode(nodeId);
    if (!original) return;

    // Cette fonctionnalité nécessite une méthode de clonage dans RuntimeNode
    // Pour l'instant, afficher un message
    QMessageBox::information(this, "Duplication",
                             "La duplication de nœuds sera implémentée prochainement.");
}

void MainWindow::onEditNode(const QUuid& nodeId)
{
    RuntimeNode* node = m_network.findNode(nodeId);
    if (!node) return;

    // Selon le type du nœud, ouvrir le dialogue approprié
    switch (node->getNodeType()) {
    case NodeType::Source: {
        // Vérifier quel type de source
        if (node->getAllParameters().contains("connectorType")) {
            QString connType = node->getParameter("connectorType");
            if (connType == "CSV") { onLoadCSVData(); }
            else if (connType == "TXT") { onLoadTXTData(); }
            else if (connType == "Excel") { onLoadExcelData(); }
            else if (connType == "PDF") { onLoadPDFData(); }
            else if (connType == "FEC") { onLoadFECData(); }
            else if (connType == "SQL") { onLoadSQLData(); }
            else if (connType == "JSON") { onLoadJSONData(); }
            else if (connType == "XML") { onLoadXMLData(); }
            else if (connType == "Web") { onLoadWebData(); }
        } else {
            onLoadCSVData(); // Par défaut
        }
        break;
    }

    case NodeType::Transform: {
        // Récupérer le TransformationNode pour accéder à la transformation
        auto* transNode = dynamic_cast<TransformationNode*>(node);
        if (!transNode || !transNode->getTransformation()) break;

        // Vérifier le type de transformation
        if (dynamic_cast<FilterTransformation*>(transNode->getTransformation())) {
            onAddFilter();
        }
        else if (dynamic_cast<SelectColumnsTransformation*>(transNode->getTransformation())) {
            onSelectColumns();
        }
        else if (dynamic_cast<CalculatedColumnTransformation*>(transNode->getTransformation())) {
            onAddCalculatedColumn();
        }
        else if (dynamic_cast<RenameColumnsTransformation*>(transNode->getTransformation())) {
            onRenameColumns();
        }
        break;
    }

    case NodeType::Merge:
        QMessageBox::information(this, "Configurer fusion",
                                 "Configuration des fusions à implémenter.");
        break;

    case NodeType::Target:
        // Modifier la cible
        if (node->getAllParameters().contains("connectorType")) {
            QString connType = node->getParameter("connectorType");
            if (connType == "CSV") { onSaveCSVData(); }
            else if (connType == "TXT") { onSaveTXTData(); }
            else if (connType == "Excel") { onSaveExcelData(); }
            else if (connType == "XML") { onSaveXMLData(); }
        }
        break;
    }
}


void MainWindow::onComputeFromNode(const QUuid& nodeId)
{
    RuntimeNode* node = m_network.findNode(nodeId);
    if (!node) return;

    m_network.computeFrom(node);

    if (m_selectedNode == node) {
        m_tableModel->setTable(&node->getCachedResult());
    }

    statusBar()->showMessage(QString("Calculé depuis le nœud: %1").arg(node->getName()), 2000);
}