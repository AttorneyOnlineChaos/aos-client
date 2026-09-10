#include "ao_syntax_highlighter.h"

#include <QTextBoundaryFinder>

spritechat::AOSyntaxHighlighter::AOSyntaxHighlighter(QTextDocument *document)
    : QSyntaxHighlighter{document}
{}

void spritechat::AOSyntaxHighlighter::setMarkup(const QList<theory::ChatMarkup> &markup)
{
  _markup = markup;
  rehighlight();
}

void spritechat::AOSyntaxHighlighter::setDefaultColor(int color)
{
  _defaultColor = color;
  rehighlight();
}

void spritechat::AOSyntaxHighlighter::highlightBlock(const QString &text)
{
  if (_defaultColor < 0 || _defaultColor >= _markup.size())
  {
    return;
  }

  QList<int> stack{_defaultColor};
  bool escaping = false;
  int position = 0;
  QTextBoundaryFinder finder(QTextBoundaryFinder::Grapheme, text);

  auto markupAt = [&text, &finder, &position](const QString &markup) {
    if (!QStringView{text}.mid(position).startsWith(markup))
    {
      return false;
    }

    finder.setPosition(position + markup.size());
    return finder.isAtBoundary();
  };

  while (position < text.size())
  {
    finder.setPosition(position);
    int next = finder.toNextBoundary();
    if (next == -1)
    {
      next = text.size();
    }

    const QString grapheme = text.mid(position, next - position);

    int color = stack.isEmpty() ? _defaultColor : stack.last();
    if (escaping)
    {
      escaping = false;
    }
    else if (grapheme == QStringLiteral("\\"))
    {
      escaping = true;
    }
    else
    {
      int match = -1;
      bool matchEnd = false;
      int matchLength = 0;
      for (int i = 0; i < _markup.size(); ++i)
      {
        const theory::ChatMarkup &markup = _markup.at(i);
        if (markup.symbolStart.isEmpty())
        {
          continue;
        }

        const bool toggle = markup.symbolEnd.isEmpty() || markup.symbolEnd == markup.symbolStart;
        const bool open = !stack.isEmpty() && stack.last() == i;
        if (markupAt(markup.symbolStart) && markup.symbolStart.size() > matchLength)
        {
          match = i;
          matchEnd = false;
          matchLength = markup.symbolStart.size();
        }

        if (!toggle && open && markupAt(markup.symbolEnd) && markup.symbolEnd.size() > matchLength)
        {
          match = i;
          matchEnd = true;
          matchLength = markup.symbolEnd.size();
        }
      }

      if (match != -1)
      {
        const theory::ChatMarkup &markup = _markup.at(match);
        const bool toggle = markup.symbolEnd.isEmpty() || markup.symbolEnd == markup.symbolStart;
        const bool open = !stack.isEmpty() && stack.last() == match;
        if (matchEnd || (toggle && open && _defaultColor != match))
        {
          stack.removeLast();
        }
        else
        {
          stack.append(match);
        }

        color = match;
        next = position + matchLength;
      }
    }

    setFormat(position, next - position, _markup.at(color).color);
    position = next;
  }
}
