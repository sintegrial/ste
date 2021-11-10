#ifndef EDITSTYLE_H
#define EDITSTYLE_H

#include <QColor>

struct EditStyle
{
public:
  EditStyle();

  QColor BackgroundColor, TextColor;
  QColor CaretLineBackgroundColor, CaretLineForegroundColor;
  QColor SelectionBackgroundColor, SelectionForegroundColor;

  QColor MatchedBraceBackgroundColor, MatchedBraceForegroundColor;
  QColor UnmatchedBraceBackgroundColor, UnmatchedBraceForegroundColor;

  QColor MarginsBackgroundColor, MarginsForegroundColor;
  QColor FoldMarginForegroundColor, FoldMarginBackgroundColor;
  QColor FoldMarkersForegroundColor, FoldMarkersBackgroundColor;

  QColor IndentationGuidesBackgroundColor, IndentationGuidesForegroundColor;

  QColor EdgeColor;

  QColor SearchIndicatorColor, HighlightIndicator1Color, HighlightIndicator2Color;
};

#endif // EDITSTYLE_H
