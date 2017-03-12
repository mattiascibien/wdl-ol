#ifndef __IPLUGEFFECTGUIRESIZE__
#define __IPLUGEFFECTGUIRESIZE__

#include "IPlug_include_in_plug_hdr.h"

class IPlugEffectGUIResize : public IPlug
{
public:
  IPlugEffectGUIResize(IPlugInstanceInfo instanceInfo);
  ~IPlugEffectGUIResize();

  void Reset();
  void OnParamChange(int paramIdx);
  void ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames);

private:
  double mGain = 1.0;
};

#endif
