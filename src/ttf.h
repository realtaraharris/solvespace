//-----------------------------------------------------------------------------
// routines to read a TrueType font as vector outlines, and generate them
// as entities, since they're always representable as either lines or
// quadratic Bezier curves
//
// copyright 2016 whitequark, Peter Barfuss.
//-----------------------------------------------------------------------------
#ifndef TTF_H
#define TTF_H

class TtfFont {
public:
    Platform::Path  fontFile; // or resource path/name as res://<path>
    std::string     name;
    FT_FaceRec_    *fontFace;
    double          capHeight;

    void SetResourceID(const std::string &resource);
    bool IsResource() const;

    std::string FontFileBaseName() const;
    bool LoadFromFile(FT_LibraryRec_ *fontLibrary, bool keepOpen = false);
    bool LoadFromResource(FT_LibraryRec_ *fontLibrary, bool keepOpen = false);

    void PlotString(const std::string &str,
                    SBezierList *sbl, Vector origin, Vector u, Vector v);
    double AspectRatio(const std::string &str);

    bool ExtractTTFData(bool keepOpen);
};

class TtfFontList {
public:
    FT_LibraryRec_ *fontLibrary;
    bool            loaded;
    List<TtfFont>   l;

    TtfFontList();
    ~TtfFontList();

    void LoadAll();
    TtfFont *LoadFont(const std::string &font);

    void PlotString(const std::string &font, const std::string &str,
                    SBezierList *sbl, Vector origin, Vector u, Vector v);
    double AspectRatio(const std::string &font, const std::string &str);
};

#endif // TTF_H