#include "editstyle.h"

EditStyle::EditStyle()
{
  BackgroundColor = Qt::white;
  TextColor = Qt::black;

  CaretLineBackgroundColor = 0xd0e9e9;
  CaretLineForegroundColor = TextColor;

  SelectionBackgroundColor = 0xe0e0ff;
  SelectionForegroundColor = Qt::transparent;

  MatchedBraceBackgroundColor = Qt::green;
  MatchedBraceForegroundColor = Qt::black;
  UnmatchedBraceBackgroundColor = Qt::yellow;
  UnmatchedBraceForegroundColor = Qt::red;

  MarginsForegroundColor = Qt::darkGray;
  MarginsBackgroundColor = 0xf0f0f0;

  FoldMarginForegroundColor = 0xf9f9f9;
  FoldMarginBackgroundColor = 0xf9f9f9;

  FoldMarkersForegroundColor = Qt::black;
  FoldMarkersBackgroundColor = Qt::white;

  IndentationGuidesForegroundColor = 0xcccccc;
  IndentationGuidesBackgroundColor = BackgroundColor;

  EdgeColor = 0x999999;

  SearchIndicatorColor = 0x6666ff;
  HighlightIndicator1Color = 0x33ff33;
  HighlightIndicator2Color = 0xffff33;
}
