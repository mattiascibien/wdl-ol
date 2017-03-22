#ifndef __IPLUGEFFECTCAIROGRAPHICS__
#define __IPLUGEFFECTCAIROGRAPHICS__

#include "IPlug_include_in_plug_hdr.h"

#define M_PI       3.14159265358979323846

class IPlugEffectCairoGraphics : public IPlug
{
public:
  IPlugEffectCairoGraphics(IPlugInstanceInfo instanceInfo);
  ~IPlugEffectCairoGraphics();

  void Reset();
  void OnParamChange(int paramIdx);
  void ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames);

private:
  double mGain = 1.0;
};


class CustomCairoControl : public IControl, ycairo_gui
{
public:
	CustomCairoControl(IPlugBase* pPlug, ycairo_base *ycairo_base, IRECT pR)
		: IControl(pPlug, pR), ycairo_gui(ycairo_base, this) {}


	bool Draw(IGraphics* pGraphics)
	{
		ycairo_prepare_draw();

		// Custom cairo code -----------------------------------------------------------------------------------------------------------------------------------

		cairo_set_line_width(cr, 6);

		cairo_rectangle(cr, 12, 12, 232, 70);
		cairo_new_sub_path(cr); cairo_arc(cr, 64, 64, 40, 0, 2 * M_PI);
		cairo_new_sub_path(cr); cairo_arc_negative(cr, 192, 64, 40, 0, -2 * M_PI);

		cairo_set_fill_rule(cr, CAIRO_FILL_RULE_EVEN_ODD);
		cairo_set_source_rgb(cr, 0, 0.7, 0); cairo_fill_preserve(cr);
		cairo_set_source_rgb(cr, 0, 0, 0); cairo_stroke(cr);

		cairo_translate(cr, 0, 128);
		cairo_rectangle(cr, 12, 12, 232, 70);
		cairo_new_sub_path(cr); cairo_arc(cr, 64, 64, 40, 0, 2 * M_PI);
		cairo_new_sub_path(cr); cairo_arc_negative(cr, 192, 64, 40, 0, -2 * M_PI);

		cairo_set_fill_rule(cr, CAIRO_FILL_RULE_WINDING);
		cairo_set_source_rgb(cr, 0, 0, 0.9); cairo_fill_preserve(cr);
		cairo_set_source_rgb(cr, 0, 0, 0); cairo_stroke(cr);

		cairo_translate(cr, 0, -128);

		// ------------------------------------------------------------------------------------------------------------------------------------------------------


		ycairo_draw();
		return true;
	}



private:
};

#endif
