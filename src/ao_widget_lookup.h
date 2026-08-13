#pragma once

#include <QPointer>
#include <QString>
#include <QWidget>

namespace spritechat
{
class AOWidgetLookup
{
public:
  explicit AOWidgetLookup(QWidget *root)
      : _root{root}
  {}

  template <typename T>
  void find(T *&widget, const QString &name) const
  {
    widget = _root->findChild<T *>(name);
  }

  template <typename T>
  void find(QPointer<T> &widget, const QString &name) const
  {
    widget = _root->findChild<T *>(name);
  }

private:
  QWidget *_root = nullptr;
};
} // namespace spritechat
