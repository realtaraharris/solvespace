/*
 * Copyright 2023, Tara Harris <3769985+realtaraharris@users.noreply.github.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "agg_rendering_buffer.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_conv_transform.h"
#include "agg_conv_curve.h"
#include "agg_bspline.h"
#include "agg_ellipse.h"
#include "agg_gsv_text.h"
#include "agg_scanline_p.h"
#include "agg_renderer_scanline.h"
#include "agg_path_storage.h"
#include "agg_pixfmt_rgba.h"
#include "agg_conv_dash.h"
#include "agg_trans_viewport.h"

#include "solvespace.h"

#include <iostream>

namespace SolveSpace {
	typedef agg::pixfmt_bgra32 pixel_format_type;
	typedef agg::renderer_base<pixel_format_type> renderer_base;

	AggRenderer::AggRenderer() {}

	void AggRenderer::Clear() {
        SurfaceRenderer::Clear();

        pixel_format_type pixf(buffer);
	    renderer_base rb(pixf);
	    rb.clear(agg::rgba(0.0, 0.0, 0.0));
	}

	void AggRenderer::GetIdent(const char **vendor, const char **renderer, const char **version) {
	    *vendor = "AGG";
	    *renderer = "AGG";
	    *version = "2.4";
	}

	void AggRenderer::FlushFrame() {
	    CullOccludedStrokes();
	    OutputInPaintOrder();
	}

	std::shared_ptr<Pixmap> AggRenderer::ReadFrame() {
	    ssassert(false, "not implemented: AggRenderer::ReadFrame");
	}

	void AggRenderer::OutputStart() {
		cameraMatrix = agg::trans_affine_translation(camera.width / 2.0, camera.height / 2.0);

	    pixel_format_type pixf(buffer);
	    renderer_base rb(pixf);

		pf.reset();
	}

	void AggRenderer::OutputEnd() {
//		ssassert(false, "not implemented: AggRenderer::OutputEnd");
	}

	void AggRenderer::SelectStroke(hStroke hcs) {
//		ssassert(false, "not implemented: AggRenderer::SelectStroke");
	}

	void AggRenderer::MoveTo(Vector p) {
		ssassert(false, "not implemented: AggRenderer::MoveTo");
	}

	void AggRenderer::FinishPath() {}

	void AggRenderer::OutputBezier(const SBezier &b, hStroke hcs) {
	    Stroke *stroke = strokes.FindById(hcs);
		RgbaColor color = stroke->color;

		agg::rasterizer_scanline_aa<> pf;
		agg::scanline_p8 sl;

		typedef agg::pixfmt_bgra32 pixel_format_type;
		typedef agg::renderer_base<pixel_format_type> renderer_base;

		typedef agg::renderer_scanline_aa_solid<renderer_base> renderer_scanline;
		typedef agg::rasterizer_scanline_aa<> rasterizer_scanline;

		pixel_format_type pixf(buffer);
		renderer_base rb(pixf);

		double strokeWidth = stroke->WidthPx(camera);

	    Vector c, n = Vector::From(0, 0, 1);
	    double r;

		if (b.deg == 1) {
		    std::vector<double> dashes = StipplePatternDashes(stroke->stipplePattern);

			if (dashes.size() == 0) { // no dashes
				agg::path_storage ps;
				agg::conv_stroke<agg::path_storage> stroke(ps);
				ps.move_to(b.ctrl[0].x, b.ctrl[0].y);
				ps.line_to(b.ctrl[1].x, b.ctrl[1].y);
				stroke.width(strokeWidth);

		        agg::conv_transform<agg::conv_stroke<agg::path_storage>> camtrans(stroke, cameraMatrix);

				pf.add_path(camtrans);
			} else {
				agg::path_storage path;
				typedef agg::conv_dash<agg::path_storage> dash_t;

				dash_t dash(path);

				for (int i = 0, n = dashes.size(); i < n - 1; i += 2) {
					double on = dashes[i];
					double off = dashes[(i + 1) % n];
					dash.add_dash(on * 8, off * 8);
			    }

				path.move_to(b.ctrl[0].x, b.ctrl[0].y);
				path.line_to(b.ctrl[1].x, b.ctrl[1].y);
				agg::conv_stroke<dash_t> stroke(dash);

		        agg::conv_transform<agg::conv_stroke<dash_t>> camtrans(stroke, cameraMatrix);

				pf.add_path(camtrans);
			}

			agg::render_scanlines_aa_solid(pf, sl, rb, agg::rgba8(color.red, color.green, color.blue, color.alpha));
		} else if(b.IsCircle(n, &c, &r)) {
			ssassert(false, "not implemented: AggRenderer::OutputBezier b.IsCircle() == true");
		} else if (b.deg == 3 && !b.IsRational()) {
			ssassert(false, "wtf, this code never runs in test!");

			// TODO: are these defaults good?
			double m_angle_tolerance = 15; // 0 - 90
			double m_approximation_scale = 1.0; // 0.1 - 5
			double m_cusp_limit = 45; // 0 - 90
			double m_width = 4.0; // -50 - 100

			int m_curve_type = 1; // 0 is incremental, 1 is subdivision
			int m_case_type = 0; // cases 0 - 8
			int m_inner_join = 0; // "Inner Bevel", "Inner Miter", "Inner Jag", "Inner Round"
			int m_line_join = 0; // "Miter Join", "Miter Revert", "Round Join", "Bevel Join", "Miter Round"
			int m_line_cap = 2; // "Butt Cap", "Square Cap", "Round Cap"

			bool m_show_points = true;
			bool m_show_outline = false;

		    agg::ellipse e1;

		    renderer_scanline ren(rb);

		    rasterizer_scanline ras;

		    agg::path_storage path;

		    double x, y;
		    path.remove_all();
		    agg::curve3 curve;

		    curve.approximation_method(agg::curve_approximation_method_e(m_curve_type));
		    curve.approximation_scale(m_approximation_scale);
		    curve.angle_tolerance(agg::deg2rad(m_angle_tolerance));
		    curve.cusp_limit(agg::deg2rad(m_cusp_limit));

		    curve.approximation_scale(m_approximation_scale);
		    curve.angle_tolerance(agg::deg2rad(m_angle_tolerance));
		    curve.cusp_limit(agg::deg2rad(m_cusp_limit));

		    curve.init(b.ctrl[1].x, b.ctrl[1].y,
		               b.ctrl[2].x, b.ctrl[2].y,
		               b.ctrl[3].x, b.ctrl[3].y);

		    path.concat_path(curve);

		    agg::conv_stroke<agg::path_storage> stroke(path);
		    stroke.width(m_width);
		    stroke.line_join(agg::line_join_e(m_line_join));
		    stroke.line_cap(agg::line_cap_e(m_line_cap));
		    stroke.inner_join(agg::inner_join_e(m_inner_join));
		    stroke.inner_miter_limit(1.01);

			agg::conv_transform<agg::conv_stroke<agg::path_storage>> camtrans(stroke, cameraMatrix);
		    ras.add_path(camtrans);
		    ren.color(agg::rgba(0, 0.5, 0, 0.5)); // TODO: is this the right color?
		    agg::render_scanlines(ras, sl, ren);
		} else {
			ssassert(false, "not implemented: AggRenderer::OutputBezier OutputBezierAsNonrationalCubic");
		}
	}

	void AggRenderer::OutputTriangle(const STriangle &tr) {
		agg::rasterizer_scanline_aa<> ras;
		agg::scanline_p8 sl;
	    typedef agg::pixfmt_bgra32 pixel_format_type;
	    typedef agg::renderer_base<pixel_format_type> renderer_base;

	    pixel_format_type pixf(buffer);
	    renderer_base rb(pixf);

		RgbaColor color = tr.meta.color;

		agg::path_storage ps;
		ps.move_to(tr.a.x, tr.a.y);
		ps.line_to(tr.b.x, tr.b.y);
		ps.line_to(tr.c.x, tr.c.y);

		agg::conv_transform<agg::path_storage> camtrans(ps, cameraMatrix);

		ras.add_path(camtrans);
		agg::rgba tcolor = agg::rgba(color.redF(), color.greenF(), color.blueF(), color.alphaF());

		agg::conv_stroke<agg::path_storage> stroke(ps);
		agg::conv_transform<agg::conv_stroke<agg::path_storage>> camtransstroke(stroke, cameraMatrix);
		ras.add_path(camtransstroke);
		agg::render_scanlines_aa_solid(ras, sl, rb, tcolor);
	}

	void AggPixmapRenderer::Init(bool standalone) {
	    Clear();

		pixmap = std::make_shared<Pixmap>();
		pixmap->format = Pixmap::Format::BGRA;
		pixmap->width  = (size_t)camera.width;
		pixmap->height = (size_t)camera.height;
		pixmap->stride = 32 * camera.width; // TODO: remove hardcoded value
		pixmap->data   = std::vector<uint8_t>(pixmap->stride * pixmap->height);

		if (standalone) {
			buffer.attach(pixmap->data.data(), pixmap->width, pixmap->height, pixmap->stride);
		}
	}

	void AggPixmapRenderer::Clear() {
	    AggRenderer::Clear();
	}

	std::shared_ptr<Pixmap> AggPixmapRenderer::ReadFrame() {
		std::shared_ptr<Pixmap> result = pixmap->Copy();
		result->ConvertTo(Pixmap::Format::RGBA);
		return result;
	}
}
