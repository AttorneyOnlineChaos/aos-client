#include "widgets/evidence_panel.h"

#include "core/logging.h"
#include "file_functions.h"
#include "options.h"
#include "spritechat_defs.h"

#include <QDir>
#include <QFileDialog>
#include <QRect>
#include <QSignalBlocker>
#include <QTextCursor>
#include <QVariant>

#include <utility>

spritechat::EvidencePanel::EvidencePanel(Mode mode, AOApplication *app, const EvidenceRegistry &registry, QWidget *parent)
    : AOImage{app, parent}
    , _mode{mode}
    , ao_app{app}
    , _registry{registry}
{
  _switch = new AOButton(ao_app, this);
  _switch->setObjectName("ui_evidence_switch");

  _inventories = new QComboBox(this);
  _inventories->view()->setTextElideMode(Qt::ElideRight);
  _inventories->setToolTip(tr("Choose which inventory to look at."));
  _inventories->setObjectName("ui_evidence_inventory_dropdown");
  _inventories->addItem(tr("Everyone"));

  _name = new QLineEdit(this);
  _nameFilter = new AOLineEditFilter();
  _nameFilter->setParent(this);
  _name->installEventFilter(_nameFilter);
  _name->setAlignment(Qt::AlignCenter);
  _name->setFrame(false);
  _name->setObjectName("ui_evidence_name");
  _name->setReadOnly(true);
  _reveal = new AOButton(ao_app, this);
  _reveal->setObjectName("ui_evidence_reveal");

  _grid = new theory::NavigableGrid(this);
  _grid->setObjectName("ui_evidence_buttons");
  _gridNavigator = new theory::MousewheelGridNavigator(_grid);
  _buttonSize = ao_app->get_button_spacing("evidence_button_size", "courtroom_design.ini");
  _addEvidence = createAddEvidence();

  _left = new AOButton(ao_app, this);
  _left->setObjectName("ui_evidence_left");
  _right = new AOButton(ao_app, this);
  _right->setObjectName("ui_evidence_right");
  _present = new AOButton(ao_app, this);
  _present->setToolTip(tr("Present this piece of evidence to everyone on your next spoken message"));
  _present->setObjectName("ui_evidence_present");

  _file = new AOButton(ao_app, this);
  _file->setToolTip(tr("Save this inventory to a .json file, or load one into it."));
  _file->setObjectName("ui_evidence_save");

  _overlay = new AOImage(ao_app, this);
  _overlay->setObjectName("ui_evidence_overlay");

  _delete = new AOButton(ao_app, _overlay);
  _delete->setToolTip(tr("Destroy this piece of evidence"));
  _delete->setObjectName("ui_evidence_delete");
  _imageName = new QLineEdit(_overlay);
  _imageNameFilter = new AOLineEditFilter();
  _imageNameFilter->setParent(this);
  _imageName->installEventFilter(_imageNameFilter);
  _imageName->setObjectName("ui_evidence_image_name");
  _imageButton = new AOButton(ao_app, _overlay);
  _imageButton->setText(tr("Choose.."));
  _imageButton->setObjectName("ui_evidence_image_button");
  _x = new AOButton(ao_app, _overlay);
  _x->setToolTip(tr("Close the evidence display/editing overlay.\n"
                    "You will be prompted if there's any unsaved changes."));
  _x->setObjectName("ui_evidence_x");
  _ok = new AOButton(ao_app, _overlay);
  _ok->setToolTip(tr("Save any changes made to this piece of evidence and send them to server."));
  _ok->setObjectName("ui_evidence_ok");

  _description = new QPlainTextEdit(_overlay);
  _description->setFrameStyle(QFrame::NoFrame);
  _description->setToolTip(tr("Click to edit. Press [X] to update your changes."));
  _description->setObjectName("ui_evidence_description");

  connect(&_registry, &EvidenceRegistry::added, this, &EvidencePanel::syncEvidence);
  connect(&_registry, &EvidenceRegistry::updated, this, &EvidencePanel::syncEvidence);
  connect(&_registry, &EvidenceRegistry::removed, this, &EvidencePanel::dropEvidence);
  connect(&_registry, &EvidenceRegistry::cleared, this, &EvidencePanel::dropAllEvidence);

  connect(_switch, &AOButton::clicked, this, &EvidencePanel::switchRequested);
  connect(_inventories, &QComboBox::currentIndexChanged, this, &EvidencePanel::selectInventory);
  connect(_left, &AOButton::clicked, _grid, &theory::NavigableGrid::previousPage);
  connect(_right, &AOButton::clicked, _grid, &theory::NavigableGrid::nextPage);
  connect(_grid, &theory::NavigableGrid::layoutUpdated, this, &EvidencePanel::updateNavigationArrows);
  connect(_grid, &theory::NavigableGrid::currentPageChanged, this, &EvidencePanel::updateNavigationArrows);
  connect(_present, &AOButton::clicked, this, &EvidencePanel::togglePresenting);
  connect(_file, &AOButton::clicked, this, &EvidencePanel::fileRequested);
  connect(_reveal, &AOButton::clicked, this, &EvidencePanel::toggleReveal);

  connect(_delete, &AOButton::clicked, this, &EvidencePanel::removeSelectedEvidence);
  connect(_imageButton, &AOButton::clicked, this, &EvidencePanel::chooseEvidenceImage);
  connect(_x, &AOButton::clicked, this, [this] { confirmOverlayClose(); });
  connect(_ok, &AOButton::clicked, this, &EvidencePanel::saveEvidence);

  connect(_name, &QLineEdit::textChanged, this, &EvidencePanel::refreshSaveButton);
  connect(_imageName, &QLineEdit::textChanged, this, &EvidencePanel::refreshSaveButton);
  connect(_description, &QPlainTextEdit::textChanged, this, &EvidencePanel::refreshSaveButton);

  _file->hide();
  _reveal->hide();
  _overlay->hide();
  hide();
}

spritechat::EvidencePanel::Mode spritechat::EvidencePanel::mode() const
{
  return _mode;
}

void spritechat::EvidencePanel::applyTheme()
{
  updateSizeAndPosition(this, "evidence_background");
  updateSizeAndPosition(_switch, "evidence_switch");
  updateSizeAndPosition(_overlay, "evidence_overlay");
  if (_mode == Mode::Public)
  {
    setImage("evidence_background");
    _overlay->setImage("evidence_overlay");
    _switch->setImage("evidence_global");
    _switch->setToolTip(tr("Viewing public evidence. Click to switch to private."));
  }
  else
  {
    setImage("evidence_background_private");
    _overlay->setImage("evidence_overlay_private");
    _switch->setImage("evidence_private");
    _switch->setToolTip(tr("Viewing private evidence. Click to switch to public."));
  }

  updateSizeAndPosition(_inventories, "evidence_inventory_dropdown");
  setShown(_inventories, true);

  updateSizeAndPosition(_name, "evidence_name");
  updateSizeAndPosition(_reveal, "evidence_reveal");
  int inset = 0;
  if (!_unthemed.contains(_reveal) && _reveal->geometry().intersects(_name->geometry()))
  {
    const QRect bar = _name->geometry();
    const QRect toggle = _reveal->geometry();
    inset = qMin(toggle.right() - bar.left(), bar.right() - toggle.left()) + 1;
  }
  _name->setTextMargins(inset, 0, inset, 0);
  updateSizeAndPosition(_grid, "evidence_buttons");
  const QPoint spacing = ao_app->get_button_spacing("evidence_button_spacing", "courtroom_design.ini");
  _grid->setHorizontalSpacing(spacing.x());
  _grid->setVerticalSpacing(spacing.y());

  updateSizeAndPosition(_left, "evidence_left");
  _left->setImage("arrow_left");
  updateSizeAndPosition(_right, "evidence_right");
  _right->setImage("arrow_right");

  updateSizeAndPosition(_present, "evidence_present");
  if (_presenting)
  {
    _present->setImage("present_disabled");
  }
  else
  {
    _present->setImage("present");
  }

  updateSizeAndPosition(_delete, "evidence_delete");
  _delete->setImage("evidence_delete");
  updateSizeAndPosition(_imageName, "evidence_image_name");
  updateSizeAndPosition(_imageButton, "evidence_image_button");
  updateSizeAndPosition(_x, "evidence_x");
  _x->setImage("evidence_x");
  updateSizeAndPosition(_ok, "evidence_ok");
  _ok->setImage("evidence_ok");
  updateSizeAndPosition(_description, "evidence_description");

  updateSizeAndPosition(_file, "evidence_save");
  _file->setImage("evidence_save");

  _buttonSize = ao_app->get_button_spacing("evidence_button_size", "courtroom_design.ini");
  _addEvidence->hide();
  _addEvidence->deleteLater();
  _addEvidence = createAddEvidence();
  rebuildButtons();
}

QLineEdit *spritechat::EvidencePanel::nameLine() const
{
  return _name;
}

QLineEdit *spritechat::EvidencePanel::imageLine() const
{
  return _imageName;
}

QPlainTextEdit *spritechat::EvidencePanel::descriptionEdit() const
{
  return _description;
}

void spritechat::EvidencePanel::setInventories(const QList<InventoryChoice> &inventories)
{
  bool unchanged = _inventories->count() == inventories.size() + 1;
  for (int index = 0; unchanged && index < inventories.size(); ++index)
  {
    const InventoryChoice &choice = inventories.at(index);
    unchanged = _inventories->itemData(index + 1).toInt() == choice.id && _inventories->itemText(index + 1) == choice.label;
  }
  if (unchanged)
  {
    return;
  }

  const QSignalBlocker blocker{_inventories};
  _inventories->clear();
  _inventories->addItem(tr("Everyone"));
  for (const InventoryChoice &choice : inventories)
  {
    _inventories->addItem(choice.label, choice.id);
  }

  int index = 0;
  if (_inventory != theory::NoInventoryId)
  {
    index = _inventories->findData(_inventory);
  }
  if (index < 0)
  {
    _inventories->setCurrentIndex(0);
    selectTab(theory::NoInventoryId);
    return;
  }
  _inventories->setCurrentIndex(index);
  display();
}

void spritechat::EvidencePanel::showItem(theory::EvidenceId id)
{
  const theory::InventoryId inventory = _inventoryOf.value(id, theory::NoInventoryId);
  if (_inventory != theory::NoInventoryId && _inventory != inventory)
  {
    const QSignalBlocker blocker{_inventories};
    _inventories->setCurrentIndex(0);
    selectTab(theory::NoInventoryId);
  }

  _grid->showPageOf(buttonFor(id));
  openOverlay(id);
}

void spritechat::EvidencePanel::closeOverlay()
{
  _description->setReadOnly(true);
  _name->setReadOnly(true);
  _imageName->setReadOnly(true);
  _reveal->hide();
  _overlay->hide();
}

void spritechat::EvidencePanel::resetSelection()
{
  closeOverlay();
  _selected = theory::NoEvidenceId;
  _grid->setCurrentPage(0);
  _name->setText("");
  stopPresenting();
  refreshSelection();
}

std::optional<theory::EvidenceId> spritechat::EvidencePanel::presentedEvidence() const
{
  if (!_presenting || _selected == theory::NoEvidenceId)
  {
    return std::nullopt;
  }
  return _selected;
}

void spritechat::EvidencePanel::stopPresenting()
{
  _presenting = false;
  _present->setImage("present");
}

bool spritechat::EvidencePanel::editable() const
{
  return _mode == Mode::Private;
}

void spritechat::EvidencePanel::updateSizeAndPosition(QWidget *widget, const QString &key)
{
  const pos_size_type geometry = ao_app->get_element_dimensions(key, "courtroom_design.ini");
  if (geometry.width < 0 || geometry.height < 0)
  {
    zWarning(log::ui) << "could not find" << key << "in courtroom_design.ini";
    _unthemed.insert(widget);
    widget->hide();
    return;
  }

  _unthemed.remove(widget);
  widget->move(geometry.x, geometry.y);
  widget->resize(geometry.width, geometry.height);
}

void spritechat::EvidencePanel::setShown(QWidget *widget, bool shown)
{
  widget->setVisible(shown && !_unthemed.contains(widget));
}

spritechat::AOEvidenceButton *spritechat::EvidencePanel::createButton(theory::EvidenceId id)
{
  AOEvidenceButton *button = new AOEvidenceButton(id, _buttonSize.x(), _buttonSize.y(), ao_app, _grid);
  button->hide();
  connect(button, &AOEvidenceButton::evidenceClicked, this, &EvidencePanel::selectEvidence);
  connect(button, &AOEvidenceButton::evidenceDoubleClicked, this, &EvidencePanel::openOverlay);
  connect(button, &AOEvidenceButton::mouseoverUpdated, this, &EvidencePanel::previewEvidence);
  return button;
}

spritechat::AOEvidenceButton *spritechat::EvidencePanel::createAddEvidence()
{
  AOEvidenceButton *button = new AOEvidenceButton(theory::NoEvidenceId, _buttonSize.x(), _buttonSize.y(), ao_app, _grid);
  button->hide();
  button->setThemeImage("addevidence.png");
  button->setRevealed(true);
  connect(button, &AOEvidenceButton::clicked, this, &EvidencePanel::addRequested);
  connect(button, &AOEvidenceButton::mouseoverUpdated, this, [this](theory::EvidenceId, bool hovering) { previewAddEvidence(hovering); });
  return button;
}

spritechat::AOEvidenceButton *spritechat::EvidencePanel::buttonFor(theory::EvidenceId id) const
{
  return _byInventory.value(_inventoryOf.value(id, theory::NoInventoryId)).value(id, nullptr);
}

void spritechat::EvidencePanel::fileEvidence(const EvidenceInfo &item)
{
  if (_mode == Mode::Public && !item.evidence.revealed)
  {
    unfileEvidence(item.id);
    return;
  }

  AOEvidenceButton *button = buttonFor(item.id);
  if (!button)
  {
    button = createButton(item.id);
    _byInventory[item.inventoryId].insert(item.id, button);
    _inventoryOf.insert(item.id, item.inventoryId);
  }
  button->setImage(item.evidence.image);
  button->setRevealed(item.evidence.revealed);

  if (item.id == _selected && _overlay->isVisible() && item.evidence != _overlayBase)
  {
    if (_ok->isHidden())
    {
      fillOverlay(item.evidence);
      _ok->hide();
    }
    else
    {
      promptOverlayChange(item);
    }
  }
}

void spritechat::EvidencePanel::unfileEvidence(theory::EvidenceId id)
{
  const auto filed = _inventoryOf.constFind(id);
  if (filed == _inventoryOf.cend())
  {
    return;
  }
  const theory::InventoryId inventory = filed.value();
  _inventoryOf.erase(filed);

  auto &buttons = _byInventory[inventory];
  AOEvidenceButton *button = buttons.take(id);
  if (buttons.isEmpty())
  {
    _byInventory.remove(inventory);
  }
  button->hide();
  button->deleteLater();

  if (id == _selected)
  {
    closeOverlay();
    _selected = theory::NoEvidenceId;
    _name->setText("");
    stopPresenting();
  }
}

void spritechat::EvidencePanel::clearButtons()
{
  for (const auto &buttons : std::as_const(_byInventory))
  {
    for (AOEvidenceButton *button : buttons)
    {
      button->hide();
      button->deleteLater();
    }
  }
  _byInventory.clear();
  _inventoryOf.clear();
}

void spritechat::EvidencePanel::rebuildButtons()
{
  clearButtons();
  for (const EvidenceInfo &item : _registry.evidence())
  {
    fileEvidence(item);
  }
  display();
}

void spritechat::EvidencePanel::appendButtons(QList<QWidget *> &widgets, const QMap<theory::EvidenceId, AOEvidenceButton *> &buttons)
{
  for (auto it = buttons.cbegin(); it != buttons.cend(); ++it)
  {
    if (const auto item = _registry.evidence(it.key()))
    {
      it.value()->setToolTip(QString::number(widgets.size() + 1) + ": " + item->evidence.name);
    }
    widgets.append(it.value());
  }
}

void spritechat::EvidencePanel::display()
{
  QList<QWidget *> widgets;
  if (_inventory == theory::NoInventoryId)
  {
    for (int index = 1; index < _inventories->count(); ++index)
    {
      appendButtons(widgets, _byInventory.value(_inventories->itemData(index).toInt()));
    }
  }
  else
  {
    appendButtons(widgets, _byInventory.value(_inventory));
  }

  const bool specific = editable() && _inventory != theory::NoInventoryId;
  if (specific)
  {
    widgets.append(_addEvidence);
  }
  if (widgets != _grid->widgets())
  {
    _grid->setWidgets(widgets);
  }
  setShown(_file, specific);
  updateNavigationArrows();
  refreshSelection();
}

void spritechat::EvidencePanel::selectTab(theory::InventoryId inventory)
{
  _inventory = inventory;
  resetSelection();
  display();
  Q_EMIT inventorySelected(_inventory);
}

void spritechat::EvidencePanel::refreshSelection()
{
  for (const auto &buttons : std::as_const(_byInventory))
  {
    for (auto it = buttons.cbegin(); it != buttons.cend(); ++it)
    {
      it.value()->setSelected(it.key() == _selected);
    }
  }
}

void spritechat::EvidencePanel::restoreSelectedName()
{
  if (const auto item = _registry.evidence(_selected))
  {
    _name->setText(item->evidence.name);
  }
  else
  {
    _name->setText("");
  }
}

void spritechat::EvidencePanel::openOverlay(theory::EvidenceId id)
{
  if (_overlay->isVisible() && !confirmOverlayClose())
  {
    return;
  }

  if (const auto item = _registry.evidence(id))
  {
    _selected = id;
    fillOverlay(item->evidence);

    show();
    _overlay->show();
    _ok->hide();
    refreshSelection();
  }
}

void spritechat::EvidencePanel::fillOverlay(const theory::Evidence &evidence)
{
  QString editHint;
  if (editable())
  {
    editHint = tr("Click to edit...");
  }

  _description->clear();
  _description->appendPlainText(evidence.description);
  _description->setReadOnly(!editable());
  _description->setToolTip(editHint);
  _description->moveCursor(QTextCursor::Start);

  _name->setText(evidence.name);
  _name->setReadOnly(!editable());
  _name->setToolTip(editHint);

  _imageName->setText(evidence.image);
  _imageName->setReadOnly(!editable());
  _imageName->setToolTip(editHint);

  _overlayBase = evidence;

  setShown(_delete, editable());
  setShown(_imageButton, editable());
  setShown(_reveal, editable());
  setRevealState(evidence.revealed);
}

void spritechat::EvidencePanel::setRevealState(bool revealed)
{
  _overlayRevealed = revealed;
  if (revealed)
  {
    _reveal->setImage("evidence_reveal");
    _reveal->setToolTip(tr("This piece of evidence is revealed to everyone in the area. Click to hide it.\n"
                           "Presenting evidence reveals it, even while it is hidden."));
  }
  else
  {
    _reveal->setImage("evidence_conceal");
    _reveal->setToolTip(tr("This piece of evidence is hidden from everyone else. Click to reveal it.\n"
                           "Presenting evidence reveals it, even while it is hidden."));
  }
}

bool spritechat::EvidencePanel::confirmOverlayClose()
{
  if (_ok->isHidden())
  {
    closeOverlay();
    return true;
  }

  QMessageBox prompt{this};
  prompt.setText(tr("Evidence has been modified."));
  prompt.setInformativeText(tr("Do you want to save your changes?"));
  prompt.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
  prompt.setDefaultButton(QMessageBox::Save);
  switch (prompt.exec())
  {
  default:
  case QMessageBox::Cancel:
    return false;
  case QMessageBox::Save:
    closeOverlay();
    saveEvidence();
    return true;
  case QMessageBox::Discard:
    closeOverlay();
    return true;
  }
}

void spritechat::EvidencePanel::promptOverlayChange(const EvidenceInfo &latest)
{
  if (_changePrompt)
  {
    return;
  }

  QMessageBox *prompt = new QMessageBox(this);
  prompt->setAttribute(Qt::WA_DeleteOnClose);
  prompt->setText(tr("The piece of evidence you've been editing has changed."));
  prompt->setInformativeText(tr("Do you wish to keep your changes?"));
  QString revealed = tr("no");
  if (latest.evidence.revealed)
  {
    revealed = tr("yes");
  }
  prompt->setDetailedText(tr("Name: %1\n"
                             "Image: %2\n"
                             "Revealed: %3\n"
                             "Description:\n%4")
                              .arg(latest.evidence.name, latest.evidence.image, revealed, latest.evidence.description));
  prompt->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
  prompt->setDefaultButton(QMessageBox::LastButton);
  connect(prompt, &QMessageBox::finished, this, [this, id = latest.id](int result) {
    if (result != QMessageBox::No || id != _selected || _overlay->isHidden())
    {
      return;
    }
    if (const auto current = _registry.evidence(id))
    {
      fillOverlay(current->evidence);
      _ok->hide();
    }
  });
  _changePrompt = prompt;
  prompt->open();
}

void spritechat::EvidencePanel::syncEvidence(theory::EvidenceId id)
{
  const auto item = _registry.evidence(id);
  if (!item || item->inventoryId == theory::NoInventoryId)
  {
    return;
  }

  fileEvidence(item.value());
  if (_inventory == theory::NoInventoryId || _inventory == item->inventoryId)
  {
    display();
  }
}

void spritechat::EvidencePanel::dropEvidence(theory::EvidenceId id)
{
  const theory::InventoryId inventory = _inventoryOf.value(id, theory::NoInventoryId);
  if (inventory == theory::NoInventoryId)
  {
    return;
  }

  unfileEvidence(id);
  if (_inventory == theory::NoInventoryId || _inventory == inventory)
  {
    display();
  }
}

void spritechat::EvidencePanel::dropAllEvidence()
{
  clearButtons();
  resetSelection();
  display();
}

void spritechat::EvidencePanel::selectEvidence(theory::EvidenceId id)
{
  if (!Options::getInstance().evidenceDoubleClickEdit())
  {
    openOverlay(id);
    return;
  }

  if (_overlay->isVisible())
  {
    return;
  }

  if (const auto item = _registry.evidence(id))
  {
    _selected = id;
    _name->setText(item->evidence.name);
    refreshSelection();
  }
}

void spritechat::EvidencePanel::previewEvidence(theory::EvidenceId id, bool hovering)
{
  if (_overlay->isVisible())
  {
    return;
  }

  if (!hovering)
  {
    restoreSelectedName();
    return;
  }
  if (const auto item = _registry.evidence(id))
  {
    _name->setText(item->evidence.name);
  }
}

void spritechat::EvidencePanel::previewAddEvidence(bool hovering)
{
  if (_overlay->isVisible())
  {
    return;
  }

  if (hovering)
  {
    _name->setText(tr("Add new evidence..."));
  }
  else
  {
    restoreSelectedName();
  }
}

void spritechat::EvidencePanel::updateNavigationArrows()
{
  setShown(_left, _grid->currentPage() > 0);
  setShown(_right, _grid->currentPage() + 1 < _grid->pageCount());
}

void spritechat::EvidencePanel::setMousewheelDirection(theory::MousewheelGridNavigator::Direction direction)
{
  _gridNavigator->setDirection(direction);
}

void spritechat::EvidencePanel::togglePresenting()
{
  _presenting = !_presenting;
  if (_presenting)
  {
    _present->setImage("present_disabled");
  }
  else
  {
    _present->setImage("present");
  }
  Q_EMIT presentingChanged(_presenting);
}

void spritechat::EvidencePanel::selectInventory(int index)
{
  const QVariant data = _inventories->itemData(index);
  theory::InventoryId inventory = theory::NoInventoryId;
  if (data.isValid())
  {
    inventory = data.toInt();
  }
  if (inventory == _inventory)
  {
    return;
  }
  selectTab(inventory);
}

void spritechat::EvidencePanel::removeSelectedEvidence()
{
  closeOverlay();
  if (!editable() || _selected == theory::NoEvidenceId)
  {
    return;
  }

  const theory::EvidenceId removed = _selected;
  _selected = theory::NoEvidenceId;
  refreshSelection();
  Q_EMIT removeRequested(removed);
}

void spritechat::EvidencePanel::toggleReveal()
{
  if (!editable())
  {
    return;
  }
  setRevealState(!_overlayRevealed);
  refreshSaveButton();
}

void spritechat::EvidencePanel::saveEvidence()
{
  _ok->hide();
  const auto item = _registry.evidence(_selected);
  if (!item || !editable())
  {
    return;
  }

  theory::Evidence evidence = item->evidence;
  evidence.name = _name->text();
  evidence.description = _description->toPlainText();
  evidence.image = _imageName->text();
  evidence.revealed = _overlayRevealed;
  Q_EMIT snapshotRequested(item->id, evidence);
}

void spritechat::EvidencePanel::chooseEvidenceImage()
{
  QDir dir(get_base_path() + "/evidence/");
  QFileDialog dialog(this);
  dialog.setFileMode(QFileDialog::ExistingFile);
  dialog.setNameFilter(tr("Images (*.png)"));
  dialog.setViewMode(QFileDialog::List);
  dialog.setDirectory(dir);

  QStringList filenames;
  if (dialog.exec())
  {
    filenames = dialog.selectedFiles();
  }
  if (filenames.size() != 1)
  {
    return;
  }

  QString filename = filenames.at(0);
  QStringList bases = Options::getInstance().mountPaths();
  bases.prepend(get_base_path());
  for (const QString &base : bases)
  {
    QDir baseDir(base);
    if (filename.startsWith(baseDir.absolutePath() + "/"))
    {
      dir.setPath(baseDir.absolutePath() + "/evidence");
      break;
    }
  }
  _imageName->setText(dir.relativeFilePath(filename));
}

void spritechat::EvidencePanel::refreshSaveButton()
{
  const auto item = _registry.evidence(_selected);
  if (!item)
  {
    return;
  }

  theory::Evidence edited = item->evidence;
  edited.name = _name->text();
  edited.description = _description->toPlainText();
  edited.image = _imageName->text();
  edited.revealed = _overlayRevealed;
  setShown(_ok, editable() && edited != item->evidence);
}
