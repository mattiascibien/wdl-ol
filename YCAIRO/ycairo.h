#ifndef YCAIRO_H
#define YCAIRO_H

/*
 Youlean - ycairo - small library for enabling Cairo graphics support in IPlug
 
 Copyright (C) 2016 and later, Youlean
 
 This software is provided 'as-is', without any express or implied
 warranty.  In no event will the authors be held liable for any damages
 arising from the use of this software.
 
 Permission is granted to anyone to use this software for any purpose,
 including commercial applications, and to alter it and redistribute it
 freely, subject to the following restrictions:
 
 1. The origin of this software must not be misrepresented; you must not
 claim that you wrote the original software. If you use this software
 in a product, an acknowledgment in the product documentation would be
 appreciated but is not required.
 2. This notice may not be removed or altered from any source distribution.
 
 */

#ifdef _WIN32
#ifndef CAIRO_WIN32_STATIC_BUILD
#define CAIRO_WIN32_STATIC_BUILD
#endif

#if defined(_WIN64)
#if defined(_DEBUG)
#pragma comment(lib,"..\\..\\YCAIRO\\Cairo_Graphics\\Lib-Win\\cairo-x64-Debug.lib")
#else
#pragma comment(lib,"..\\..\\YCAIRO\\Cairo_Graphics\\Lib-Win\\cairo-x64-Release.lib")
#endif

#else // If Win32

#if defined(_DEBUG)
#pragma comment(lib,"..\\..\\YCAIRO\\Cairo_Graphics\\Lib-Win\\cairo-Win32-Debug.lib")
#else
#pragma comment(lib,"..\\..\\YCAIRO\\Cairo_Graphics\\Lib-Win\\cairo-Win32-Release.lib")
#endif
#endif

#elif defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#endif

#include "../WDL/IPlug/IPlugGUIResize.h"

#include <cairo.h>
#include <cairo-pdf.h>
#include <cairo-ft.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <string>
#include "../WDL/IPlug/IControl.h"


typedef enum _ycairo_text_w_aligement { YCAIRO_TEXT_W_ALIGN_LEFT, YCAIRO_TEXT_W_ALIGN_RIGHT, YCAIRO_TEXT_W_ALIGN_CENTER } ycairo_text_w_aligement;
typedef enum _ycairo_text_h_aligement { YCAIRO_TEXT_H_ALIGN_CENTER, YCAIRO_TEXT_H_ALIGN_TOP, YCAIRO_TEXT_H_ALIGN_BOTTOM } ycairo_text_h_aligement;


// Background drawing class
class ycairo_background : public IControl
{
public:
	ycairo_background(IPlugBase *pPlug, ycairo_base *ycairo_base, IGraphics *pGraphics, double red, double green, double blue);

	void AfterGUIResize(double guiScaleRatio);
	bool Draw(IGraphics* pGraphics);

private:
	double bg_red;
	double bg_green;
	double bg_blue;
	cairo_surface_t *bg_surface;
	cairo_t *bg_cr;
	IRECT *tmp;
	ycairo_base *ycairo;
	IGraphics *mGraphics;
};

class ycairo_base
{

public:

	ycairo_base(IPlugBase *pPlug);

	~ycairo_base();

#ifdef _WIN32
	void set_HINSTANCE(HINSTANCE hinstance);
	HINSTANCE get_HINSTANCE();
#elif defined(__APPLE__)
	void set_BUNDLE_ID(const char* _bundleID);
	const char* get_BUNDLE_ID();
#endif

	void create_global_font_from_path(const char* path);
	void create_global_font_from_memory(int name, int type, const char* relative_path);

	void bind_to_lice(IGraphics *pGraphics);
	void attach_background(IGraphics *pGraphics, IColor color);

	cairo_t *get_cr();
	cairo_surface_t *get_surface();

	int get_width();
	int get_height();

	FT_Library *get_global_ft_lib();
	FT_Face *get_global_ft_face();

	bool global_font_initialized();

	IPlugBase *GetIPlugBase();

private:
	cairo_surface_t *surface = NULL;
	cairo_t *cr = NULL;
	int base_width, base_height;
	double red, green, blue;
	IPlugBase *ycairo_iplug_base;
	bool global_font = false;

	FT_Library ft_library;
	FT_Face ft_face;

#ifdef _WIN32
	HINSTANCE hinstance_handle;
#elif defined(__APPLE__)
    const char* bundleID;
#endif
};

class ycairo_gui
{
public:

	ycairo_gui(ycairo_base *ycairo_base, IControl *pControl);
	
	//void ycairo_update_draw_rect(IRECT rect)
	//{
	//	draw_rect = rect;
	//}

	void ycairo_rounded_rectangle(cairo_t *cr, double x, double y, double width, double height, double corner);
	void ycairo_circle(cairo_t *cr, double x, double y, double radius);
	void ycairo_triangle(cairo_t *cr, double x0, double y0, double x1, double y1, double x2, double y2);

	void ycairo_reset_clip(cairo_t *cr);
	
	void ycairo_set_source_rgba(cairo_t *cr, IColor color);
	void ycairo_set_source_rgba(cairo_t *cr, IColor *color);
	void ycairo_set_source_rgba_fast(cairo_t *cr, IColor color);
	void ycairo_set_source_rgba_fast(cairo_t *cr, IColor *color);
	void ycairo_convert_colors_to_grayscale(cairo_t *cr)
	{

	}
	
	void ycairo_prepare_draw();
	void ycairo_draw();

private:
	IRECT *draw_rect;
	int size;
	int base_width, base_height;
	unsigned char *surface_out_test;

	ycairo_base *ycairo;

protected:
	cairo_surface_t *surface;
	cairo_surface_t *surface_out;
	cairo_t *cr;
	cairo_t *cr_out;
	IControl *mControl;
};

class ycairo_text
{

public:
	ycairo_text(ycairo_base *ycairo_base);

	~ycairo_text();


	void ycairo_create_font_from_path(const char* path);
	void ycairo_create_font_from_memory(int name, int type, const char* relative_path);

	void ycairo_initialize_font_face(cairo_t *cr);
	void ycairo_destroy_font_face();

	void ycairo_set_text(cairo_t *cr, const char *text);

	void ycairo_set_text_position(cairo_t *cr, IRECT rect, ycairo_text_w_aligement w_aligement = YCAIRO_TEXT_W_ALIGN_CENTER, ycairo_text_h_aligement h_aligement = YCAIRO_TEXT_H_ALIGN_CENTER);

	void ycairo_calculate_extents(cairo_t *cr);

	cairo_font_extents_t* ycairo_get_font_extents(cairo_t *cr);
	cairo_text_extents_t* ycairo_get_text_extents(cairo_t *cr);

	void ycairo_show_text(cairo_t *cr, const char *text, double size, IColor color, IRECT rect,
		ycairo_text_w_aligement w_aligement = YCAIRO_TEXT_W_ALIGN_CENTER, ycairo_text_h_aligement h_aligement = YCAIRO_TEXT_H_ALIGN_CENTER);

	void ycairo_show_text(cairo_t *cr);

private:
	const char *draw_text = "";
	IRECT text_rect;
	ycairo_text_w_aligement width_aligement;
	ycairo_text_h_aligement height_aligement;

	cairo_font_face_t *current_font_face;
	cairo_text_extents_t *text_extents, *ext_height;
	cairo_font_extents_t *font_extents;
	unsigned char *surface_out_test;

	FT_Library *global_ft_library;
	FT_Face *global_ft_face;

	FT_Library local_ft_library;
	FT_Face local_ft_face;

	bool global_font_initialized;
	bool local_font_initialized = false;

#ifdef _WIN32
	HINSTANCE hinstance_handle;
#elif defined(__APPLE__)
	const char* bundleID;
#endif

};

#endif