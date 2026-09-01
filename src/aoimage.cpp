#include "aoimage.h"

#include "core/logging.h"
#include "file_functions.h"
#include "options.h"
#include "spritechat_defs.h"

#include <QBitmap>

spritechat::AOImage::AOImage(AOApplication *ao_app, QWidget *parent)
    : QLabel(parent)
    , ao_app(ao_app)
{}

QString spritechat::AOImage::image()
{
  return m_file_name;
}

bool spritechat::AOImage::setImage(const QString &fileName, const QString &miscellaneous)
{
  QString p_image_resolved = ao_app->get_image(fileName, Options::getInstance().theme(), Options::getInstance().subTheme(), ao_app->default_theme, miscellaneous, "", "", false);

  if (!file_exists(p_image_resolved))
  {
    zWarning(log::asset) << "could not find image" << fileName;
    return false;
  }

  m_file_name = p_image_resolved;
  QPixmap f_pixmap(m_file_name);
  f_pixmap = f_pixmap.scaled(size(), Qt::IgnoreAspectRatio);
  setPixmap(f_pixmap);

  return true;
}

bool spritechat::AOImage::setImage(const QString &fileName)
{
  return setImage(fileName, QString());
}
