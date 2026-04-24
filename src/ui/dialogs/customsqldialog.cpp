#include "customsqldialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QGroupBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>

using namespace dn::dialogs;

CustomSQLDialog::CustomSQLDialog(const QString& defaultNodeName, QWidget *parent)
    : QDialog(parent)
    , m_defaultNodeName(defaultNodeName)
{
    setWindowTitle("Connexion à une base de données SQL");
    setMinimumWidth(500);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QLabel *nameLabel = new QLabel("<b>Nom du nœud :</b>", this);
    mainLayout->addWidget(nameLabel);
    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setMaxLength(20);
    m_nameEdit->setText(m_defaultNodeName.trimmed().left(20));
    mainLayout->addWidget(m_nameEdit);

    QLabel *infoLabel = new QLabel(
        "<b>Connexion à une base de données SQL</b><br>"
        "Sélectionnez le type de base de données et entrez les paramètres de connexion.", this);
    infoLabel->setWordWrap(true);
    mainLayout->addWidget(infoLabel);

    QGroupBox *dbTypeGroup = new QGroupBox("Type de base de données", this);
    QVBoxLayout *dbTypeLayout = new QVBoxLayout(dbTypeGroup);

    m_dbTypeCombo = new QComboBox(this);
    m_dbTypeCombo->addItem("SQLite (fichier local)", "QSQLITE");
    m_dbTypeCombo->addItem("MySQL", "QMYSQL");
    m_dbTypeCombo->addItem("PostgreSQL", "QPSQL");
    m_dbTypeCombo->addItem("ODBC", "QODBC");
    m_dbTypeCombo->addItem("SQL Server", "QODBC");
    dbTypeLayout->addWidget(m_dbTypeCombo);
    mainLayout->addWidget(dbTypeGroup);

    QGroupBox *connectionGroup = new QGroupBox("Paramètres de connexion", this);
    QGridLayout *gridLayout = new QGridLayout(connectionGroup);

    QLabel *hostLabel = new QLabel("Hôte :", this);
    m_hostEdit = new QLineEdit(this);
    m_hostEdit->setPlaceholderText("localhost");
    gridLayout->addWidget(hostLabel, 0, 0);
    gridLayout->addWidget(m_hostEdit, 0, 1);

    QLabel *portLabel = new QLabel("Port :", this);
    m_portEdit = new QLineEdit(this);
    m_portEdit->setPlaceholderText("3306");
    gridLayout->addWidget(portLabel, 1, 0);
    gridLayout->addWidget(m_portEdit, 1, 1);

    QLabel *databaseLabel = new QLabel("Base de données :", this);
    m_databaseEdit = new QLineEdit(this);
    m_databaseEdit->setPlaceholderText("ma_base.db ou nom de la base");
    gridLayout->addWidget(databaseLabel, 2, 0);
    gridLayout->addWidget(m_databaseEdit, 2, 1);

    QLabel *usernameLabel = new QLabel("Utilisateur :", this);
    m_usernameEdit = new QLineEdit(this);
    m_usernameEdit->setPlaceholderText("root");
    gridLayout->addWidget(usernameLabel, 3, 0);
    gridLayout->addWidget(m_usernameEdit, 3, 1);

    QLabel *passwordLabel = new QLabel("Mot de passe :", this);
    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    gridLayout->addWidget(passwordLabel, 4, 0);
    gridLayout->addWidget(m_passwordEdit, 4, 1);

    mainLayout->addWidget(connectionGroup);

    QGroupBox *queryGroup = new QGroupBox("Requête SQL", this);
    QVBoxLayout *queryLayout = new QVBoxLayout(queryGroup);

    QLabel *tableLabel = new QLabel("Nom de la table (optionnel) :", this);
    queryLayout->addWidget(tableLabel);
    m_tableNameEdit = new QLineEdit(this);
    m_tableNameEdit->setPlaceholderText("ma_table");
    queryLayout->addWidget(m_tableNameEdit);

    QLabel *orLabel = new QLabel("Ou entrez une requête SQL (prioritaire) :", this);
    queryLayout->addWidget(orLabel);
    m_sqlQueryEdit = new QLineEdit(this);
    m_sqlQueryEdit->setPlaceholderText("SELECT * FROM ma_table WHERE condition");
    queryLayout->addWidget(m_sqlQueryEdit);

    mainLayout->addWidget(queryGroup);

    mainLayout->addStretch();

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_testButton = new QPushButton("Tester la connexion", this);
    m_okButton = new QPushButton("Charger", this);
    QPushButton *cancelButton = new QPushButton("Annuler", this);

    m_okButton->setEnabled(false);
    m_okButton->setDefault(true);
    m_okButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; }");

    buttonLayout->addWidget(m_testButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_okButton);
    buttonLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonLayout);

    connect(m_dbTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CustomSQLDialog::onDatabaseTypeChanged);
    connect(m_testButton, &QPushButton::clicked, this, &CustomSQLDialog::onTestConnection);
    connect(m_okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    connect(m_databaseEdit, &QLineEdit::textChanged, this, [this](const QString& text) {
        m_okButton->setEnabled(!text.trimmed().isEmpty());
    });

    onDatabaseTypeChanged(0);
}

void CustomSQLDialog::onDatabaseTypeChanged(int index)
{
    QString dbType = m_dbTypeCombo->itemData(index).toString();

    bool needsServer = (dbType != "QSQLITE");
    m_hostEdit->setEnabled(needsServer);
    m_portEdit->setEnabled(needsServer);
    m_usernameEdit->setEnabled(needsServer);
    m_passwordEdit->setEnabled(needsServer);

    if (dbType == "QSQLITE") {
        m_hostEdit->clear();
        m_portEdit->clear();
        m_usernameEdit->clear();
        m_passwordEdit->clear();
        m_databaseEdit->setPlaceholderText("chemin/vers/fichier.db");
    } else if (dbType == "QMYSQL") {
        m_portEdit->setText("3306");
        m_databaseEdit->setPlaceholderText("nom_de_la_base");
    } else if (dbType == "QPSQL") {
        m_portEdit->setText("5432");
        m_databaseEdit->setPlaceholderText("nom_de_la_base");
    } else if (dbType == "QODBC") {
        m_portEdit->clear();
        m_databaseEdit->setPlaceholderText("DSN ou chaîne de connexion");
    }
}

void CustomSQLDialog::onTestConnection()
{
    QString dbType = m_dbTypeCombo->itemData(m_dbTypeCombo->currentIndex()).toString();
    QString dbName = m_databaseEdit->text().trimmed();

    if (dbName.isEmpty()) {
        QMessageBox::warning(this, "Test de connexion", "Veuillez entrer le nom de la base de données.");
        return;
    }

    QString connectionName = QString("test_conn_%1").arg(QDateTime::currentMSecsSinceEpoch());

    if (QSqlDatabase::contains(connectionName)) {
        QSqlDatabase::removeDatabase(connectionName);
    }

    QSqlDatabase db = QSqlDatabase::addDatabase(dbType, connectionName);
    db.setDatabaseName(dbName);

    if (dbType != "QSQLITE") {
        if (!m_hostEdit->text().isEmpty())
            db.setHostName(m_hostEdit->text());
        if (!m_portEdit->text().isEmpty())
            db.setPort(m_portEdit->text().toInt());
        if (!m_usernameEdit->text().isEmpty())
            db.setUserName(m_usernameEdit->text());
        if (!m_passwordEdit->text().isEmpty())
            db.setPassword(m_passwordEdit->text());
    }

    if (db.open()) {
        QMessageBox::information(this, "Test de connexion", "Connexion réussie !");
        db.close();
    } else {
        QMessageBox::critical(this, "Test de connexion", "Échec de la connexion :\n" + db.lastError().text());
    }

    QSqlDatabase::removeDatabase(connectionName);
}

QMap<QString, QVariant> CustomSQLDialog::getParameters() const
{
    QMap<QString, QVariant> params;
    params["databaseType"] = m_dbTypeCombo->itemData(m_dbTypeCombo->currentIndex());
    params["nodeName"] = m_nameEdit->text().trimmed();
    params["databaseName"] = m_databaseEdit->text();
    params["tableName"] = m_tableNameEdit->text();
    params["sqlQuery"] = m_sqlQueryEdit->text();
    params["hostName"] = m_hostEdit->text();
    params["port"] = m_portEdit->text();
    params["userName"] = m_usernameEdit->text();
    params["password"] = m_passwordEdit->text();
    return params;
}

QMap<QString, QVariant> CustomSQLDialog::getSQLReadParameters(QWidget *parent, const QString& defaultNodeName)
{
    CustomSQLDialog dialog(defaultNodeName, parent);

    if (dialog.exec() == QDialog::Accepted) {
        return dialog.getParameters();
    }

    return QMap<QString, QVariant>();
}
