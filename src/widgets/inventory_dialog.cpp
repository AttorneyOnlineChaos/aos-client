#include "inventory_dialog.h"

#include "ao_widget_lookup.h"
#include "core/logging.h"
#include "options.h"
#include "spritechat_defs.h"

#include <QFile>
#include <QFileDialog>
#include <QUiLoader>
#include <QVBoxLayout>

spritechat::InventoryDialog::InventoryDialog(const QString &directory, QWidget *parent)
    : QDialog{parent}
    , _directory{directory}
{
  QUiLoader loader(this);
  QFile file(Options::getInstance().getUIAsset(QStringLiteral("inventory_dialog.ui")));
  if (!file.open(QFile::ReadOnly))
  {
    zCritical(log::ui) << "Unable to open file " << file.fileName();
    return;
  }
  _widget = loader.load(&file, this);

  auto layout = new QVBoxLayout(this);
  layout->addWidget(_widget);

  AOWidgetLookup lookup{this};
  lookup.find(_replace, "replace");
  lookup.find(_append, "append");
  lookup.find(_save, "save");
  lookup.find(_load, "load");
  lookup.find(_close, "close");

  connect(_save, &QPushButton::clicked, this, &InventoryDialog::saveInventory);
  connect(_load, &QPushButton::clicked, this, &InventoryDialog::loadInventory);
  connect(_close, &QPushButton::clicked, this, &InventoryDialog::reject);
}

void spritechat::InventoryDialog::setInventory(const QString &label, const QList<theory::Evidence> &items)
{
  setWindowTitle(tr("Inventory: %1").arg(label));
  _items = items;
}

theory::InventoryTransferPacket::Mode spritechat::InventoryDialog::transferMode() const
{
  if (_append->isChecked())
  {
    return theory::InventoryTransferPacket::Append;
  }
  return theory::InventoryTransferPacket::Replace;
}

void spritechat::InventoryDialog::saveInventory()
{
  const QString path = QFileDialog::getSaveFileName(this, tr("Save Inventory"), _directory, tr("JSON Files (*.json)"));
  if (path.isEmpty())
  {
    return;
  }

  if (const auto error = theory::saveEvidence(path, _items))
  {
    Q_EMIT errorOccurred(tr("Failed to save inventory: %1").arg(error->toString()));
  }
}

void spritechat::InventoryDialog::loadInventory()
{
  const QString path = QFileDialog::getOpenFileName(this, tr("Open Inventory"), _directory, tr("JSON Files (*.json)"));
  if (path.isEmpty())
  {
    return;
  }

  QList<theory::Evidence> list;
  if (const auto error = theory::loadEvidence(path, list))
  {
    Q_EMIT errorOccurred(tr("Failed to load inventory: %1").arg(error->toString()));
    return;
  }
  Q_EMIT transferRequested(transferMode(), list);
  accept();
}
