#include "IPlugEffectGUILiveEdit.h"
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

	kKnobFrames = 60
};

IPlugEffectGUILiveEdit::IPlugEffectGUILiveEdit(IPlugInstanceInfo instanceInfo)
	: IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo)
{
	TRACE;

	//arguments are: name, defaultVal, minVal, maxVal, step, label
	GetParam(kGain)->InitDouble("Gain", 50., 0., 100.0, 0.01, "%");
	GetParam(kGain)->SetShape(2.);

	IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight);

	// Here we are attaching our GUI resize control ------------------------------------------------------------------------------
	// It is important to create on top of all controls!
	AttachGUIResize(new IPlugGUIResize(this, pGraphics, true, 16, 16));
	// ---------------------------------------------------------------------------------------------------------------------------

	pGraphics->AttachPanelBackground(&COLOR_GRAY);

	// Start Live GUI edit
	GUI_EDIT_START;

	NEW_LAYER;
	IRECT knobPosition = DRAW_RECT(IRECT(110, 120, 158, 168));
	IBitmap* knob = pGraphics->LoadPointerToBitmap(KNOB_ID, KNOB_FN, kKnobFrames);
	pGraphics->AttachControl(new IKnobMultiControl(this, knobPosition.L, knobPosition.T, kGain, knob));
	END;

	NEW_LAYER;
	IColor textColor = IColor(255, 0, 0, 0);
	IText textProps(24, &textColor, "Arial", IText::kStyleNormal, IText::kAlignCenter, 0, IText::kQualityDefault);
	pGraphics->AttachControl(new ITextControl(this, DRAW_RECT(IRECT(70, 40, 210, 80)), &textProps, "Hello IPlug!"));
	END;

	// End Live GUI edit
	GUI_EDIT_FINISH;

	// Attach GUI live edit control. Use __FILE__ macro to get source file path. You can remove it after you edit your GUI. 
	pGraphics->AttachControl(new IPlugGUILiveEdit(this, __FILE__, 10));

	AttachGraphics(pGraphics);

	//MakePreset("preset 1", ... );
	MakeDefaultPreset((char *) "-", kNumPrograms);
}

IPlugEffectGUILiveEdit::~IPlugEffectGUILiveEdit() {}

void IPlugEffectGUILiveEdit::ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames)
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

void IPlugEffectGUILiveEdit::Reset()
{
	TRACE;
	IMutexLock lock(this);
}

void IPlugEffectGUILiveEdit::OnParamChange(int paramIdx)
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
