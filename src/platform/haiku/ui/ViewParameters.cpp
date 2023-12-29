#include "ViewParameters.h"
#include "App.h"

#include <LayoutBuilder.h>

#include "solvespace.h"

ViewParameters::ViewParameters(void)
    : BWindow(BRect(BPoint(730, 300), BSize(300, 400)), "View Parameters",
              B_FLOATING_WINDOW_LOOK, B_FLOATING_APP_WINDOW_FEEL,
              B_NOT_ZOOMABLE | B_ASYNCHRONOUS_CONTROLS, B_CURRENT_WORKSPACE) {
    overallScaleFactor = new BStringView(NULL, "");
    origin = new BStringView(NULL, "");
    projection = new BStringView(NULL, "");
    perspectiveFactor = new BStringView(NULL, "");
    explodeDistance = new BStringView(NULL, "");

    BLayoutBuilder::Group<>(this, B_VERTICAL, 0.0f)
        .Add(overallScaleFactor)
        .Add(BSpaceLayoutItem::CreateVerticalStrut(10.0f))
        .Add(new BStringView(NULL, "Origin (maps to center of screen):"))
        .Add(origin)
        .Add(BSpaceLayoutItem::CreateVerticalStrut(10.0f))
        .Add(new BStringView(NULL, "Projection onto screen:"))
        .Add(projection)
        .Add(BSpaceLayoutItem::CreateVerticalStrut(10.0f))
        .Add(new BStringView(NULL, "Perspective factor (0 for parallel):"))
        .Add(perspectiveFactor)

        // TODO: add light direction

        .Add(BSpaceLayoutItem::CreateVerticalStrut(10.0f))
        .Add(new BStringView(NULL, "Explode distance:"))
        .Add(explodeDistance)
        .Add(BSpaceLayoutItem::CreateGlue())
        .SetInsets(10.0f)
        .End();
}

void ViewParameters::UpdateViewParameters() {
    overallScaleFactor->SetText(std::format("Overall scale factor: {} px/{}",
                                            SS.GW.scale * SS.MmPerUnit(),
                                            SS.UnitName())
                                    .c_str());
    origin->SetText(std::format("({}, {}, {})", SS.MmToString(SS.GW.offset.x),
                                SS.MmToString(SS.GW.offset.y),
                                SS.MmToString(SS.GW.offset.z))
                        .c_str());
    Vector n = (SS.GW.projRight).Cross(SS.GW.projUp);
    projection->SetText(std::format("({}, {}, {})\n({}, {}, {})\n({}, {}, {})",
                                    CO(SS.GW.projRight), CO(SS.GW.projUp),
                                    CO(n))
                            .c_str());
    perspectiveFactor->SetText(
        std::format("{}", SS.cameraTangent * 1000).c_str());
    explodeDistance->SetText(
        std::format("{} {}", SS.MmToString(SS.explodeDistance), SS.UnitName())
            .c_str());
}

void ViewParameters::MessageReceived(BMessage *msg) {
    switch (msg->what) {
    case UPDATE_VIEW_PARAMETERS: {
        UpdateViewParameters();
        break;
    }
    }
}
