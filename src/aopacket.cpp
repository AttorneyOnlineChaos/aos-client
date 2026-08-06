#include "aopacket.h"

QString spritechat::AOPacket::encode(QString data)
{
  return data.replace("#", "<num>").replace("%", "<percent>").replace("$", "<dollar>").replace("&", "<and>");
}

QString spritechat::AOPacket::decode(QString data)
{
  return data.replace("<num>", "#").replace("<percent>", "%").replace("<dollar>", "$").replace("<and>", "&");
}

spritechat::AOPacket::AOPacket()
{}

spritechat::AOPacket::AOPacket(QString header)
    : m_header(header)
{}

spritechat::AOPacket::AOPacket(QString header, QStringList content)
    : m_header(header)
    , m_content(content)
{}

QString spritechat::AOPacket::header()
{
  return m_header;
}

QStringList &spritechat::AOPacket::content()
{
  return m_content;
}

QString spritechat::AOPacket::toString(bool ensureEncoded)
{
  QString message = m_header;
  if (!m_content.isEmpty())
  {
    for (QString item : std::as_const(m_content))
    {
      if (ensureEncoded)
      {
        item = encode(item);
      }
      message += "#" + item;
    }
  }

  return message + "#%";
}
