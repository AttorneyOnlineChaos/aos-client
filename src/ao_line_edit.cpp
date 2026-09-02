#include "ao_line_edit.h"

spritechat::AOLineEdit::AOLineEdit(QWidget *parent)
    : QLineEdit{parent}
{}

int spritechat::AOLineEdit::capacity() const
{
  return _capacity;
}

void spritechat::AOLineEdit::setCapacity(int capacity)
{
  _capacity = qMax(0, capacity);
  if (_capacity == 0)
  {
    _redo.clear();
  }
  while (_undo.size() > _capacity)
  {
    _undo.removeFirst();
  }
}

void spritechat::AOLineEdit::record()
{
  _redo.clear();

  const QString message = text();
  const bool blank = message.trimmed().isEmpty();
  const bool duplicate = !_undo.isEmpty() && _undo.top() == message;
  if (_capacity > 0 && !blank && !duplicate)
  {
    _undo.push(message);
    if (_undo.size() > _capacity)
    {
      _undo.removeFirst();
    }
  }
}

void spritechat::AOLineEdit::keyPressEvent(QKeyEvent *event)
{
  if (_capacity == 0 || (event->modifiers() & ~Qt::KeypadModifier))
  {
    QLineEdit::keyPressEvent(event);
    return;
  }

  switch (event->key())
  {
  default:
    QLineEdit::keyPressEvent(event);
    break;
  case Qt::Key_Up:
    undoMessage();
    event->accept();
    break;
  case Qt::Key_Down:
    redoMessage();
    event->accept();
    break;
  }
}

void spritechat::AOLineEdit::undoMessage()
{
  if (_undo.isEmpty())
  {
    return;
  }

  if (_redo.isEmpty())
  {
    _redo.push(text());
  }
  else
  {
    _redo.push(_currentMessage);
  }
  _currentMessage = _undo.pop();
  setText(_currentMessage);
  end(false);
}

void spritechat::AOLineEdit::redoMessage()
{
  if (_redo.isEmpty())
  {
    return;
  }

  _undo.push(_currentMessage);
  _currentMessage = _redo.pop();
  setText(_currentMessage);
  end(false);
}
