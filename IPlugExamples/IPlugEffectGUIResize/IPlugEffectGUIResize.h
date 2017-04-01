#ifndef __IPLUGEFFECTGUIRESIZE__
#define __IPLUGEFFECTGUIRESIZE__

#include "IPlug_include_in_plug_hdr.h"

class IPlugEffectGUIResize : public IPlug
{
public:
  IPlugEffectGUIResize(IPlugInstanceInfo instanceInfo);
  ~IPlugEffectGUIResize();

  void SetGUILayout(int viewMode, double windowWidth, double windowHeight);

  void Reset();
  void OnParamChange(int paramIdx);
  void ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames);

private:
  double mGain = 1.0;
  int *knobIndex, *helloIPlugIndex, *miniViewIndex, *defaultViewIndex, *hugeViewIndex, *handleSelectorIndex;
};

class viewSelector : public IControl
{
private:
	WDL_String mStr;
	int view_mode;

public:
	viewSelector(IPlugBase* pPlug, IRECT pR, const char* label, int viewMode)
		: IControl(pPlug, pR)
	{
		view_mode = viewMode;
		mStr.Set(label);
		mText.mColor = COLOR_WHITE;
		mText.mSize = 24;
	}

	~viewSelector() {}

	void AfterGUIResize(double guiScaleRatio)
	{
		mText = IText(mDrawRECT.H(), &mText.mColor,
			mText.mFont, mText.mStyle, mText.mAlign, mText.mOrientation,
			mText.mQuality, &mText.mTextEntryBGColor, &mText.mTextEntryFGColor);
	}

	bool Draw(IGraphics* pGraphics)
	{
		pGraphics->FillIRect(&COLOR_BLACK, &mDrawRECT, &mBlend);
		pGraphics->DrawIText(&mText, mStr.Get(), &mDrawRECT);
		return true;
	}

	void OnMouseDown(int x, int y, IMouseMod* pMod)
	{
		GetGUIResize()->SelectViewMode(view_mode);
		GetGUIResize()->ResizeGraphics();
	}

};

class handleSelector : public IControl
{
public:
	handleSelector(IPlugBase* pPlug, IRECT pR)
		: IControl(pPlug, pR)
	{
		scaling.Set("guiScaling");
		resize.Set("windowResizing");
		mText.mColor = COLOR_WHITE;
		mText.mSize = 24;
	}

	~handleSelector() {}

	void AfterGUIResize(double guiScaleRatio)
	{
		mText = IText(mDrawRECT.H(), &mText.mColor,
			mText.mFont, mText.mStyle, mText.mAlign, mText.mOrientation,
			mText.mQuality, &mText.mTextEntryBGColor, &mText.mTextEntryFGColor);
	}

	bool Draw(IGraphics* pGraphics)
	{
		pGraphics->FillIRect(&COLOR_BLUE, &mDrawRECT, &mBlend);

		if (!button)
		{
			pGraphics->DrawIText(&mText, resize.Get(), &mDrawRECT);
		}
		else
		{
			pGraphics->DrawIText(&mText, scaling.Get(), &mDrawRECT);
		}
		return true;
	}

	void OnMouseDown(int x, int y, IMouseMod* pMod)
	{
		if (button)
			button = false;
		else
			button = true;

		GetGUIResize()->UseHandleForGUIScaling(button);

		SetDirty();
	}


private:
	WDL_String scaling;
	WDL_String resize;
	IPlugGUIResize *GUIResize;
	bool button = false;
	int redrawTest = 0;
};
#endif
