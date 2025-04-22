#ifndef __CONTROLLERMANAGER_H__
#define __CONTROLLERMANAGER_H__

#include "../../SDL2/include/SDL.h"
#include "../../SexyAppFramework/Common.h"
#include "../../GameConstants.h"


class Controller
{
private:
	LawnApp*			mApp;
	SDL_GameController* mGameController;
	float				mThreshold;
	float				mDeadZone;
	std::map<SDL_GameControllerButton, bool> mButtonStates;
	std::map<SDL_GameControllerButton, bool> mLastButtonStates;

public:
	Controller(LawnApp* theApp, SDL_GameController* theGameController, float theThreshold, float theDeadZone);
	~Controller();

	void				UpdateButtonStates();
	void				ClearButtonStates();
	bool                GetButton(SDL_GameControllerButton theButton);
	bool				GetButtonDown(SDL_GameControllerButton theButton);
	bool				GetButtonUp(SDL_GameControllerButton theButton);
	bool				IsAxisActive(SDL_GameControllerAxis theAxis);
	int					GetAxisRawValue(SDL_GameControllerAxis theAxis);
	float				GetAxisValue(SDL_GameControllerAxis theAxis);
	float				GetTriggerAxisValue(SDL_GameControllerAxis theAxis);
	void				Rumble(float theLow, float theHigh, int theDuration);
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
	ControllerManager(LawnApp* theApp);
	~ControllerManager();

	void				Update();
	bool				IsActive();
	Controller*			GetController(int theIndex);
	void				RumbleAll(float theLow, float theHigh, int theDuration);
};

#endif