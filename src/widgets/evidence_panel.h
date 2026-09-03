#pragma once

#include "aoapplication.h"
#include "aobutton.h"
#include "aoevidencebutton.h"
#include "aoimage.h"
#include "datatypes.h"
#include "eventfilters.h"
#include "evidence_registry.h"
#include "game/evidence.h"
#include "game/game_defs.h"
#include "widgets/mousewheel_grid_navigator.h"
#include "widgets/navigable_grid.h"

#include <QComboBox>
#include <QHash>
#include <QLineEdit>
#include <QList>
#include <QMap>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPoint>
#include <QPointer>
#include <QSet>
#include <QString>
#include <QWidget>

#include <optional>

namespace spritechat
{
class EvidencePanel : public AOImage
{
  Q_OBJECT

public:
  enum class Mode
  {
    Public,
    Private,
  };

  struct InventoryChoice
  {
    theory::InventoryId id = theory::NoInventoryId;
    QString label;
  };

  EvidencePanel(Mode mode, AOApplication *app, const EvidenceRegistry &registry, QWidget *parent = nullptr);

  Mode mode() const;

  void applyTheme();
  QLineEdit *nameLine() const;
  QLineEdit *imageLine() const;
  QPlainTextEdit *descriptionEdit() const;

  void setInventories(const QList<InventoryChoice> &inventories);

  void setMousewheelDirection(theory::MousewheelGridNavigator::Direction direction);

  void showItem(theory::EvidenceId id);
  void closeOverlay();
  void resetSelection();

  std::optional<theory::EvidenceId> presentedEvidence() const;
  void stopPresenting();

Q_SIGNALS:
  void switchRequested();
  void inventorySelected(theory::InventoryId id);
  void addRequested();
  void removeRequested(theory::EvidenceId id);
  void snapshotRequested(theory::EvidenceId id, const theory::Evidence &evidence);
  void fileRequested();
  void presentingChanged(bool presenting);

private:
  Mode _mode;
  AOApplication *ao_app;
  const EvidenceRegistry &_registry;

  QMap<theory::InventoryId, QMap<theory::EvidenceId, AOEvidenceButton *>> _byInventory;
  QHash<theory::EvidenceId, theory::InventoryId> _inventoryOf;
  theory::InventoryId _inventory = theory::NoInventoryId;
  QPoint _buttonSize;
  theory::EvidenceId _selected = theory::NoEvidenceId;
  bool _presenting = false;
  theory::Evidence _overlayBase;
  bool _overlayRevealed = false;
  QPointer<QMessageBox> _changePrompt;
  QSet<QWidget *> _unthemed;

  AOButton *_switch;
  QComboBox *_inventories;
  QLineEdit *_name;
  AOLineEditFilter *_nameFilter;
  AOButton *_reveal;
  theory::NavigableGrid *_grid;
  theory::MousewheelGridNavigator *_gridNavigator;
  AOEvidenceButton *_addEvidence;
  AOButton *_left;
  AOButton *_right;
  AOButton *_present;
  AOButton *_file;
  AOImage *_overlay;
  AOButton *_delete;
  QLineEdit *_imageName;
  AOLineEditFilter *_imageNameFilter;
  AOButton *_imageButton;
  AOButton *_x;
  AOButton *_ok;
  QPlainTextEdit *_description;

  bool editable() const;
  void updateSizeAndPosition(QWidget *widget, const QString &key);
  void setShown(QWidget *widget, bool shown);

  AOEvidenceButton *createButton(theory::EvidenceId id);
  AOEvidenceButton *createAddEvidence();
  AOEvidenceButton *buttonFor(theory::EvidenceId id) const;
  void fileEvidence(const EvidenceInfo &item);
  void unfileEvidence(theory::EvidenceId id);
  void clearButtons();
  void rebuildButtons();
  void appendButtons(QList<QWidget *> &widgets, const QMap<theory::EvidenceId, AOEvidenceButton *> &buttons);
  void display();
  void selectTab(theory::InventoryId inventory);
  void refreshSelection();
  void restoreSelectedName();

  void openOverlay(theory::EvidenceId id);
  void fillOverlay(const theory::Evidence &evidence);
  void setRevealState(bool revealed);
  bool confirmOverlayClose();
  void promptOverlayChange(const EvidenceInfo &latest);

private Q_SLOTS:
  void syncEvidence(theory::EvidenceId id);
  void dropEvidence(theory::EvidenceId id);
  void dropAllEvidence();
  void selectEvidence(theory::EvidenceId id);
  void previewEvidence(theory::EvidenceId id, bool hovering);
  void previewAddEvidence(bool hovering);
  void updateNavigationArrows();
  void togglePresenting();
  void selectInventory(int index);
  void removeSelectedEvidence();
  void toggleReveal();
  void saveEvidence();
  void chooseEvidenceImage();
  void refreshSaveButton();
};
} // namespace spritechat
