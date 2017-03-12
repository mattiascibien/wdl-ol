#ifndef __IPLUGEFFECTGUILIVEEDIT__
#define __IPLUGEFFECTGUILIVEEDIT__

#include "IPlug_include_in_plug_hdr.h"

class IPlugEffectGUILiveEdit : public IPlug
{
public:
  IPlugEffectGUILiveEdit(IPlugInstanceInfo instanceInfo);
  ~IPlugEffectGUILiveEdit();

  void Reset();
  void OnParamChange(int paramIdx);
  void ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames);

private:
  double mGain = 1.0;
};

#endif
