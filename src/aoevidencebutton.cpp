#include "aoevidencebutton.h"

#include "file_functions.h"

#include <QPoint>

spritechat::AOEvidenceButton::AOEvidenceButton(theory::EvidenceId id, int width, int height, AOApplication *ao_app, QWidget *parent)
    : QPushButton(parent)
    , ao_app(ao_app)
    , m_id(id)
{
  resize(width, height);

  ui_hidden = new AOImage(ao_app, this);
  QPoint hidden_size = ao_app->get_button_spacing("evidence_hidden_size", "courtroom_design.ini");
  if (hidden_size.x() <= 0 || hidden_size.y() <= 0)
  {
    hidden_size = QPoint{width / 3, height / 3};
  }
  ui_hidden->resize(hidden_size.x(), hidden_size.y());
  ui_hidden->move(width - hidden_size.x(), height - hidden_size.y());
  if (!ui_hidden->setImage("evidence_hidden"))
  {
    ui_hidden->setStyleSheet("background-color: rgba(0, 0, 0, 96);");
  }
  ui_hidden->setAttribute(Qt::WA_TransparentForMouseEvents);
  ui_hidden->hide();

  ui_selected = new AOImage(ao_app, this);
  ui_selected->resize(width, height);
  ui_selected->setImage("evidence_selected");
  ui_selected->setAttribute(Qt::WA_TransparentForMouseEvents);
  ui_selected->hide();

  ui_selector = new AOImage(ao_app, this);
  ui_selector->resize(width, height);
  ui_selector->setImage("evidence_selector");
  ui_selector->setAttribute(Qt::WA_TransparentForMouseEvents);
  ui_selector->hide();

  connect(this, &AOEvidenceButton::clicked, this, &AOEvidenceButton::on_clicked);
}

void spritechat::AOEvidenceButton::setImage(const QString &fileName)
{
  QString image_path = ao_app->get_real_path(ao_app->get_evidence_path(fileName));
  if (file_exists(fileName))
  {
    setText("");
    setStyleSheet("QPushButton { border-image: url(\"" + fileName +
                  "\") 0 0 0 0 stretch stretch; }"
                  "QToolTip { color: #000000; background-color: #ffffff; border: 0px; }");
  }
  else if (file_exists(image_path))
  {
    setText("");
    setStyleSheet("QPushButton { border-image: url(\"" + image_path +
                  "\") 0 0 0 0 stretch stretch; }"
                  "QToolTip { color: #000000; background-color: #ffffff; border: 0px; }");
  }
  else
  {
    setText(fileName);
    setStyleSheet("QPushButton { border-image: url(); }"
                  "QToolTip { background-image: url(); color: #000000; "
                  "background-color: #ffffff; border: 0px; }");
  }
}

void spritechat::AOEvidenceButton::setThemeImage(const QString &fileName)
{
  QString theme_image_path = ao_app->get_real_path(ao_app->get_theme_path(fileName));
  QString default_image_path = ao_app->get_real_path(ao_app->get_theme_path(fileName, ao_app->default_theme));

  QString final_image_path;

  if (file_exists(theme_image_path))
  {
    final_image_path = theme_image_path;
  }
  else
  {
    final_image_path = default_image_path;
  }

  setImage(final_image_path);
}

void spritechat::AOEvidenceButton::setSelected(bool p_selected)
{
  if (p_selected)
  {
    ui_selected->show();
  }
  else
  {
    ui_selected->hide();
  }
}

void spritechat::AOEvidenceButton::setRevealed(bool revealed)
{
  ui_hidden->setVisible(!revealed);
}

void spritechat::AOEvidenceButton::on_clicked()
{
  Q_EMIT evidenceClicked(m_id);
}

void spritechat::AOEvidenceButton::mouseDoubleClickEvent(QMouseEvent *e)
{
  QPushButton::mouseDoubleClickEvent(e);
  Q_EMIT evidenceDoubleClicked(m_id);
}

void spritechat::AOEvidenceButton::enterEvent(QEnterEvent *e)
{
  ui_selector->show();

  Q_EMIT mouseoverUpdated(m_id, true);

  setFlat(false);
  QPushButton::enterEvent(e);
}

void spritechat::AOEvidenceButton::leaveEvent(QEvent *e)
{
  ui_selector->hide();

  Q_EMIT mouseoverUpdated(m_id, false);
  QPushButton::leaveEvent(e);
}
