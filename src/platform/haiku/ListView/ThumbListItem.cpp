// this entire file is copied from WonderBrush

#include <View.h>
#include <Region.h>
#include "ThumbListItem.h"

#include "solvespace.h"

ThumbListItem::ThumbListItem(BBitmap *thumb, const char *text, int32 w, int level, bool expanded)
    : BListItem(level, expanded) {
  width = w;
  f_thumb = 0;
  BFont font(be_plain_font);
  font_height FontAttributes;
  be_plain_font->GetHeight(&FontAttributes);
  float FontHeight = ceil(FontAttributes.ascent) + ceil(FontAttributes.descent);
  float Width = font.StringWidth(text);

  if (thumb != NULL) {
    Width = thumb->Bounds().Width() + 8;
    BRect rect = thumb->Bounds();
    icon = new BBitmap(rect, B_RGBA32);
    if ((thumb->BytesPerRow() == icon->BytesPerRow()) &&
        (thumb->BitsLength() == icon->BitsLength())) {
      memcpy(icon->Bits(), thumb->Bits(), thumb->BitsLength());
    } else {
      delete icon;
    }
  } else {
    icon = NULL;
  }

  label.SetTo(text);
  SetWidth(Width);
  SetHeight(FontHeight + 2);
}

ThumbListItem::~ThumbListItem() {
  if (icon != NULL) {
    delete icon;
    icon = NULL;
  }
}

void ThumbListItem::ChangeIcon(ThumbListItem *item, BBitmap *micon) {
  BFont font(be_plain_font);
  font_height FontAttributes;
  be_plain_font->GetHeight(&FontAttributes);
  float FontHeight = ceil(FontAttributes.ascent) + ceil(FontAttributes.descent);
  float Width = font.StringWidth(label.String());

  if (item->icon != NULL) {
    delete item->icon;
    icon = NULL;
  }

  if (micon != NULL) {
    FontHeight += micon->Bounds().bottom + 8;
    Width = micon->Bounds().Width() + 8;

    BRect rect = micon->Bounds();
    item->icon = new BBitmap(rect, B_RGBA32);
    if ((micon->BytesPerRow() == item->icon->BytesPerRow()) &&
        (micon->BitsLength() == item->icon->BitsLength())) {
      memcpy(item->icon->Bits(), micon->Bits(), micon->BitsLength());
    } else {
      delete icon;
    }
  }
}

void ThumbListItem::DrawItem(BView *view, BRect frame, bool complete) {
  if (IsSelected()) {
    view->SetHighUIColor(B_LIST_SELECTED_BACKGROUND_COLOR);
  } else {
    view->SetHighUIColor(B_LIST_BACKGROUND_COLOR);
  }
  view->FillRect(frame);

  font_height fh;
  view->GetFontHeight(&fh);
  BString truncatedString(label);
  view->TruncateString(&truncatedString, B_TRUNCATE_MIDDLE, frame.Width() - TEXT_OFFSET - 4.0);

  float height = frame.Height();
  float textHeight = fh.ascent + fh.descent;
  BPoint textPoint;
  textPoint.x = frame.left + TEXT_OFFSET;
  textPoint.y = frame.top + ceilf(height / 2.0 - textHeight / 2.0 + fh.ascent);
  view->SetHighUIColor(B_PANEL_TEXT_COLOR);
  view->DrawString(truncatedString.String(), textPoint);

  view->SetDrawingMode(B_OP_ALPHA);
  view->DrawBitmap(icon, BPoint(frame.left, frame.top));
}

void ThumbListItem::Update(BView *view, const BFont *font) {}

const char *ThumbListItem::GetLabel(void) {
  return label.String();
}

void ThumbListItem::SetThumb(int t) {
  f_thumb = t;
}

int ThumbListItem::isThumbed(void) {
  return f_thumb;
}

BListItem *ThumbListItem::Clone(void) {
  return new ThumbListItem(icon, label.String(), width, 0, false);
}
