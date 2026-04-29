#ifndef TABLEVIEW_H
#define TABLEVIEW_H

#include <QTableView>
#include <QContextMenuEvent>
#include "models/TwoLineHeader.h"

namespace dn::ui {

class TableView : public QTableView
{
    Q_OBJECT

public:
    explicit TableView(QWidget* parent = nullptr);

signals:
    void renameColumnsRequested();

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;
};

} // namespace dn::ui

#endif // TABLEVIEW_H