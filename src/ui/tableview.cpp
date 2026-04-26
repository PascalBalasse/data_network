#include "tableview.h"
#include <QMenu>
#include <QAction>

dn::ui::TableView::TableView(QWidget* parent)
    : QTableView(parent)
{
}

void dn::ui::TableView::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu contextMenu(this);
    QAction* titleAction = contextMenu.addAction(tr("Actions colonnes"));
    titleAction->setEnabled(false);
    contextMenu.addSeparator();
    QAction* renameColumnsAction = contextMenu.addAction(tr("Renommer les colonnes"));
    connect(renameColumnsAction, &QAction::triggered, this, &dn::ui::TableView::renameColumnsRequested);
    contextMenu.exec(event->globalPos());
}