#include "ao_plain_text_edit.h"

#include <QChar>
#include <QString>
#include <QTextCursor>
#include <QTextDocument>

spritechat::AOPlainTextEdit::AOPlainTextEdit(QWidget *parent)
    : QPlainTextEdit{parent}
{
  setLineWrapMode(QPlainTextEdit::NoWrap);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setTabChangesFocus(true);
  document()->setDocumentMargin(2);
}

int spritechat::AOPlainTextEdit::capacity() const
{
  return _history.capacity();
}

void spritechat::AOPlainTextEdit::setCapacity(int capacity)
{
  _history.setCapacity(capacity);
}

void spritechat::AOPlainTextEdit::record()
{
  _history.record(toPlainText());
}

void spritechat::AOPlainTextEdit::keyPressEvent(QKeyEvent *event)
{
  const bool browsing = _history.capacity() > 0 && !(event->modifiers() & ~Qt::KeypadModifier);
  switch (event->key())
  {
  default:
    QPlainTextEdit::keyPressEvent(event);
    break;
  case Qt::Key_Return:
  case Qt::Key_Enter:
    event->accept();
    Q_EMIT returnPressed();
    break;
  case Qt::Key_Up:
    if (!browsing)
    {
      QPlainTextEdit::keyPressEvent(event);
      break;
    }

    if (const auto message = _history.undo(toPlainText()))
    {
      setPlainText(message.value());
      moveCursor(QTextCursor::End);
    }

    event->accept();
    break;
  case Qt::Key_Down:
    if (!browsing)
    {
      QPlainTextEdit::keyPressEvent(event);
      break;
    }

    if (const auto message = _history.redo())
    {
      setPlainText(message.value());
      moveCursor(QTextCursor::End);
    }

    event->accept();
    break;
  }
}

void spritechat::AOPlainTextEdit::insertFromMimeData(const QMimeData *source)
{
  QString text = source->text();
  text.replace(QStringLiteral("\r\n"), QStringLiteral(" "));
  text.replace(QChar{QChar::CarriageReturn}, QChar{QChar::Space});
  text.replace(QChar{QChar::LineFeed}, QChar{QChar::Space});
  text.replace(QChar{QChar::LineSeparator}, QChar{QChar::Space});
  text.replace(QChar{QChar::ParagraphSeparator}, QChar{QChar::Space});
  insertPlainText(text);
}
