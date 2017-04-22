#include "IPlugEffectGUIResize.h"
#include "IPlug_include_in_plug_src.h"
#include "IControl.h"
#include "resource.h"

const int kNumPrograms = 1;

enum EParams
{
  kGain = 0,
  kNumParams
};

enum ELayout
{
  kWidth = GUI_WIDTH,
  kHeight = GUI_HEIGHT,

  kGainX = 100,
  kGainY = 80,
  kKnobFrames = 60
};

// Adding names for views. Default view is needed, place it on top
enum viewSets
{
	defaultView, // Default view will always be at 0

    // Add here your custom views
	miniView,
	hugeView
};

IPlugEffectGUIResize::IPlugEffectGUIResize(IPlugInstanceInfo instanceInfo)
  :	IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo)
{
  TRACE;

  //arguments are: name, defaultVal, minVal, maxVal, step, label
  GetParam(kGain)->InitDouble("Gain", 50., 0., 100.0, 0.01, "%");
  GetParam(kGain)->SetShape(2.);

  IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight);
 
  // Here we are attaching our GUI resize control. It is important to create on top of all controls!
  AttachGUIResize(new IPlugGUIResize(this, pGraphics, true, 16, 16));

  // You must call UsingBitmaps() if you want to use GUI scaling with bitmaps
  GetGUIResize()->UsingBitmaps();

  // Adding a new view. Default view will always be 0.
  GetGUIResize()->AddNewView(miniView, 200, 200);
  GetGUIResize()->AddNewView(hugeView, 500, 200);

  GetGUIResize()->SelectViewMode(defaultView);

  // This will limit GUI scaling from 50% to 400%
  GetGUIResize()->SetGUIScaleLimits(50, 400);

  // This will limit normalized window size ie. window size that is not affected by GUI scale ratio.
  GetGUIResize()->SetWindowSizeLimits(defaultView, 300, 360, 600, 720);
  GetGUIResize()->SetWindowSizeLimits(miniView, 200, 200, 400, 400);
  GetGUIResize()->SetWindowSizeLimits(hugeView, 500, 200, 1000, 1440);

  // You can use bitmaps with higher resolution, so that when you resize interface up, everything will look nicely
  // This must be called before LoadPointerToBitmap
  pGraphics->SetBitmapOversample(2);

  pGraphics->AttachPanelBackground(&COLOR_GRAY);

  IColor textColor = IColor(255, 0, 0, 0);
  IText textProps(24, &textColor, "Arial", IText::kStyleNormal, IText::kAlignCenter, 0, IText::kQualityDefault);
  helloIPlugIndex = pGraphics->AttachControl(new ITextControl(this, IRECT(80, 40, 220, 80), &textProps, "Hello IPlug!"));

  IBitmap* knob = pGraphics->LoadPointerToBitmap(KNOB_ID, KNOB_FN, kKnobFrames);
  knobIndex = pGraphics->AttachControl(new IKnobMultiControl(this, kGainX, kGainY, kGain, knob));
  
  miniViewIndex = pGraphics->AttachControl(new viewSelector(this, IRECT(80, 200, 220, 220), "miniView", miniView));
  defaultViewIndex = pGraphics->AttachControl(new viewSelector(this, IRECT(80, 240, 220, 260), "defaultView", defaultView));
  hugeViewIndex = pGraphics->AttachControl(new viewSelector(this, IRECT(80, 280, 220, 300), "hugeView", hugeView));
  
  handleSelectorIndex = pGraphics->AttachControl(new handleSelector(this, IRECT(80, 320, 220, 340)));

  AttachGraphics(pGraphics);

  //MakePreset("preset 1", ... );
  MakeDefaultPreset((char *) "-", kNumPrograms);
}

IPlugEffectGUIResize::~IPlugEffectGUIResize() {}

void IPlugEffectGUIResize::SetGUILayout(int viewMode, double windowWidth, double windowHeight)
{
	// Use this function to move, hide, show and resize controls. windowWidth and windowHeight are not affected by GUI scaling
		
	// Every view will have it's own gui layout, so if you for example hide some control on miniView you don't
	// need to unhide it in defaultView because layout is separate for every view
	

	if (viewMode == defaultView)
	{
		GetGUIResize()->MoveControlHorizontally(*knobIndex, windowWidth - 200);
		GetGUIResize()->MoveControlHorizontally(*helloIPlugIndex, windowWidth - 220);
		GetGUIResize()->MoveControlHorizontally(*miniViewIndex, windowWidth - 220);
		GetGUIResize()->MoveControlHorizontally(*defaultViewIndex, windowWidth - 220);
		GetGUIResize()->MoveControlHorizontally(*hugeViewIndex, windowWidth - 220);
		GetGUIResize()->MoveControlHorizontally(*handleSelectorIndex, windowWidth - 220);
	}

	if (viewMode == miniView)
	{
		GetGUIResize()->MoveControl(*knobIndex, windowWidth - 150, 20);
		GetGUIResize()->MoveControl(*miniViewIndex, windowWidth - 170, 120);
		GetGUIResize()->MoveControl(*defaultViewIndex, windowWidth - 170, 140);
		GetGUIResize()->MoveControl(*hugeViewIndex, windowWidth - 170, 160);
		GetGUIResize()->MoveControl(*handleSelectorIndex, windowWidth - 170, 180);

		GetGUIResize()->HideControl(*helloIPlugIndex);
	}

	if (viewMode == hugeView)
	{
		GetGUIResize()->MoveControl(*knobIndex, 200, 70);
		GetGUIResize()->MoveControl(*helloIPlugIndex, 180, 30);
		GetGUIResize()->MoveControl(*miniViewIndex, 20, 80);
		GetGUIResize()->MoveControl(*defaultViewIndex, 20, 110);
		GetGUIResize()->MoveControl(*hugeViewIndex, 20, 140);

		GetGUIResize()->ResizeControlRelativeToWindowSize(*handleSelectorIndex);
	}
}

void IPlugEffectGUIResize::ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames)
{
  // Mutex is already locked for us.

  double* in1 = inputs[0];
  double* in2 = inputs[1];
  double* out1 = outputs[0];
  double* out2 = outputs[1];

  for (int s = 0; s < nFrames; ++s, ++in1, ++in2, ++out1, ++out2)
  {
    *out1 = *in1 * mGain;
    *out2 = *in2 * mGain;
  }
}

void IPlugEffectGUIResize::Reset()
{
  TRACE;
  IMutexLock lock(this);
}

void IPlugEffectGUIResize::OnParamChange(int paramIdx)
{
  IMutexLock lock(this);

  switch (paramIdx)
  {
    case kGain:
      mGain = GetParam(kGain)->Value() / 100.;
      break;

    default:
      break;
  }
}
