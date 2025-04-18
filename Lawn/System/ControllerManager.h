#ifndef __CONTROLLERMANAGER_H__
#define __CONTROLLERMANAGER_H__

#include "../../SDL2/include/SDL.h"
#include "../../GameConstants.h"
#include "../../ConstEnums.h"
#include "../../SexyAppFramework/Color.h"
#include "../CursorObject.h"
#include "../ToolTipWidget.h"

//could add threshold to right joystick and trigger threshold

class Controller
{
private:
	LawnApp*			mApp;
	SDL_GameController* mSDLGameController;
	Sexy::Color			mColor;
	float				mThreshold;
	float				mDeadZone;
	std::map<SDL_GameControllerButton, bool> mButtonStates;
	std::map<SDL_GameControllerButton, bool> mLastButtonStates;

public:
	Controller(LawnApp* theApp, SDL_GameController* theController, Sexy::Color theColor, float theThreshold, float theDeadZone);
	~Controller();

	Sexy::Color			GetColor();
	bool                GetButton(SDL_GameControllerButton theButton);
	bool				GetButtonDown(SDL_GameControllerButton theButton);
	bool				GetButtonUp(SDL_GameControllerButton theButton);
	void				UpdateButtonStates();
	void				ClearButtonStates();
	bool				IsAxisActive(SDL_GameControllerAxis theAxis);
	int					GetAxisRawValue(SDL_GameControllerAxis theAxis);
	float				GetAxisValue(SDL_GameControllerAxis theAxis);
	float				GetTriggerAxisValue(SDL_GameControllerAxis theAxis);
	void				Rumble(float theLow, float theHigh, int theDuration);
};

//actually initialize this class, dont be checking for controllers due to the previous indexes. if it gets deleted, no previous index aswell

class ControllerPlayer
{
private:
	int					mIndex;

public:
	LawnApp*			mApp;
	Controller*			mController;
	int					mBoardX;
	int					mBoardY;
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
	ControllerPlayer(LawnApp* theApp, int theIndex);
	~ControllerPlayer();

	void							Update();
};

class ControllerManager
{
private:
	LawnApp*			mApp;
	bool				mIsInitialized;
	std::map<SDL_JoystickID, int> mControllerMap;
	Controller*			mController[MAX_CONTROLLERS];
	int					mCurrentMouse;

public:
	ControllerManager();
	~ControllerManager();

	void				Update();
	bool				IsActive();
	Controller*			GetController(int theIndex);
	void				RumbleAll(float theLow, float theHigh, int theDuration);
};

#endif