#pragma once

#include "IPlugBase.h"
#include "IPlugDelegate.h"

/**
 * @file
 * @copydoc IPlugBaseGraphics
 */

class IGraphics;

/** An IPlugBase class for an IPlug plug-in that uses IGraphics for it's UI */
class IPlugBaseGraphics : public IPlugBase
                        , public IDelegate
{
public:
  IPlugBaseGraphics(IPlugConfig config, EAPI plugAPI);
  ~IPlugBaseGraphics();

  //IPlugBase
  void* GetAAXViewInterface() override { return (void*) GetUI(); }
  void* OpenWindow(void* pHandle) override;
  void CloseWindow() override;
  void AttachGraphics(IGraphics* pGraphics);

  /** If you override this method you should call this parent, or implement the same functionality in order to get controls to update. */
  virtual void OnRestoreState() override;
  void SendParameterValueToUIFromAPI(int paramIdx, double value, bool normalized) override;
  void PrintDebugInfo() const override;

  //IDelegate
  IParam* GetParamFromUI(int paramIdx) override { return GetParam(paramIdx); }
  void SetParameterValueFromUI(int paramIdx, double value) override { SetParameterValue(paramIdx, value); }
  void BeginInformHostOfParamChangeFromUI(int paramIdx) override { BeginInformHostOfParamChange(paramIdx); }
  void EndInformHostOfParamChangeFromUI(int paramIdx) override { EndInformHostOfParamChange(paramIdx); }
  
  //IPlugBaseGraphics
  IGraphics* GetUI() { assert(mGraphics); return mGraphics; }
private:
  IGraphics* mGraphics = nullptr;
};