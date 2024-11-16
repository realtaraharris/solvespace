//-----------------------------------------------------------------------------
// Routines for PDF output, some extra complexity because we have to generate
// a correct xref table.
//-----------------------------------------------------------------------------

#include "solvespace.h"
#include "filewriter/pdffilewriter.h"
#include "stipplepattern.h"

void PdfFileWriter::StartFile() {
    if((ptMax.x - ptMin.x) > 200*25.4 ||
       (ptMax.y - ptMin.y) > 200*25.4)
    {
        Message(_("PDF page size exceeds 200 by 200 inches; many viewers may "
                  "reject this file."));
    }

    fprintf(f,
"%%PDF-1.1\r\n"
"%%%c%c%c%c\r\n",
        0xe2, 0xe3, 0xcf, 0xd3);

    xref[1] = (uint32_t)ftell(f);
    fprintf(f,
"1 0 obj\r\n"
"  << /Type /Catalog\r\n"
"     /Outlines 2 0 R\r\n"
"     /Pages 3 0 R\r\n"
"  >>\r\n"
"endobj\r\n");

    xref[2] = (uint32_t)ftell(f);
    fprintf(f,
"2 0 obj\r\n"
"  << /Type /Outlines\r\n"
"     /Count 0\r\n"
"  >>\r\n"
"endobj\r\n");

    xref[3] = (uint32_t)ftell(f);
    fprintf(f,
"3 0 obj\r\n"
"  << /Type /Pages\r\n"
"     /Kids [4 0 R]\r\n"
"     /Count 1\r\n"
"  >>\r\n"
"endobj\r\n");

    xref[4] = (uint32_t)ftell(f);
    fprintf(f,
"4 0 obj\r\n"
"  << /Type /Page\r\n"
"     /Parent 3 0 R\r\n"
"     /MediaBox [0 0 %.3f %.3f]\r\n"
"     /Contents 5 0 R\r\n"
"     /Resources << /ProcSet 7 0 R\r\n"
"                   /Font << /F1 8 0 R >>\r\n"
"                >>\r\n"
"  >>\r\n"
"endobj\r\n",
            MmToPts(ptMax.x - ptMin.x),
            MmToPts(ptMax.y - ptMin.y));

    xref[5] = (uint32_t)ftell(f);
    fprintf(f,
"5 0 obj\r\n"
"  << /Length 6 0 R >>\r\n"
"stream\r\n");
    bodyStart = (uint32_t)ftell(f);
}

void PdfFileWriter::FinishAndCloseFile() {
    uint32_t bodyEnd = (uint32_t)ftell(f);

    fprintf(f,
"endstream\r\n"
"endobj\r\n");

    xref[6] = (uint32_t)ftell(f);
    fprintf(f,
"6 0 obj\r\n"
"  %d\r\n"
"endobj\r\n",
        bodyEnd - bodyStart);

    xref[7] = (uint32_t)ftell(f);
    fprintf(f,
"7 0 obj\r\n"
"  [/PDF /Text]\r\n"
"endobj\r\n");

    xref[8] = (uint32_t)ftell(f);
    fprintf(f,
"8 0 obj\r\n"
"  << /Type /Font\r\n"
"     /Subtype /Type1\r\n"
"     /Name /F1\r\n"
"     /BaseFont /Helvetica\r\n"
"     /Encoding /WinAnsiEncoding\r\n"
"  >>\r\n"
"endobj\r\n");

    xref[9] = (uint32_t)ftell(f);
    fprintf(f,
"9 0 obj\r\n"
"  << /Creator (SolveSpace)\r\n"
"  >>\r\n");

    uint32_t xrefStart = (uint32_t)ftell(f);
    fprintf(f,
"xref\r\n"
"0 10\r\n"
"0000000000 65535 f\r\n");

    int i;
    for(i = 1; i <= 9; i++) {
        fprintf(f, "%010d %05d n\r\n", xref[i], 0);
    }

    fprintf(f,
"\r\n"
"trailer\r\n"
"  << /Size 10\r\n"
"     /Root 1 0 R\r\n"
"     /Info 9 0 R\r\n"
"  >>\r\n"
"startxref\r\n"
"%d\r\n"
"%%%%EOF\r\n",
        xrefStart);

    fclose(f);

}

void PdfFileWriter::Background(RgbaColor color) {
    double width  = ptMax.x - ptMin.x;
    double height = ptMax.y - ptMin.y;
    double sw     = max(width, height) / 1000;

    fprintf(f,
"1 J 1 j\r\n"
"%.3f %.3f %.3f RG\r\n"
"%.3f %.3f %.3f rg\r\n"
"%.3f w\r\n"
"%.3f %.3f m\r\n"
"%.3f %.3f l\r\n"
"%.3f %.3f l\r\n"
"%.3f %.3f l\r\n"
"b\r\n",
            color.redF(), color.greenF(), color.blueF(),
            color.redF(), color.greenF(), color.blueF(),
            MmToPts(sw),
            MmToPts(0),     MmToPts(0),
            MmToPts(width), MmToPts(0),
            MmToPts(width), MmToPts(height),
            MmToPts(0),     MmToPts(height));
}

void PdfFileWriter::StartPath(RgbaColor strokeRgb, double lineWidth,
                              bool filled, RgbaColor fillRgb, hStyle hs)
{
    StipplePattern pattern = Style::PatternType(hs);
    double stippleScale = MmToPts(Style::StippleScaleMm(hs));

    fprintf(f, "1 J 1 j " // round endcaps and joins
               "%.3f w [%s] 0 d "
               "%.3f %.3f %.3f RG\r\n",
        MmToPts(lineWidth),
        MakeStipplePattern(pattern, stippleScale, ' ').c_str(),
        strokeRgb.redF(), strokeRgb.greenF(), strokeRgb.blueF());
    if(filled) {
        fprintf(f, "%.3f %.3f %.3f rg\r\n",
            fillRgb.redF(), fillRgb.greenF(), fillRgb.blueF());
    }

    prevPt = Vector::From(VERY_POSITIVE, VERY_POSITIVE, VERY_POSITIVE);
}
void PdfFileWriter::FinishPath(RgbaColor strokeRgb, double lineWidth,
                               bool filled, RgbaColor fillRgb, hStyle hs)
{
    if(filled) {
        fprintf(f, "b\r\n");
    } else {
        fprintf(f, "S\r\n");
    }
}

void PdfFileWriter::MaybeMoveTo(Vector st, Vector fi) {
    if(!prevPt.Equals(st)) {
        fprintf(f, "%.3f %.3f m\r\n",
            MmToPts(st.x - ptMin.x), MmToPts(st.y - ptMin.y));
    }
    prevPt = fi;
}

void PdfFileWriter::Triangle(STriangle *tr) {
    double sw = max(ptMax.x - ptMin.x, ptMax.y - ptMin.y) / 1000;

    fprintf(f,
"1 J 1 j\r\n"
"%.3f %.3f %.3f RG\r\n"
"%.3f %.3f %.3f rg\r\n"
"%.3f w\r\n"
"%.3f %.3f m\r\n"
"%.3f %.3f l\r\n"
"%.3f %.3f l\r\n"
"b\r\n",
            tr->meta.color.redF(), tr->meta.color.greenF(), tr->meta.color.blueF(),
            tr->meta.color.redF(), tr->meta.color.greenF(), tr->meta.color.blueF(),
            MmToPts(sw),
            MmToPts(tr->a.x - ptMin.x), MmToPts(tr->a.y - ptMin.y),
            MmToPts(tr->b.x - ptMin.x), MmToPts(tr->b.y - ptMin.y),
            MmToPts(tr->c.x - ptMin.x), MmToPts(tr->c.y - ptMin.y));
}

void PdfFileWriter::Bezier(SBezier *sb) {
    if(sb->deg == 1) {
        MaybeMoveTo(sb->ctrl[0], sb->ctrl[1]);
        fprintf(f,
"%.3f %.3f l\r\n",
            MmToPts(sb->ctrl[1].x - ptMin.x), MmToPts(sb->ctrl[1].y - ptMin.y));
    } else if(sb->deg == 3 && !sb->IsRational()) {
        MaybeMoveTo(sb->ctrl[0], sb->ctrl[3]);
        fprintf(f,
"%.3f %.3f %.3f %.3f %.3f %.3f c\r\n",
            MmToPts(sb->ctrl[1].x - ptMin.x), MmToPts(sb->ctrl[1].y - ptMin.y),
            MmToPts(sb->ctrl[2].x - ptMin.x), MmToPts(sb->ctrl[2].y - ptMin.y),
            MmToPts(sb->ctrl[3].x - ptMin.x), MmToPts(sb->ctrl[3].y - ptMin.y));
    } else {
        BezierAsNonrationalCubic(sb);
    }
}
