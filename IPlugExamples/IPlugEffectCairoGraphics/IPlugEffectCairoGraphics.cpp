#include "IPlugEffectCairoGraphics.h"
#include "IPlug_include_in_plug_src.h"
#include "IControl.h"
#include "resource.h"

const int kNumPrograms = 1;

enum EParams
{
  kNumParams
};

IPlugEffectCairoGraphics::IPlugEffectCairoGraphics(IPlugInstanceInfo instanceInfo)
  :	IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo)
{
  TRACE;
  IGraphics* pGraphics = MakeGraphics(this, GUI_WIDTH, GUI_HEIGHT);

  GetYCAIRO()->attach_background(pGraphics, COLOR_GRAY);

  pGraphics->AttachControl(new CustomCairoControl(this, GetYCAIRO(), IRECT(0, 0, GUI_WIDTH, GUI_HEIGHT)));

  AttachGraphics(pGraphics);

  //MakePreset("preset 1", ... );
  MakeDefaultPreset((char *) "-", kNumPrograms);
}

IPlugEffectCairoGraphics::~IPlugEffectCairoGraphics() {}

void IPlugEffectCairoGraphics::ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames)
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

void IPlugEffectCairoGraphics::Reset()
{
  TRACE;
  IMutexLock lock(this);
}

void IPlugEffectCairoGraphics::OnParamChange(int paramIdx)
{
  IMutexLock lock(this);

  switch (paramIdx)
  {
    default:
      break;
  }
}
