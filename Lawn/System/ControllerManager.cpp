#include "ControllerManager.h"
#include "../../Sexy.TodLib/TodCommon.h"
#include "../../LawnApp.h"
#include "../../Lawn/Board.h"
#include "../../Lawn/ControllerBoard.h"

Controller::Controller(LawnApp* theApp, SDL_GameController* theGameController, float theThreshold, float theDeadZone)
{
    mApp = theApp;
	mGameController = theGameController;
	mThreshold = theThreshold;
	mDeadZone = theDeadZone;
}

Controller::~Controller()
{
	SDL_GameControllerClose(mGameController);
}

void Controller::UpdateButtonStates()
{
	mLastButtonStates = mButtonStates;
	for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; i++)
	{
		SDL_GameControllerButton aButton = (SDL_GameControllerButton)i;
		mButtonStates[aButton] = SDL_GameControllerGetButton(mGameController, aButton);
	}
}

void Controller::ClearButtonStates()
{
	for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; i++)
	{
		SDL_GameControllerButton aButton = (SDL_GameControllerButton)i;
		mButtonStates[aButton] = false;
		mLastButtonStates[aButton] = false;
	}
}

bool Controller::GetButton(SDL_GameControllerButton theButton)
{
    return mButtonStates[theButton];
}

bool Controller::GetButtonDown(SDL_GameControllerButton theButton)
{
    return GetButton(theButton) && !mLastButtonStates[theButton];
}

bool Controller::GetButtonUp(SDL_GameControllerButton theButton)
{
    return !GetButton(theButton) && mLastButtonStates[theButton];
}

bool Controller::IsAxisActive(SDL_GameControllerAxis theAxis)
{
    if (!mApp->mHasFocus || theAxis == SDL_CONTROLLER_AXIS_TRIGGERLEFT || theAxis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT)
        return false;
	return fabs((float)GetAxisRawValue(theAxis) / INT16_MAX) > mThreshold;
}

int Controller::GetAxisRawValue(SDL_GameControllerAxis theAxis)
{
    if (!mApp->mHasFocus)
        return 0;
    return (int)SDL_GameControllerGetAxis(mGameController, theAxis);
}

float Controller::GetAxisValue(SDL_GameControllerAxis theAxis)
{
    if (!mApp->mHasFocus || theAxis == SDL_CONTROLLER_AXIS_TRIGGERLEFT || theAxis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT)
        return 0.0f;
	return ClampFloat(min((float)GetAxisRawValue(theAxis) / INT16_MAX, mDeadZone) / mDeadZone, -1, 1);
}

float Controller::GetTriggerAxisValue(SDL_GameControllerAxis theAxis)
{
    if (mApp->mHasFocus && (theAxis == SDL_CONTROLLER_AXIS_TRIGGERLEFT || theAxis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT))
        return (float)GetAxisRawValue(theAxis) / INT16_MAX;
    return 0.0f;
}

void Controller::Rumble(float theLow, float theHigh, int theDuration)
{
	SDL_GameControllerRumble(mGameController, ClampFloat(theLow, 0, 1) * UINT16_MAX, ClampFloat(theHigh, 0, 1) * UINT16_MAX, theDuration);
}

ControllerManager::ControllerManager(LawnApp* theApp)
{
	mApp = theApp;
	mIsInitialized = SDL_Init(SDL_INIT_GAMECONTROLLER | SDL_INIT_JOYSTICK) == 0;
	memset(mController, 0, sizeof(mController));
	mCurrentMouse = -1;
	if (!mIsInitialized)
		MessageBox(NULL, StrFormat("Couldn't initialize SDL2 for the game controllers. Error: %s", SDL_GetError()).c_str(), "SDL2 Error", MB_OK | MB_ICONERROR);
}

ControllerManager::~ControllerManager()
{
	for (int i = 0; i < MAX_CONTROLLERS; i++)
	{
		if (mController[i])
			delete mController[i];
	}
	SDL_Quit();
}

void ControllerManager::Update()
{
	if (!mIsInitialized)
		return;

	const int aMaxSpeed = 7;
	static int aMouseX = -1;
	static int aMouseY = -1;
	static int aSpeedX = 0;
	static int aSpeedY = 0;

	if (aSpeedX != 0 || aSpeedY != 0)
	{
		if (aMouseX == -1 && aMouseY == -1)
		{
			aMouseX = mApp->mWidgetManager->mLastMouseX;
			aMouseY = mApp->mWidgetManager->mLastMouseY;
		}
		aMouseX = ClampInt(aMouseX + aSpeedX, 0, BOARD_WIDTH - 1);
		aMouseY = ClampInt(aMouseY + aSpeedY, 0, BOARD_HEIGHT - 1);
		POINT aPoint = { aMouseX, aMouseY };
		::ClientToScreen(mApp->mHWnd, &aPoint);
		::SetCursorPos(aPoint.x, aPoint.y);
	}
	else
	{
		aMouseX = -1;
		aMouseY = -1;
	}

	for (int i = 0; i < MAX_CONTROLLERS; i++)
	{
		Controller* aController = mController[i];
		if (!aController)
			continue;

		if (mApp->mHasFocus)
			aController->UpdateButtonStates();
		else
		{
			aController->ClearButtonStates();
			continue;
		}
		if (aController->GetButtonDown(SDL_CONTROLLER_BUTTON_LEFTSTICK))
		{
			if (mCurrentMouse == -1)
				mCurrentMouse = i;
			else if (mCurrentMouse == i)
			{
				mCurrentMouse = -1;
				aSpeedX = 0;
				aSpeedY = 0;
			}
		}
		if (mCurrentMouse == i)
		{
			if (aController->IsAxisActive(SDL_CONTROLLER_AXIS_LEFTX))
				aSpeedX = aMaxSpeed * aController->GetAxisValue(SDL_CONTROLLER_AXIS_LEFTX);
			else
			{
				if (aController->GetButton(SDL_CONTROLLER_BUTTON_DPAD_LEFT))
					aSpeedX = -aMaxSpeed;
				else if (aController->GetButton(SDL_CONTROLLER_BUTTON_DPAD_RIGHT))
					aSpeedX = aMaxSpeed;
				else
					aSpeedX = 0;
			}
			if (aController->IsAxisActive(SDL_CONTROLLER_AXIS_LEFTY))
				aSpeedY = aMaxSpeed * aController->GetAxisValue(SDL_CONTROLLER_AXIS_LEFTY);
			else
			{
				if (aController->GetButton(SDL_CONTROLLER_BUTTON_DPAD_UP))
					aSpeedY = -aMaxSpeed;
				else if (aController->GetButton(SDL_CONTROLLER_BUTTON_DPAD_DOWN))
					aSpeedY = aMaxSpeed;
				else
					aSpeedY = 0;
			}
			if (aController->GetButtonDown(SDL_CONTROLLER_BUTTON_A))
				SendMessage(mApp->mHWnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(mApp->mWidgetManager->mLastMouseX, mApp->mWidgetManager->mLastMouseY));
			else if (aController->GetButtonUp(SDL_CONTROLLER_BUTTON_A))
				SendMessage(mApp->mHWnd, WM_LBUTTONUP, 0, MAKELPARAM(mApp->mWidgetManager->mLastMouseX, mApp->mWidgetManager->mLastMouseY));

			if (aController->GetButtonDown(SDL_CONTROLLER_BUTTON_B))
				SendMessage(mApp->mHWnd, WM_RBUTTONDOWN, MK_RBUTTON, MAKELPARAM(mApp->mWidgetManager->mLastMouseX, mApp->mWidgetManager->mLastMouseY));
			else if (aController->GetButtonUp(SDL_CONTROLLER_BUTTON_B))
				SendMessage(mApp->mHWnd, WM_RBUTTONUP, 0, MAKELPARAM(mApp->mWidgetManager->mLastMouseX, mApp->mWidgetManager->mLastMouseY));
		}
	}

	SDL_Event aEvent;
	while (SDL_PollEvent(&aEvent))
	{
		int aIndex = -1;
		SDL_JoystickID aID = aEvent.cdevice.which;
		auto aIt = mControllerMap.find(aID);
		if (aIt != mControllerMap.end())
			aIndex = aIt->second;
		switch (aEvent.type)
		{
		case SDL_CONTROLLERDEVICEADDED:
			if (SDL_GameController* aGameController = SDL_GameControllerOpen(aID))
			{
				aID = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(aGameController));
				if (mControllerMap.find(aID) == mControllerMap.end())
				{
					for (int i = 0; i < MAX_CONTROLLERS; i++)
					{
						if (!mController[i])
						{
							aIndex = i;
							mController[aIndex] = new Controller(mApp, aGameController, 0.2f, 0.85f);
							mControllerMap[aID] = aIndex;
							break;
						}
					}
				}
				if (aIndex == -1)
					SDL_GameControllerClose(aGameController);
			}
			break;
		case SDL_CONTROLLERDEVICEREMOVED:
			if (aIndex != -1)
			{
				mControllerMap.erase(aIt);
				delete mController[aIndex];
				mController[aIndex] = nullptr;
				if (mCurrentMouse == aIndex)
				{
					mCurrentMouse = -1;
					aSpeedX = 0;
					aSpeedY = 0;
				}
				for (int i = aIndex + 1; i < MAX_CONTROLLERS; i++)
				{
					if (mController[i])
					{
						mController[i - 1] = mController[i];
						mController[i] = nullptr;
						for (auto& pair : mControllerMap)
						{
							if (pair.second == i)
							{
								pair.second = i - 1;
								break;
							}
						}
					}
				}
				if (mApp->mBoard)
				{
					ControllerBoard* aOldControllerBoard = mApp->mBoard->mControllerBoardList[aIndex];
					mApp->mBoard->mControllerBoardList[aIndex] = nullptr;
					for (int i = aIndex + 1; i < MAX_CONTROLLERS; i++)
					{
						if (mApp->mBoard->mControllerBoardList[i])
						{
							mApp->mBoard->mControllerBoardList[i - 1] = mApp->mBoard->mControllerBoardList[i];
							mApp->mBoard->mControllerBoardList[i] = nullptr;
							ControllerBoard* aControllerBoard = mApp->mBoard->mControllerBoardList[i - 1];
							aControllerBoard->mIndex--;
							mApp->mBoard->mControllerBoardList[i - 1]->UpdateColor();
						}
					}
					if (aOldControllerBoard)
					{
						int aNewIndex = -1;
						for (int i = 0; i < MAX_CONTROLLERS; i++)
						{
							if (!mApp->mBoard->mControllerBoardList[i])
							{
								aNewIndex = i;
								break;
							}
						}
						if (aNewIndex != -1)
						{
							mApp->mBoard->mControllerBoardList[aNewIndex] = aOldControllerBoard;
							aOldControllerBoard->mIndex = aNewIndex;
							aOldControllerBoard->UpdateColor();
						}
					}
				}
			}
			break;
		}
	}
}

bool ControllerManager::IsActive()
{
	return mIsInitialized;
}

Controller* ControllerManager::GetController(int theIndex)
{
	return mIsInitialized && mCurrentMouse != theIndex && mController[theIndex] != nullptr ? mController[theIndex] : nullptr;
}

void ControllerManager::RumbleAll(float theLow, float theHigh, int theDuration)
{
	if (!mIsInitialized)
		return;
	for (int i = 0; i < MAX_CONTROLLERS; i++)
	{
		if (Controller* aController = mController[i])
			aController->Rumble(theLow, theHigh, theDuration);
	}
}