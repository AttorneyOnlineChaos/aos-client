#include "datatypes.h"

#include <QObject>
#include <QString>

QString spritechat::AreaInfo::displayName() const
{
  if (name.isEmpty())
  {
    return QObject::tr("Area #%1").arg(id);
  }
  return name;
}
