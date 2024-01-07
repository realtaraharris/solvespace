#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <Window.h>

class AppToolbar : public BWindow {
  public:
    AppToolbar(BRect rect, BSize toolbarIconSize);
    void MessageReceived(BMessage *msg);

  private:
};

#endif
