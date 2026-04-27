#include "tableview.h"
#include <QMenu>
#include <QAction>

dn::ui::TableView::TableView(QWidget* parent)
    : QTableView(parent)
{
    auto* header = new TwoLineHeader(Qt::Horizontal, this);
    setHorizontalHeader(header);
    header->setVisible(true);
}

// création du menu contextuel
void dn::ui::TableView::contextMenuEvent(QContextMenuEvent* event)
{
    // création du menu
    QMenu contextMenu(this);
    QAction* titleAction = contextMenu.addAction(tr("Actions colonnes"));
    titleAction->setEnabled(false);
    contextMenu.addSeparator();
    QAction* renameColumnsAction = contextMenu.addAction(tr("Renommer les colonnes"));

    // connection des action aux signaux
    connect(renameColumnsAction, &QAction::triggered, this, &dn::ui::TableView::renameColumnsRequested);

    // éxécution du menu
    contextMenu.exec(event->globalPos());
}