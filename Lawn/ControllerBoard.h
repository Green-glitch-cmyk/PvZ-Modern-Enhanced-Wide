#ifndef __CONTROLLERBOARD_H__
#define __CONTROLLERBOARD_H__

#include "GameObject.h"

using namespace Sexy;

class CursorObject;
class CursorPreview;
class ToolTipWidget;

class ControllerBoard : public GameObject
{
public:
	int					mIndex;
	Color				mColor;
	SeedType			mSeedChooserSeed;
	SeedType			mSeedChooserPrevSeed;
	int					mSeedBankIndex;
	int					mSeedBankPrevIndex;
	CursorObject*		mCursorObject;
	CursorPreview*		mCursorPreview;
	ToolTipWidget*		mSeedChooserToolTip;
	int					mArrowStartMotion;
	int					mArrowEndMotion;
	int					mSeedChooserMoveMotion;
	int					mSeedChooserButtonMotion;

public:
	void				ControllerBoardInitialize(int theIndex);
	void				Update();
	void				UpdateColor();
};

#endif
