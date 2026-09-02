#pragma once

#include <QKeyEvent>
#include <QLineEdit>
#include <QStack>
#include <QString>
#include <QWidget>

namespace spritechat
{
class AOLineEdit : public QLineEdit
{
  Q_OBJECT

public:
  explicit AOLineEdit(QWidget *parent = nullptr);

  int capacity() const;
  void setCapacity(int capacity);

  void record();

protected:
  void keyPressEvent(QKeyEvent *event) override;

private:
  int _capacity = 100;
  QStack<QString> _undo;
  QStack<QString> _redo;
  QString _currentMessage;

  void undoMessage();
  void redoMessage();
};
} // namespace spritechat
