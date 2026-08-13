#include "server_editor_dialog.h"

#include "ao_widget_lookup.h"
#include "debug_functions.h"
#include "options.h"

#include <QDebug>
#include <QFile>
#include <QUiLoader>
#include <QVBoxLayout>

const QString spritechat::ServerEditorDialog::UI_FILE_PATH = "favorite_server_dialog.ui";

spritechat::ServerEditorDialog::ServerEditorDialog(QWidget *parent)
    : QDialog(parent)
{
  setWindowIcon(QIcon(":/data/logo-client.png"));
  QUiLoader loader(this);
  QFile file(Options::getInstance().getUIAsset(UI_FILE_PATH));

  if (!file.open(QFile::ReadOnly))
  {
    qFatal("Unable to open file %s", qPrintable(file.fileName()));
    return;
  }
  ui_body = loader.load(&file, this);

  auto layout = new QVBoxLayout(this);
  layout->addWidget(ui_body);

  AOWidgetLookup l_ui{this};

  l_ui.find(ui_name, "name");
  l_ui.find(ui_hostname, "hostname");
  l_ui.find(ui_port, "port");
  l_ui.find(ui_description, "description");
  l_ui.find(ui_button_box, "button_box");

  l_ui.find(ui_legacy_edit, "legacy_edit");
  l_ui.find(ui_parse_legacy, "parse_legacy");

  connect(ui_parse_legacy, &QPushButton::released, this, &ServerEditorDialog::parseLegacyEntry);

  connect(ui_button_box, &QDialogButtonBox::accepted, this, &ServerEditorDialog::accept);
  connect(ui_button_box, &QDialogButtonBox::rejected, this, &ServerEditorDialog::reject);
}

spritechat::ServerEditorDialog::ServerEditorDialog(const ServerBookmark &server, QWidget *parent)
    : ServerEditorDialog(parent)
{
  ui_name->setText(server.name);
  ui_hostname->setText(server.address);
  ui_port->setValue(server.port);
  ui_description->setPlainText(server.description);
}

spritechat::ServerBookmark spritechat::ServerEditorDialog::currentServerBookmark() const
{
  ServerBookmark server;
  server.name = ui_name->text();
  server.address = ui_hostname->text();
  server.port = ui_port->value();
  server.description = ui_description->toPlainText();
  return server;
}

void spritechat::ServerEditorDialog::parseLegacyEntry()
{
  QStringList entry = ui_legacy_edit->text().split(":");
  if (entry.size() < 3)
  {
    call_warning("Invalid legacy server entry");
    return;
  }

  ui_hostname->setText(entry.at(0));
  ui_port->setValue(entry.at(1).toInt());
  ui_name->setText(entry.at(2));
}
