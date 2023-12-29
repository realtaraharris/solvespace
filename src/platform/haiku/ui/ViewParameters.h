#ifndef VIEWPARAMETERS_H
#define VIEWPARAMETERS_H

#include <StringView.h>
#include <Window.h>

class ViewParameters : public BWindow {
  public:
    ViewParameters(void);
    void UpdateViewParameters();
    void MessageReceived(BMessage *msg);

  private:
    BStringView *overallScaleFactor;
    BStringView *origin;
    BStringView *projection;
    BStringView *perspectiveFactor;
    BStringView *explodeDistance;
};

#endif // VIEWPARAMETERS_H
