#pragma once

#include "game/evidence.h"
#include "protocol/packets/evidence_packets.h"

#include <QDialog>
#include <QList>
#include <QPushButton>
#include <QRadioButton>
#include <QString>
#include <QWidget>

namespace spritechat
{
class InventoryDialog : public QDialog
{
  Q_OBJECT

public:
  explicit InventoryDialog(const QString &directory, QWidget *parent = nullptr);

  void setInventory(const QString &label, const QList<theory::Evidence> &items);

  theory::InventoryTransferPacket::Mode transferMode() const;

Q_SIGNALS:
  void transferRequested(theory::InventoryTransferPacket::Mode mode, const QList<theory::Evidence> &list);
  void errorOccurred(const QString &message);

private:
  QString _directory;
  QList<theory::Evidence> _items;

  QWidget *_widget;
  QRadioButton *_replace;
  QRadioButton *_append;
  QPushButton *_save;
  QPushButton *_load;
  QPushButton *_close;

private Q_SLOTS:
  void saveInventory();
  void loadInventory();
};
} // namespace spritechat
