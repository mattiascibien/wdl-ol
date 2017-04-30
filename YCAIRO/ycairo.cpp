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
	mControl = pControl;

	if (ycairo_base->GetIPlugBase()->GetGUIResize())
	{
		draw_rect = mControl->GetNonScaledDrawRECT();
	}
	else
	{
		draw_rect = mControl->GetDrawRECT();
	}

	base_width = ycairo_base->get_width();
	base_height = ycairo_base->get_height();

	ycairo = ycairo_base;
}

void ycairo_gui::ycairo_reset_clip(cairo_t * cr)
{
	//cairo_new_path(cr);
	cairo_reset_clip(cr);
	cairo_rectangle(cr, draw_rect->L, draw_rect->T, draw_rect->W(), draw_rect->H());
	cairo_clip(cr);
}

void ycairo_gui::ycairo_rounded_rectangle(cairo_t * cr, double x, double y, double width, double height, double corner)
{
	cairo_new_sub_path(cr);
	cairo_arc(cr, x + width - corner, y + corner, corner, -1.5707963267948966192313216916398, 0);
	cairo_arc(cr, x + width - corner, y + height - corner, corner, 0, 1.5707963267948966192313216916398);
	cairo_arc(cr, x + corner, y + height - corner, corner, 1.5707963267948966192313216916398, 3.1415926535897932384626433832796);
	cairo_arc(cr, x + corner, y + corner, corner, 3.1415926535897932384626433832796, 4.7123889803846898576939650749193);
	cairo_close_path(cr);
}

void ycairo_gui::ycairo_circle(cairo_t * cr, double x, double y, double radius)
{
	cairo_arc(cr, x, y, radius, 0, 6.283185307179586476925286766559);
}

void ycairo_gui::ycairo_set_source_rgba(cairo_t * cr, IColor color)
{
	cairo_set_source_rgba(cr, color.R / 255.0, color.G / 255.0, color.B / 255.0, color.A / 255.0);
}

void ycairo_gui::ycairo_set_source_rgba(cairo_t * cr, IColor *color)
{
	cairo_set_source_rgba(cr, color->R / 255.0, color->G / 255.0, color->B / 255.0, color->A / 255.0);
}

void ycairo_gui::ycairo_set_source_rgba_fast(cairo_t * cr, IColor color)
{
	cairo_set_source_rgba(cr, color.R / 256.0, color.G / 256.0, color.B / 256.0, color.A / 256.0);
}
void ycairo_gui::ycairo_set_source_rgba_fast(cairo_t * cr, IColor *color)
{
	cairo_set_source_rgba(cr, color->R / 256.0, color->G / 256.0, color->B / 256.0, color->A / 256.0);
}

void ycairo_gui::ycairo_triangle(cairo_t * cr, double x0, double y0, double x1, double y1, double x2, double y2)
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
	cairo_rectangle(cr, draw_rect->L, draw_rect->T, draw_rect->W(), draw_rect->H());
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

void ycairo_text::ycairo_set_text_position(cairo_t * cr, IRECT rect, ycairo_text_w_aligement w_aligement, ycairo_text_h_aligement h_aligement)
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
		x = text_rect.L + (((double)text_rect.W() - text_extents->width - text_extents->x_bearing) / 2.0);
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
