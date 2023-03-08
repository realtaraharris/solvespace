// Copyright 2021 pulkomandy <pulkomandy@kitt>, MIT

#include "HVIFUtil.h"

#include "Bitmap.h"

BBitmap *LoadIconFromResource(const char* name, float iconSize) {
	BResources resources;
	resources.SetToImage(name);

	size_t size = 0;
	const uint8* buffer = (const uint8*)resources.LoadResource('VICN', name, &size);

	if (buffer == NULL) {
		return NULL;
	}

	BBitmap* result = new BBitmap(BRect(0, 0, iconSize, iconSize), B_RGBA32);
	status_t status = BIconUtils::GetVectorIcon(buffer, size, result);
	if (status != B_OK) {
			delete result;
			return NULL;
	}
	return result;
}
