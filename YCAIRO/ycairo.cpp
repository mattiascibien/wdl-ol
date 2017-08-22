#include "ycairo.h"

// ycairo_background ------------------------------------------------------------------------------------------------------------------------------------
ycairo_background::ycairo_background(IPlugBase * pPlug, ycairo_base * ycairo_base, IGraphics * pGraphics, double red, double green, double blue)
	: IControl(pPlug, IRECT(0, 0, pGraphics->Width(), pGraphics->Height()))
{
	bg_red = red;
	bg_green = green;
	bg_blue = blue;

	ycairo = ycairo_base;
	mGraphics = pGraphics;
}

void ycairo_background::AfterGUIResize(double guiScaleRatio)
{
}

bool ycairo_background::Draw(IGraphics * pGraphics)
{
	//pGraphics->FillIRect(&IColor(255,255,255,255), &mDrawRECT);

	bg_surface = ycairo->get_surface();
	bg_cr = ycairo->get_cr();

	if (GetGUIResize())
		cairo_surface_set_device_scale(bg_surface, GetGUIResize()->GetGUIScaleRatio(), GetGUIResize()->GetGUIScaleRatio());

	cairo_reset_clip(bg_cr);
	cairo_new_path(bg_cr);

	//pGraphics->MarkAllIntersectingControlsDirty();

	//IRECT dirtyR;
	//if (pGraphics->IsDirty(&dirtyR))
	//{
	//	cairo_rectangle(bg_cr, dirtyR.L, dirtyR.T, dirtyR.W(), dirtyR.H());
	//}

	if (true)
	{
		cairo_rectangle(bg_cr, mNonScaledDrawRECT.L, mNonScaledDrawRECT.T, mNonScaledDrawRECT.W(), mNonScaledDrawRECT.H());
	}
	else
	{
		for (int i = 1; i < pGraphics->GetNControls(); i++)
		{
			if (pGraphics->GetControl(i)->IsDirty() && !pGraphics->GetControl(i)->IsHidden())
			{
				IRECT tmpRECT = *pGraphics->GetControl(i)->GetNonScaledDrawRECT();

				if (i == 1)
				{
					cairo_move_to(bg_cr, tmpRECT.L, tmpRECT.T);
					cairo_rel_line_to(bg_cr, tmpRECT.W(), 0);
					cairo_rel_line_to(bg_cr, 0, tmpRECT.H());
					cairo_rel_line_to(bg_cr, -tmpRECT.W(), 0);
				}
				else if (i > 1)
				{
					cairo_new_sub_path(bg_cr);
					cairo_line_to(bg_cr, tmpRECT.L, tmpRECT.T);
					cairo_rel_line_to(bg_cr, tmpRECT.W(), 0);
					cairo_rel_line_to(bg_cr, 0, tmpRECT.H());
					cairo_rel_line_to(bg_cr, -tmpRECT.W(), 0);
				}
			}
		}
	}

	cairo_set_source_rgb(bg_cr, bg_red, bg_green, bg_blue);
	cairo_fill(bg_cr);

	return true;
}
// ------------------------------------------------------------------------------------------------------------------------------------------------------


ycairo_base::ycairo_base(IPlugBase * pPlug)
{
	ycairo_iplug_base = pPlug;
}

ycairo_base::~ycairo_base()
{
	if (cr) cairo_destroy(cr);
	if (surface) cairo_surface_destroy(surface);

	// If global font was initialized, destroy font on exit
	if (global_font)
	{
		FT_Done_Face(ft_face);
		FT_Done_FreeType(ft_library);
	}

}

#ifdef _WIN32
void ycairo_base::set_HINSTANCE(HINSTANCE hinstance)
{
	hinstance_handle = hinstance;
}

HINSTANCE ycairo_base::get_HINSTANCE()
{
	return hinstance_handle;
}
#elif defined(__APPLE__)
void ycairo_base::set_BUNDLE_ID(const char* _bundleID)
{
	bundleID = _bundleID;
}
const char* ycairo_base::get_BUNDLE_ID()
{
	return bundleID;
}
#endif

void ycairo_base::create_global_font_from_path(const char * path)
{
	// If global font was initialized, destroy old font
	if (global_font)
	{
		FT_Done_Face(ft_face);
		FT_Done_FreeType(ft_library);
	}

	FT_Init_FreeType(&ft_library);
	FT_New_Face(ft_library, path, 0, &ft_face);


	global_font = true;
}

void ycairo_base::create_global_font_from_memory(int name, int type, const char * relative_path)
{
	// If global font was initialized, destroy old font
	if (global_font)
	{
		FT_Done_Face(ft_face);
		FT_Done_FreeType(ft_library);
	}

#ifdef _WIN32
	HRSRC rc = ::FindResource(hinstance_handle, MAKEINTRESOURCE(name), MAKEINTRESOURCE(type));
	HGLOBAL rcData = ::LoadResource(hinstance_handle, rc);
	int size = ::SizeofResource(hinstance_handle, rc);
	const FT_Byte* data = static_cast<const FT_Byte*>(::LockResource(rcData));

	FT_Init_FreeType(&ft_library);
	FT_New_Memory_Face(ft_library, data, size, 0, &ft_face);

#elif defined(__APPLE__)
    CFStringRef CFBundleID = __CFStringMakeConstantString(bundleID);
    CFBundleRef requestedBundle = CFBundleGetBundleWithIdentifier(CFBundleID);
	CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(requestedBundle);
	char path[PATH_MAX];
	CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)path, PATH_MAX);

	CFRelease(resourcesURL);

	chdir(path);

	std::string font_path = path;
	std::string font_name = relative_path;

	int lastindex = font_name.find_last_of("/");

	font_path.append(font_name.substr(lastindex, font_name.size() - lastindex));

	FT_Init_FreeType(&ft_library);
	FT_New_Face(ft_library, font_path.c_str(), 0, &ft_face);

#endif
	global_font = true;
}

void ycairo_base::bind_to_lice(IGraphics * pGraphics)
{
	base_width = pGraphics->Width();
	base_height = pGraphics->Height();

	int w = base_width;
	w = (w + 3)&~3; // Always keep backing store a multiple of 4px wide // Same as LICE System Bitmap

	surface = cairo_image_surface_create_for_data((unsigned char*)pGraphics->GetBits(),
		CAIRO_FORMAT_RGB24, base_width, base_height, cairo_format_stride_for_width(CAIRO_FORMAT_RGB24, w));

	cr = cairo_create(surface);

	cairo_set_source_rgb(cr, red, green, blue);
	cairo_rectangle(cr, 0, 0, base_width, base_height);
	cairo_fill(cr);
}

void ycairo_base::attach_background(IGraphics * pGraphics, IColor color)
{
	base_width = pGraphics->Width();
	base_height = pGraphics->Height();

	red = (double)color.R / 255.0;
	green = (double)color.G / 255.0;
	blue = (double)color.B / 255.0;

	IControl* pBG = new ycairo_background(ycairo_iplug_base, this, pGraphics, red, green, blue);
	pGraphics->AttachControl(pBG);
}

cairo_t * ycairo_base::get_cr()
{
	return cr;
}

cairo_surface_t * ycairo_base::get_surface()
{
	return surface;
}

int ycairo_base::get_width()
{
	return base_width;
}

int ycairo_base::get_height()
{
	return base_height;
}

FT_Library * ycairo_base::get_global_ft_lib()
{
	return &ft_library;
}

FT_Face * ycairo_base::get_global_ft_face()
{
	return &ft_face;
}

bool ycairo_base::global_font_initialized()
{
	return global_font;
}

IPlugBase * ycairo_base::GetIPlugBase()
{
	return ycairo_iplug_base;
}

ycairo_gui::ycairo_gui(ycairo_base * ycairo_base, IControl *pControl)
{
	if (ycairo_base->GetIPlugBase()->GetGUIResize())
	{
		non_scaled_draw_rect = pControl->GetNonScaledDrawRECT();
	}
	else
	{
		draw_rect = pControl->GetDrawRECT();
	}

	ycairo = ycairo_base;
}

void ycairo_gui::ycairo_reset_clip(cairo_t * cr)
{
	if (ycairo->GetIPlugBase()->GetGUIResize())
	{
		ycairo_reset_clip_to(cr, *non_scaled_draw_rect);
	}
	else
	{
		ycairo_reset_clip_to(cr, *draw_rect);
	}
}

void ycairo_helper::ycairo_reset_clip_to(cairo_t *cr, IRECT rect)
{
	cairo_new_path(cr);
	cairo_reset_clip(cr);
	cairo_rectangle(cr, rect.L, rect.T, rect.W(), rect.H());
	cairo_clip(cr);
}

void ycairo_helper::ycairo_rounded_rectangle(cairo_t * cr, double x, double y, double width, double height, double corner)
{
	cairo_new_sub_path(cr);
	cairo_arc(cr, x + width - corner, y + corner, corner, -1.5707963267948966192313216916398, 0);
	cairo_arc(cr, x + width - corner, y + height - corner, corner, 0, 1.5707963267948966192313216916398);
	cairo_arc(cr, x + corner, y + height - corner, corner, 1.5707963267948966192313216916398, 3.1415926535897932384626433832796);
	cairo_arc(cr, x + corner, y + corner, corner, 3.1415926535897932384626433832796, 4.7123889803846898576939650749193);
	cairo_close_path(cr);
}

void ycairo_helper::ycairo_circle(cairo_t * cr, double x, double y, double radius)
{
	cairo_arc(cr, x, y, radius, 0, 6.283185307179586476925286766559);
}

void ycairo_helper::ycairo_set_source_rgba(cairo_t * cr, IColor color)
{
	cairo_set_source_rgba(cr, color.R / 255.0, color.G / 255.0, color.B / 255.0, color.A / 255.0);
}

void ycairo_helper::ycairo_set_source_rgba(cairo_t * cr, IColor *color)
{
	cairo_set_source_rgba(cr, color->R / 255.0, color->G / 255.0, color->B / 255.0, color->A / 255.0);
}

void ycairo_helper::ycairo_set_source_rgba_fast(cairo_t * cr, IColor color)
{
	cairo_set_source_rgba(cr, color.R / 256.0, color.G / 256.0, color.B / 256.0, color.A / 256.0);
}

void ycairo_helper::ycairo_set_source_rgba_fast(cairo_t * cr, IColor *color)
{
	cairo_set_source_rgba(cr, color->R / 256.0, color->G / 256.0, color->B / 256.0, color->A / 256.0);
}


void ycairo_drop_shadow::_ycairo_draw_drop_shadow_fast(cairo_t * cr, bool stroke)
{
	int props_index = _get_props_index(cr);

	cairo_path_t *path = cairo_copy_path(cr);
	cairo_pattern_t *source_pattern = cairo_pattern_reference(cairo_get_source(cr));
	
	double cx1, cy1, cx2, cy2;
	cairo_clip_extents(cr, &cx1, &cy1, &cx2, &cy2);

	double x1, y1, x2, y2;
	if (stroke)
	{
		cairo_set_line_width(cr, props[props_index].shadow_radius);
		cairo_stroke_extents(cr, &x1, &y1, &x2, &y2);
	}
	else cairo_path_extents(cr, &x1, &y1, &x2, &y2);

	x1 = IPMAX(x1, cx1);
	y1 = IPMAX(y1, cy1);
	x2 = IPMIN(x2, cx2);
	y2 = IPMIN(y2, cy2);

	double surface_width = IPMAX(x2 - x1, 0.0) / props[props_index].shadow_radius;
	double surface_height = IPMAX(y2 - y1, 0.0) / props[props_index].shadow_radius;

	if (surface_width - int(surface_width) > 0) surface_width++;
	if (surface_height - int(surface_height) > 0) surface_height++;

	if (surface_width < 1) surface_width++;
	if (surface_height < 1) surface_height++;

	cairo_surface_t *shadow_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, (int)surface_width, (int)surface_height);
	cairo_t *shadow_cr = cairo_create(shadow_surface);

	cairo_translate(shadow_cr, -x1 / props[props_index].shadow_radius, -y1 / props[props_index].shadow_radius);
	cairo_scale(shadow_cr, 1 / props[props_index].shadow_radius, 1 / props[props_index].shadow_radius);

	cairo_set_source_rgba(shadow_cr, 0, 0, 0, props[props_index].shadow_opacity);
	cairo_append_path(shadow_cr, path);

	if (stroke)
	{
		cairo_set_line_width(shadow_cr, props[props_index].shadow_radius);
		cairo_stroke(shadow_cr);
	}
	else cairo_fill(shadow_cr);

	cairo_scale(shadow_cr, props[props_index].shadow_radius, props[props_index].shadow_radius);
	cairo_translate(shadow_cr, x1 / props[props_index].shadow_radius, y1 / props[props_index].shadow_radius);

	// Destination surface
	cairo_scale(cr, props[props_index].shadow_radius, props[props_index].shadow_radius);

	cairo_set_source_surface(cr, shadow_surface, (x1 + props[props_index].shadow_offset_x) / props[props_index].shadow_radius, (y1 + props[props_index].shadow_offset_y) / props[props_index].shadow_radius);
	cairo_paint(cr);

	cairo_scale(cr, 1 / props[props_index].shadow_radius, 1 / props[props_index].shadow_radius);

	cairo_set_source(cr, source_pattern);

	if (!stroke)
	{
		cairo_new_path(cr);
		cairo_append_path(cr, path);
	}
		
	cairo_pattern_destroy(source_pattern);
	cairo_path_destroy(path);
	cairo_destroy(shadow_cr);
	cairo_surface_destroy(shadow_surface);
}

void ycairo_drop_shadow::_ycairo_draw_drop_shadow(cairo_t * cr, bool stroke, double downsample)
{
	int props_index = _get_props_index(cr);

	cairo_path_t *path = cairo_copy_path(cr);
	cairo_pattern_t *source_pattern = cairo_pattern_reference(cairo_get_source(cr));
	
	double cx1, cy1, cx2, cy2;
	cairo_clip_extents(cr, &cx1, &cy1, &cx2, &cy2);

	double x1, y1, x2, y2;
	if (stroke)
	{
		cairo_set_line_width(cr, props[props_index].shadow_radius);
		cairo_stroke_extents(cr, &x1, &y1, &x2, &y2);
	}
	else cairo_path_extents(cr, &x1, &y1, &x2, &y2);

	x1 = IPMAX(x1, cx1);
	y1 = IPMAX(y1, cy1);
	x2 = IPMIN(x2, cx2);
	y2 = IPMIN(y2, cy2);

	double surface_width = (IPMAX(x2 - x1, 0.0) + props[props_index].shadow_radius * 2) / downsample;
	double surface_height = (IPMAX(y2 - y1, 0.0) + props[props_index].shadow_radius * 2) / downsample;

	if (surface_width - int(surface_width) > 0) surface_width++;
	if (surface_height - int(surface_height) > 0) surface_height++;

	if (surface_width < 1) surface_width++;
	if (surface_height < 1) surface_height++;

	cairo_surface_t *shadow_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, (int)surface_width, (int)surface_height);
	cairo_t *shadow_cr = cairo_create(shadow_surface);

	cairo_translate(shadow_cr, (-x1 + props[props_index].shadow_radius) / downsample, (-y1 + props[props_index].shadow_radius) / downsample);
	cairo_scale(shadow_cr, 1.0 / downsample, 1.0 / downsample);

	cairo_set_source_rgb(shadow_cr, 0, 0, 0);
	cairo_append_path(shadow_cr, path);

	if (stroke)
	{
		cairo_set_line_width(shadow_cr, props[props_index].shadow_radius);
		cairo_stroke(shadow_cr);
	}
	else cairo_fill(shadow_cr);

	cairo_scale(shadow_cr, downsample, downsample);
	cairo_translate(shadow_cr, (x1 - props[props_index].shadow_radius) / downsample, (y1 - props[props_index].shadow_radius) / downsample);

	// Destination surface
	_ycairo_blur_surface_channel_offseted_minus_radius(shadow_surface, int(props[props_index].shadow_radius / downsample), 3);

	cairo_scale(cr, downsample, downsample);

	cairo_set_source_surface(cr, shadow_surface, ((x1 + props[props_index].shadow_offset_x) - props[props_index].shadow_radius / 2) / downsample, ((y1 + props[props_index].shadow_offset_y) - props[props_index].shadow_radius / 2) / downsample);
	cairo_paint_with_alpha(cr, props[props_index].shadow_opacity);

	cairo_scale(cr, 1.0 / downsample, 1.0 / downsample);

	cairo_new_path(cr);
	cairo_append_path(cr, path);
	cairo_set_source(cr, source_pattern);

	cairo_pattern_destroy(source_pattern);
	cairo_path_destroy(path);
	cairo_destroy(shadow_cr);

	cairo_surface_destroy(shadow_surface);
}


void ycairo_helper::ycairo_triangle(cairo_t * cr, double x0, double y0, double x1, double y1, double x2, double y2)
{
	cairo_new_sub_path(cr);
	cairo_move_to(cr, x0, y0);
	cairo_line_to(cr, x1, y1);
	cairo_line_to(cr, x2, y2);
	cairo_close_path(cr);
}

void ycairo_gui::ycairo_prepare_draw()
{
	//Getting surface
	surface = ycairo->get_surface();
	cr = ycairo->get_cr();
	
	//Adding new path and new clip region
	cairo_new_path(cr);

	if (ycairo->GetIPlugBase()->GetGUIResize())
	{
		cairo_rectangle(cr, non_scaled_draw_rect->L, non_scaled_draw_rect->T, non_scaled_draw_rect->W(), non_scaled_draw_rect->H());
	}
	else
	{
		cairo_rectangle(cr, draw_rect->L, draw_rect->T, draw_rect->W(), draw_rect->H());
	}
		
	cairo_clip(cr);
}

void ycairo_gui::ycairo_draw()
{
	//cairo_surface_flush(surface);
	cairo_reset_clip(cr);
}


ycairo_text::ycairo_text(ycairo_base * ycairo_base)
{
#ifdef _WIN32
	hinstance_handle = ycairo_base->get_HINSTANCE();
#elif defined(__APPLE__)
	bundleID = ycairo_base->get_BUNDLE_ID();
#endif
	
	ext_height = new cairo_text_extents_t;
	text_extents = new cairo_text_extents_t;
	font_extents = new cairo_font_extents_t;

	global_font_initialized = ycairo_base->global_font_initialized();

	if (global_font_initialized)
	{
		global_ft_library = ycairo_base->get_global_ft_lib();
		global_ft_face = ycairo_base->get_global_ft_face();
	}	
}

ycairo_text::~ycairo_text()
{
	delete ext_height;
	delete text_extents;
	delete font_extents;

	// If local font was initialized destroy font on exit
	if (local_font_initialized)
	{
		FT_Done_Face(local_ft_face);
		FT_Done_FreeType(local_ft_library);
	}
}

void ycairo_text::ycairo_create_font_from_path(const char * path)
{
	// If local font was initialized destroy old font
	if (local_font_initialized)
	{
		FT_Done_Face(local_ft_face);
		FT_Done_FreeType(local_ft_library);
	}

	FT_Init_FreeType(&local_ft_library);
	FT_New_Face(local_ft_library, path, 0, &local_ft_face);

	local_font_initialized = true;
}

void ycairo_text::ycairo_create_font_from_memory(int name, int type, const char * relative_path)
{
	// If local font was initialized destroy old font
	if (local_font_initialized)
	{
		FT_Done_Face(local_ft_face);
		FT_Done_FreeType(local_ft_library);
	}

#ifdef _WIN32
	HRSRC rc = ::FindResource(hinstance_handle, MAKEINTRESOURCE(name), MAKEINTRESOURCE(type));
	HGLOBAL rcData = ::LoadResource(hinstance_handle, rc);
	int size = ::SizeofResource(hinstance_handle, rc);
	const FT_Byte* data = static_cast<const FT_Byte*>(::LockResource(rcData));

	FT_Init_FreeType(&local_ft_library);
	FT_New_Memory_Face(local_ft_library, data, size, 0, &local_ft_face);

#elif defined(__APPLE__)
    CFStringRef CFBundleID = __CFStringMakeConstantString(bundleID);
    CFBundleRef requestedBundle = CFBundleGetBundleWithIdentifier(CFBundleID);
	CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(requestedBundle);
	char path[PATH_MAX];
	CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)path, PATH_MAX);

	CFRelease(resourcesURL);

	chdir(path);

	std::string font_path = path;
	std::string font_name = relative_path;

	int lastindex = font_name.find_last_of("/");

	font_path.append(font_name.substr(lastindex, font_name.size() - lastindex));

	FT_Init_FreeType(&local_ft_library);
	FT_New_Face(local_ft_library, font_path.c_str(), 0, &local_ft_face);

#endif
	local_font_initialized = true;
}

void ycairo_text::ycairo_initialize_font_face(cairo_t * cr)
{
	if (local_font_initialized)
	{
		current_font_face = cairo_ft_font_face_create_for_ft_face(local_ft_face, 0);
		cairo_set_font_face(cr, current_font_face);
	}
	else if (global_font_initialized)
	{
		current_font_face = cairo_ft_font_face_create_for_ft_face(*global_ft_face, 0);
		cairo_set_font_face(cr, current_font_face);
	}
	else
	{
		return;
	}
}

void ycairo_text::ycairo_destroy_font_face()
{
	cairo_font_face_destroy(current_font_face);
}

void ycairo_text::ycairo_set_text(cairo_t * cr, const char * text)
{
	draw_text = text;
}

void ycairo_text::ycairo_set_text_position(cairo_t * cr, DRECT rect, ycairo_text_w_aligement w_aligement, ycairo_text_h_aligement h_aligement)
{
	width_aligement = w_aligement;
	height_aligement = h_aligement;

	text_rect = rect;

	ycairo_calculate_extents(cr);

	double x, y;

	switch (width_aligement)
	{
	case YCAIRO_TEXT_W_ALIGN_LEFT:
		x = text_rect.L;
		break;

	case YCAIRO_TEXT_W_ALIGN_RIGHT:
		x = text_rect.R - text_extents->width - text_extents->x_bearing;
		break;

	case YCAIRO_TEXT_W_ALIGN_CENTER:
		x = text_rect.L + ((text_rect.W() - text_extents->width - text_extents->x_bearing) / 2.0);
		break;

	default:
		break;
	}

	switch (height_aligement)
	{
	case YCAIRO_TEXT_H_ALIGN_TOP:
		y = text_rect.T + font_extents->ascent;
		break;

	case YCAIRO_TEXT_H_ALIGN_BOTTOM:
		y = text_rect.B - font_extents->descent;
		break;

	case YCAIRO_TEXT_H_ALIGN_CENTER:
		y = text_rect.B - ((text_rect.H() - font_extents->height) / 2.0) - font_extents->descent;
		break;

	default:
		break;
	}

	cairo_move_to(cr, x, y);
}

void ycairo_text::ycairo_calculate_extents(cairo_t * cr)
{
	cairo_font_extents(cr, font_extents);
	cairo_text_extents(cr, draw_text, text_extents);
}

cairo_font_extents_t * ycairo_text::ycairo_get_font_extents(cairo_t * cr)
{
	return font_extents;
}

cairo_text_extents_t * ycairo_text::ycairo_get_text_extents(cairo_t * cr)
{
	return text_extents;
}

void ycairo_text::ycairo_show_text(cairo_t * cr, const char * text, double size, IColor color, IRECT rect, ycairo_text_w_aligement w_aligement, ycairo_text_h_aligement h_aligement)

{
	cairo_set_source_rgba(cr, color.R / 255.0, color.G / 255.0, color.B / 255.0, color.A / 255.0);

	ycairo_initialize_font_face(cr);
	cairo_set_font_size(cr, size);
	ycairo_set_text(cr, text);
	ycairo_set_text_position(cr, rect, w_aligement, h_aligement);

	//// Adding subpixel rendering improves rendering speed by 10% on my system
	//cairo_font_options_t *options;
	//cairo_get_font_options(cr, options);

	//cairo_font_options_set_antialias(options, CAIRO_ANTIALIAS_SUBPIXEL);
	//cairo_set_font_options(cr, options);
	//// -----------------------------------------------------------------------

	cairo_show_text(cr, text);

	ycairo_destroy_font_face();
}

void ycairo_text::ycairo_show_text(cairo_t * cr)
{
	cairo_show_text(cr, draw_text);
}


void ycairo_blur::_ycairo_blur_surface_offseted(cairo_surface_t * src_surface, unsigned int radius, unsigned int channels_num)
{
	unsigned char* src = cairo_image_surface_get_data(src_surface);
	unsigned int src_width = cairo_image_surface_get_width(src_surface);
	unsigned int src_stride = cairo_image_surface_get_stride(src_surface);
	unsigned int src_height = cairo_image_surface_get_height(src_surface);
	unsigned int src_excess_w_stride = src_stride - src_width * channels_num;

	cairo_surface_flush(src_surface);

	unsigned int w_radius_stride = radius * channels_num;
	unsigned int h_radius_stride = radius * src_stride;

	unsigned char *src_c = src;

	// Blur horizontaly
	for (unsigned int h = 0; h < src_height; h++)
	{
		_reset();

		for (unsigned int w = 0; w < src_width; w++)
		{
			_average_pixel(src_c, w, radius, w_radius_stride);
			src_c += channels_num;
		}
		src_c += src_excess_w_stride;
	}

	// Blur vertically
	for (unsigned int s = 0; s < src_stride - src_excess_w_stride; s += channels_num)
	{
		src_c = src + s;
		_reset();

		for (unsigned int h = 0; h < src_height; h++)
		{
			_average_pixel(src_c, h, radius, h_radius_stride);
			src_c += src_stride;
		}
	}
}

void ycairo_blur::_ycairo_blur_surface_channel_offseted(cairo_surface_t * src_surface, unsigned int radius, unsigned int channel, unsigned int channels_num)
{
	unsigned char* src = cairo_image_surface_get_data(src_surface);
	unsigned int src_width = cairo_image_surface_get_width(src_surface);
	unsigned int src_stride = cairo_image_surface_get_stride(src_surface);
	unsigned int src_height = cairo_image_surface_get_height(src_surface);
	unsigned int src_excess_w_stride = src_stride - src_width * channels_num;

	cairo_surface_flush(src_surface);

	unsigned int w_radius_stride = radius * channels_num;
	unsigned int h_radius_stride = radius * src_stride;

	unsigned char *src_c = src + channel;

	// Blur horizontaly
	for (unsigned int h = 0; h < src_height; h++)
	{
		_reset();

		for (unsigned int w = 0; w < src_width; w++)
		{
			_average_channel(src_c, w, radius, w_radius_stride);
			src_c += channels_num;
		}
		src_c += src_excess_w_stride;
	}

	// Blur vertically
	for (unsigned int s = 0; s < src_stride - src_excess_w_stride; s += channels_num)
	{
		src_c = src + s;
		_reset();

		for (unsigned int h = 0; h < src_height; h++)
		{
			_average_channel(src_c, h, radius, h_radius_stride);
			src_c += src_stride;
		}
	}
}


void ycairo_blur::_ycairo_blur_surface_offseted_minus_radius(cairo_surface_t * src_surface, unsigned int radius, unsigned int channels_num)
{
	unsigned int shift = _get_bit_shift(radius);

	unsigned char* src = cairo_image_surface_get_data(src_surface);
	unsigned int src_width = cairo_image_surface_get_width(src_surface);
	unsigned int src_stride = cairo_image_surface_get_stride(src_surface);
	unsigned int src_height = cairo_image_surface_get_height(src_surface);
	unsigned int src_excess_w_stride = src_stride - src_width * channels_num;

	cairo_surface_flush(src_surface);

	unsigned int w_radius_stride = radius * channels_num;
	unsigned int h_radius_stride = radius * src_stride;

	unsigned char *src_c = src + h_radius_stride + w_radius_stride;

	if (shift > 0)
	{
		// Blur horizontaly
		for (unsigned int h = radius; h < src_height; h++)
		{
			_reset();

			for (unsigned int w = radius; w < src_width; w++)
			{
				_average_pixel_bit_shift(src_c, shift, w_radius_stride);
				src_c += channels_num;
			}
			src_c += w_radius_stride;
			src_c += src_excess_w_stride;
		}

		// Blur vertically
		for (unsigned int s = 0; s < src_stride - src_excess_w_stride; s += channels_num)
		{
			src_c = src + s + h_radius_stride;
			_reset();

			for (unsigned int h = radius; h < src_height; h++)
			{
				_average_pixel_bit_shift(src_c, shift, h_radius_stride);
				src_c += src_stride;
			}
		}
	}
	else
	{
		// Blur horizontaly
		for (unsigned int h = radius; h < src_height; h++)
		{
			_reset();

			for (unsigned int w = radius; w < src_width; w++)
			{
				_average_pixel_minus_radius(src_c, radius, w_radius_stride);
				src_c += channels_num;
			}
			src_c += w_radius_stride;
			src_c += src_excess_w_stride;
		}

		// Blur vertically
		for (unsigned int s = 0; s < src_stride - src_excess_w_stride; s += channels_num)
		{
			src_c = src + s + h_radius_stride;
			_reset();

			for (unsigned int h = radius; h < src_height; h++)
			{
				_average_pixel_minus_radius(src_c, radius, h_radius_stride);
				src_c += src_stride;
			}
		}
	}
	cairo_surface_mark_dirty(src_surface);
}

void ycairo_blur::_ycairo_blur_surface_channel_offseted_minus_radius(cairo_surface_t * src_surface, unsigned int radius, unsigned int channel, int unsigned channels_num)
{
	unsigned int shift = _get_bit_shift(radius);

	unsigned char* src = cairo_image_surface_get_data(src_surface);
	unsigned int src_width = cairo_image_surface_get_width(src_surface);
	unsigned int src_stride = cairo_image_surface_get_stride(src_surface);
	unsigned int src_height = cairo_image_surface_get_height(src_surface);
	unsigned int src_excess_w_stride = src_stride - src_width * channels_num;

	cairo_surface_flush(src_surface);

	unsigned int w_radius_stride = radius * channels_num;
	unsigned int h_radius_stride = radius * src_stride;

	unsigned char *src_c = src + channel + h_radius_stride + w_radius_stride;

	unsigned int max_sum = radius * 255;

	if (shift > 0)
	{
		// Blur horizontaly
		for (unsigned int h = radius; h < src_height; h++)
		{
			_reset();

			for (int w = radius; w < src_width; w++)
			{
				_average_channel_bit_shift(src_c, shift, w_radius_stride, max_sum);
				src_c += channels_num;
			}
			src_c += w_radius_stride;
			src_c += src_excess_w_stride;
		}

		// Blur vertically
		for (unsigned int s = 0; s < src_stride - src_excess_w_stride; s += channels_num)
		{
			src_c = src + s + channel + h_radius_stride;
			_reset();

			for (unsigned int h = radius; h < src_height; h++)
			{
				_average_channel_bit_shift(src_c, shift, h_radius_stride, max_sum);
				src_c += src_stride;
			}
		}
	}
	else
	{
		// Blur horizontaly
		for (unsigned int h = radius; h < src_height; h++)
		{
			_reset();

			for (int w = radius; w < src_width; w++)
			{
				_average_channel_minus_radius(src_c, radius, w_radius_stride, max_sum);
				src_c += channels_num;
			}
			src_c += w_radius_stride;
			src_c += src_excess_w_stride;
		}

		// Blur vertically
		for (unsigned int s = 0; s < src_stride - src_excess_w_stride; s += channels_num)
		{
			src_c = src + s + channel + h_radius_stride;
			_reset();

			for (unsigned int h = radius; h < src_height; h++)
			{
				_average_channel_minus_radius(src_c, radius, h_radius_stride, max_sum);
				src_c += src_stride;
			}
		}
	}

	cairo_surface_mark_dirty(src_surface);
}


inline unsigned int ycairo_blur::_get_bit_shift(unsigned int radius)
{
	switch (radius)
	{
	case 2: return 1;
	case 4: return 2;
	case 8: return 3;
	case 16: return 4;
	case 32: return 5;
	case 64: return 6;
	case 128: return 7;
	case 256: return 8;
	case 512: return 9;
	case 1024: return 10;
	}

	return 0;
}


inline void _ycairo_pixel_average::_average_channel_minus_radius(unsigned char *in, unsigned int size, unsigned int delay_stride, unsigned int max_sum)
{
	unsigned char *delayed = in - delay_stride;
	out = out + *in - *delayed;

	//// Prevent drawing same over one another
	//if (out == max_sum && *delayed == 255) return; // Prevent overdrawing 255
	//if (out < size && *delayed == 0) return; // Prevent overdrawing 0

	*delayed = (unsigned char)(out / size);
}

inline void _ycairo_pixel_average::_average_pixel_minus_radius(unsigned char * in, unsigned int size, unsigned int delay_stride)
{
	unsigned char *delayed = in - delay_stride;

	out1 = (out1 + *(in + 0) - *(delayed + 0));
	out2 = (out2 + *(in + 1) - *(delayed + 1));
	out3 = (out3 + *(in + 2) - *(delayed + 2));
	out4 = (out4 + *(in + 3) - *(delayed + 3));
	
	*delayed = (unsigned char)(out1 / size);
	*(delayed + 1) = (unsigned char)(out2 / size);
	*(delayed + 2) = (unsigned char)(out3 / size);
	*(delayed + 3) = (unsigned char)(out4 / size);
}

inline void _ycairo_pixel_average::_average_pixel(unsigned char * in, unsigned int start_index, unsigned int size, unsigned int delay_stride)
{
	if (start_index < size)
	{
		out1 = (out1 + *(in + 0));
		out2 = (out2 + *(in + 1));
		out3 = (out3 + *(in + 2));
		out4 = (out4 + *(in + 3));
	}
	else
	{
		unsigned char *delayed = in - delay_stride;

		out1 = ((out1 + *(in + 0)) - *(delayed + 0));
		out2 = (out2 + *(in + 1) - *(delayed + 1));
		out3 = (out3 + *(in + 2) - *(delayed + 2));
		out4 = (out4 + *(in + 3) - *(delayed + 3));

		*delayed = (unsigned char)(out1 / size);
		*(delayed + 1) = (unsigned char)(out2 / size);
		*(delayed + 2) = (unsigned char)(out3 / size);
		*(delayed + 3) = (unsigned char)(out4 / size);
	}
}

inline void _ycairo_pixel_average::_average_channel(unsigned char * in, unsigned int start_index, unsigned int size, unsigned int delay_stride)
{
	if (start_index < size)
	{
		out1 = (out1 + *(in + 0));
	}
	else
	{
		unsigned char *delayed = in - delay_stride;
		out1 = ((out1 + *(in + 0)) - *(delayed + 0));
		*delayed = (unsigned char)(out1 / size);
	}
}

inline void _ycairo_pixel_average::_average_channel_bit_shift(unsigned char *in, unsigned int shift, unsigned int delay_stride, unsigned int max_sum)
{
	unsigned char *delayed = in - delay_stride;
	out = out - *delayed + *in;

	//// Prevent drawing same over one another
	//if (out == max_sum && *delayed == 255) return; // Prevent overdrawing 255
	//if (out < shift && *delayed == 0) return; // Prevent overdrawing 0

	*delayed = (unsigned char)(out >> shift);
}

inline void _ycairo_pixel_average::_average_pixel_bit_shift(unsigned char * in, unsigned int shift, unsigned int delay_stride)
{
	unsigned char *delayed = in - delay_stride;

	out1 = (out1 + *(in + 0) - *(delayed + 0));
	out2 = (out2 + *(in + 1) - *(delayed + 1));
	out3 = (out3 + *(in + 2) - *(delayed + 2));
	out4 = (out4 + *(in + 3) - *(delayed + 3));
	
	*delayed = (unsigned char)(out1 >> shift);
	*(delayed + 1) = (unsigned char)(out2 >> shift);
	*(delayed + 2) = (unsigned char)(out3 >> shift);
	*(delayed + 3) = (unsigned char)(out4 >> shift);
}

inline void _ycairo_pixel_average::_reset()
{
	out = 0;

	out1 = 0;
	out2 = 0;
	out3 = 0;
	out4 = 0;
}


void ycairo_drop_shadow::ycairo_drop_shadow_set_opacity(cairo_t * cr, double opacity)
{
	int props_index = _get_props_index(cr);
	props[props_index].shadow_opacity = opacity;
}

void ycairo_drop_shadow::ycairo_drop_shadow_set_radius(cairo_t * cr, double radius)
{
	int props_index = _get_props_index(cr);
	props[props_index].shadow_radius = IPMAX(radius, 1);
}

void ycairo_drop_shadow::ycairo_drop_shadow_set_distance(cairo_t * cr, double distance)
{
	int props_index = _get_props_index(cr);
	props[props_index].shadow_distance = distance;
	_calculate_shadow_offset(props_index);
}

void ycairo_drop_shadow::ycairo_drop_shadow_set_angle(cairo_t * cr, double angle)
{
	int props_index = _get_props_index(cr);
	props[props_index].shadow_angle = IPMAX(IPMIN(angle, 180), -180);
	_calculate_shadow_offset(props_index);
}

void ycairo_drop_shadow::_calculate_shadow_offset(int props_index)
{
	props[props_index].shadow_offset_x = props[props_index].shadow_distance * cos((1 - (props[props_index].shadow_angle / 180)) * 3.14159265359);
	props[props_index].shadow_offset_y = props[props_index].shadow_distance * sin((1 - (props[props_index].shadow_angle / 180)) * 3.14159265359);
}

int ycairo_drop_shadow::_get_props_index(cairo_t * cr)
{
	// If there is no cairo_t* inside vector, create new
	for (int i = 0; i < props.size(); i++)
	{
		if (props[i].cr == cr) return i;
	}

	props.push_back(shadow_properties());

	int index = props.size() - 1;
	props[index].cr = cr;
	_calculate_shadow_offset(index);

	return index;
}

void ycairo_drop_shadow::ycairo_drop_shadow_fill(cairo_t * cr, double downsample)
{
	_ycairo_draw_drop_shadow(cr, false, downsample);
}

void ycairo_drop_shadow::ycairo_drop_shadow_fill_fast(cairo_t * cr)
{
	_ycairo_draw_drop_shadow_fast(cr, false);
}

void ycairo_drop_shadow::ycairo_drop_shadow_stroke(cairo_t * cr, double downsample)
{
	_ycairo_draw_drop_shadow(cr, true, downsample);
}

void ycairo_drop_shadow::ycairo_drop_shadow_stroke_fast(cairo_t * cr)
{
	_ycairo_draw_drop_shadow_fast(cr, true);
}