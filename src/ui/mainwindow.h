/**
 * @file mainwindow.h
 * @brief Fenêtre principale de l'application
 *
 * Gère l'interface utilisateur principale:
 * - Menu et barre d'outils
 * - Vue graphique du réseau (QGraphicsScene)
 * - Aperçu des données (QTableView)
 * - Gestion du réseau de données
 *
 * Parent: QMainWindow
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "../network/core/DataTable.h"
#include "models/DataTableModel.h"
#include "../network/Network.h"
#include <QMainWindow>
#include <QGraphicsScene>
#include <QQueue>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

using namespace dn::network;
using namespace dn::transformations;

/// Type pour les opérations en file d'attente
using NetworkOperation = std::function<void()>;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    //══════════════════════════════════════════════════════════════════
    // Slots du menu Fichier
    //══════════════════════════════════════════════════════════════════
    void onNewNetwork();
    void onLoadNetwork();
    void onSaveNetwork();
    void onSaveNetworkAs();
    void onLoadCSVData();
    void onSaveCSVData();
    void onLoadTXTData();
    void onSaveTXTData();
    void onLoadExcelData();
    void onSaveExcelData();
    void onLoadPDFData();
    void onLoadFECData();
    void onLoadSQLData();
    void onLoadJSONData();
void onLoadXMLData();
    void onSaveXMLData();
    void onLoadWebData();
    void onAddFilter();
    void onSelectColumns();
    void onRenameColumns();
    void onAddCalculatedColumn();

    //══════════════════════════════════════════════════════════════════
    // Slots internes
    //══════════════════════════════════════════════════════════════════
    void onExportCompleted(bool success, const QString& message);
    void onApplySteps();
    void onClearNetwork();

    void onNodeTableChanged(const dn::core::DataTable& table);
    void onNodeAdded(dn::nodes::RuntimeNode* node);
    void onNodeRemoved(dn::nodes::RuntimeNode* node);

    void onGraphNodeSelected(const QUuid& nodeId);
    void onNetworkChanged();
    void onTableRowSelected(const QModelIndex &current, const QModelIndex &previous);

    void updateTablePreview(const dn::core::DataTable& table);

    /// Traitement de la file d'attente
    void processNextOperation();

    //══════════════════════════════════════════════════════════════════
    // Menu contextuel
    //════════════════════════���═════════════════════════════════════════
    void onDeleteNode(const QUuid& nodeId);
    void onRenameNode(const QUuid& nodeId);
    void onDuplicateNode(const QUuid& nodeId);
    void onEditNode(const QUuid& nodeId);
    void onComputeFromNode(const QUuid& nodeId);

private:
    //══════════════════════════════════════════════════════════════════
    // Variables membres
    //══════════════════════════════════════════════════════════════════
    Ui::MainWindow *ui;
    Network m_network;
    RuntimeNode* m_selectedNode = nullptr;
    std::unique_ptr<dn::ui::DataTableModel> m_tableModel;
    QString m_currentNetworkFile;

    // File d'attente d'opérations asynchrones
    QQueue<NetworkOperation> m_operationQueue;
    bool m_isProcessing = false;

    //══════════════════════════════════════════════════════════════════
    // Méthodes utilitaires
    //══════════════════════════════════════════════════════════════════
    void setupConnections();
    void queueOperation(NetworkOperation operation);
    void setUiEnabled(bool enabled);
    bool checkNodeSelected(const QString& errorMessage = "Veuillez d'abord charger des données ou sélectionner un nœud.");
    bool checkNodeHasData(const QString& errorMessage = "Le nœud sélectionné ne contient pas de données.");
    QList<int> selectedTableColumns() const;
    QString columnTypeToLabel(dn::core::ColumnType type) const;
    void applyTypeToSelectedColumns(dn::core::ColumnType type);

    //══════════════════════════════════════════════════════════════════
    // Opérations de lecture/écriture
    //══════════════════════════════════════════════════════════════════
    void doReadCSV();
    void doWriteCSV();
    void doReadTXT();
    void doWriteTXT();
    void doReadExcel();
    void doWriteExcel();
    void doReadPDF();
    void doReadFEC();
    void doReadSQL();
    void doReadJSON();
    void doReadXML();
    void doWriteXML();
    void doReadWeb();
    void doAddFilter();
    void doSelectColumns();
    void doRenameColumns();
    void doAddCalculatedColumn();
    void doClearNetwork();
    void AddConnection(dn::nodes::RuntimeNode* source, dn::nodes::RuntimeNode* target);

    //══════════════════════════════════════════════════════════════════
    // Dialogues de configuration
    //══════════════════════════════════════════════════════════════════
    QMap<QString, QVariant> showCSVReadDialog();
    QMap<QString, QVariant> showCSVWriteDialog();
    QMap<QString, QVariant> showTXTReadDialog();
    QMap<QString, QVariant> showTXTWriteDialog();
    QMap<QString, QVariant> showExcelReadDialog();
    QMap<QString, QVariant> showExcelWriteDialog();
    QMap<QString, QVariant> showPDFReadDialog();
    QMap<QString, QVariant> showJSONDialog();
    QMap<QString, QVariant> showXMLReadDialog();
    QMap<QString, QVariant> showXMLWriteDialog();
    QMap<QString, QVariant> showWebReadDialog();

    //══════════════════════════════════════════════════════════════════
    // Gestion du réseau
    //══════════════════════════════════════════════════════════════════
    bool confirmDiscardChanges();
    void clearNetwork();
    bool saveNetworkToFile(const QString& fileName);
    bool loadNetworkFromFile(const QString& fileName);
    void updateWindowTitle();
};

#endif // MAINWINDOW_H
