// this entire file is copied from WonderBrush

#pragma once

#include <String.h>
#include <Bitmap.h> 
#include <Rect.h>
#include <Font.h>
#include <ListView.h>
#include <ListItem.h>

enum {
    FLAGS_NONE = 0x00,
    FLAGS_TINTED_LINE = 0x01,
    FLAGS_FOCUSED = 0x02,
};

#define TEXT_OFFSET 25.0

class ThumbListItem : public BListItem {
public:
    ThumbListItem(BBitmap *icon, const char *text, int32 width, int level, bool expand);
    virtual ~ThumbListItem();
    virtual void DrawItem(BView *owner, BRect frame, bool complete = false);
    void ChangeIcon(ThumbListItem* item, BBitmap *micon);
    virtual void Update(BView *view, const BFont *font);
    const char *GetLabel(void);
    void SetThumb(int);
    int isThumbed(void);
    BListItem* Clone(void);
private:
    BRect bounds;
    BBitmap *icon;
    BString label;
    BString data;
    int32 width;
    int f_thumb;
};