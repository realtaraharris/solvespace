/*
 * Copyright 2024, My Name <my@email.address>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include <functional>
#include <iostream>

void LineToolButtonUnclickedEventHook() {}
void RectToolButtonUnclickedEventHook() {}
void CircleToolButtonUnclickedEventHook() {}
void ArcToolButtonUnclickedEventHook() {}
void TangentArcToolButtonUnclickedEventHook() {}
void CubicSplineToolButtonUnclickedEventHook() {}
void DatumPointToolButtonUnclickedEventHook() {}
void ConstructionToolButtonUnclickedEventHook() {}
void SplitCurvesToolButtonUnclickedEventHook() {}
void TextToolButtonUnclickedEventHook() {}

void ErrorDialog(std::string message, std::string description,
  std::function<void()> onDismiss = std::function<void()>()
) {}
void WarningDialog(std::string message, std::string description,
  std::function<void()> onDismiss = std::function<void()>()
) {}