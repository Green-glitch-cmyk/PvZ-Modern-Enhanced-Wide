#ifndef __GAMEPADMANAGER_H__
#define __GAMEPADMANAGER_H__

#include "../../SDL/include/SDL3/SDL.h"
#include "../../SexyAppFramework/Common.h"

using namespace std;

class LawnApp;

class Gamepad
{
private:
	LawnApp*				mApp;
	SDL_Gamepad*			mSDLGamepad;
	float                   mThreshold;
	bool					mButtons[SDL_GAMEPAD_BUTTON_COUNT];
	bool					mLastButtons[SDL_GAMEPAD_BUTTON_COUNT];

public:
	Gamepad(LawnApp* theApp, SDL_Gamepad* theGamepad, float theThreshold);
	~Gamepad();

	bool                    UpdateButtons();
	SDL_Gamepad*			GetSDLGamepad();
	bool                    GetButton(SDL_GamepadButton theButton);
	bool                    GetButtonDown(SDL_GamepadButton theButton);
	bool                    GetButtonUp(SDL_GamepadButton theButton);
	int                     GetAxisRawValue(SDL_GamepadAxis theAxis);
	float                   GetAxisValue(SDL_GamepadAxis theAxis);
	float                   GetTriggerAxisValue(SDL_GamepadAxis theAxis);
	void                    Rumble(float theLow, float theHigh, int theDuration);
};

class GamepadManager
{
private:
	LawnApp*				mApp;
	bool                    mIsInitialized;
	std::map<SDL_JoystickID, std::pair<int, Gamepad*>> mGamepadControllers;
	int                     mCurrentMouse;
	SDL_GamepadType         mLastUsedType;

public:
	GamepadManager(LawnApp* theApp);
	~GamepadManager();

	void                    Update();
	Gamepad*				GetGamepad(int theIndex);
	SDL_GamepadType         GetLastUsedType();
	void                    RumbleAll(float theLow, float theHigh, int theDuration);
};

#endif