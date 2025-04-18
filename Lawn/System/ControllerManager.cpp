#include "ControllerManager.h"
#include "../../Sexy.TodLib/TodCommon.h"
#include "../../Resources.h"
#include "../../LawnApp.h"
#include "../../Lawn/Board.h"
#include "../Cutscene.h"
#include "../../Lawn/Widget/SeedChooserScreen.h"
#include "../../Lawn/SeedPacket.h"
#include "../../Lawn/Widget/GameButton.h"

const Sexy::Color cPredefinedControllerColors[MAX_CONTROLLERS] = { Sexy::Color(255, 242, 0), Sexy::Color(71, 198, 255) };

ControllerManager::ControllerManager()
{
	mApp = nullptr;
	mIsInitialized = SDL_Init(SDL_INIT_GAMECONTROLLER | SDL_INIT_JOYSTICK) == 0;
	for (int i = 0; i < MAX_CONTROLLERS; i++)
		mController[i] = nullptr;
	mCurrentMouse = -1;
	if (!mIsInitialized)
		MessageBox(NULL, StrFormat("Couldn't initialize SDL2. Error: %s", SDL_GetError()).c_str(), "SDL2 Error", MB_OK | MB_ICONERROR);
}

ControllerManager::~ControllerManager()
{
	for (int i = 0; i < MAX_CONTROLLERS; i++)
	{
		if (mController[i] != nullptr)
		{
			delete mController[i];
			mController[i] = nullptr;
		}
	}
	SDL_Quit();
}

void ControllerManager::Update()
{
	if (mApp == nullptr)
	{
		mApp = gLawnApp;
		return;
	}
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
        if (Controller* aController = mController[i])
        {
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
            if (SDL_GameController* aController = SDL_GameControllerOpen(aID))
            {
                if (mControllerMap.find(aID) == mControllerMap.end())
                {
                    for (int i = 0; i < MAX_CONTROLLERS; i++)
                    {
                        if (mController[i] == nullptr)
                        {
                            aIndex = i;
                            mController[aIndex] = new Controller(mApp, aController, cPredefinedControllerColors[aIndex], 0.2f, 0.85f);
                            mControllerMap[aID] = aIndex;
                            break;
                        }
                    }
                }
                if (aIndex == -1)
                    SDL_GameControllerClose(aController);
            }
            break;
        case SDL_CONTROLLERDEVICEREMOVED:
            if (aIndex != -1)
            {
                delete mController[aIndex];
                mController[aIndex] = nullptr;
                mControllerMap.erase(aIt);
                if (mCurrentMouse == aIndex)
                {
                    mCurrentMouse = -1;
                    aSpeedX = 0;
                    aSpeedY = 0;
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

Controller::Controller(LawnApp* theApp, SDL_GameController* theController, Sexy::Color theColor, float theThreshold, float theDeadZone)
{
    mApp = theApp;
	mSDLGameController = theController;
	mColor = theColor;
	mThreshold = theThreshold;
	mDeadZone = theDeadZone;
}

Controller::~Controller()
{
	SDL_GameControllerClose(mSDLGameController);
}

Sexy::Color Controller::GetColor()
{
	return mColor;
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

void Controller::UpdateButtonStates()
{
    mLastButtonStates = mButtonStates;
    for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; i++)
    {
        SDL_GameControllerButton aButton = (SDL_GameControllerButton)i;
        mButtonStates[aButton] = SDL_GameControllerGetButton(mSDLGameController, aButton);
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
    return (int)SDL_GameControllerGetAxis(mSDLGameController, theAxis);
}

float Controller::GetAxisValue(SDL_GameControllerAxis theAxis)
{
    if (!mApp->mHasFocus || theAxis == SDL_CONTROLLER_AXIS_TRIGGERLEFT || theAxis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT)
        return 0.0f;
	return ClampFloat(min((float)GetAxisRawValue(theAxis) / INT16_MAX, mDeadZone) / mDeadZone, -1.0f, 1.0f);
}

float Controller::GetTriggerAxisValue(SDL_GameControllerAxis theAxis)
{
    if (mApp->mHasFocus && (theAxis == SDL_CONTROLLER_AXIS_TRIGGERLEFT || theAxis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT))
        return (float)GetAxisRawValue(theAxis) / INT16_MAX;
    return 0.0f;
}

void Controller::Rumble(float theLow, float theHigh, int theDuration)
{
	SDL_GameControllerRumble(mSDLGameController, ClampFloat(theLow, 0, 1) * 0xFFFF, ClampFloat(theHigh, 0, 1) * 0xFFFF, theDuration);
}

ControllerPlayer::ControllerPlayer(int theIndex)
{
	mIndex = theIndex;
	mApp = nullptr;
	mController = nullptr;
	mBoardX = 0;
	mBoardY = 0;
	mSeedChooserSeed = SEED_NONE;
	mSeedChooserPrevSeed = SEED_NONE;
	mSeedBankIndex = -1;
	mSeedBankPrevIndex = -1;
	mCursorObject = new CursorObject();
	mCursorPreview = new CursorPreview();
	mSeedChooserToolTip = new ToolTipWidget();
	mArrowStartMotion = -1;
	mArrowEndMotion = -1;
	mSeedChooserMoveMotion = -1;
	mSeedChooserButtonMotion = -1;
}

ControllerPlayer::~ControllerPlayer()
{
	delete mCursorObject;
	delete mCursorPreview;
	delete mSeedChooserToolTip;
}

void ControllerPlayer::Update()
{
	if (mApp == nullptr)
	{
		mApp = gLawnApp;
		return;
	}
	mController = mApp->mControllerManager->GetController(mIndex);
	if (mController != nullptr)
	{
		bool aIsSeedChoosing = mApp->mBoard->mCutScene->mSeedChoosing;
		if (aIsSeedChoosing)
			mSeedChooserToolTip->Update();

		if (mSeedChooserSeed == SEED_NONE)
		{
			mSeedChooserSeed = mSeedChooserPrevSeed != SEED_NONE ? mSeedChooserPrevSeed : SEED_PEASHOOTER;
			mSeedChooserPrevSeed = SEED_NONE;
		}

		if (mSeedBankIndex == -1 && !aIsSeedChoosing)
		{
			mSeedBankIndex = mSeedBankPrevIndex != -1 ? mSeedBankPrevIndex : 0;
			mSeedBankPrevIndex = -1;
		}

		if (mController->GetButtonDown(SDL_CONTROLLER_BUTTON_START))
		{
			if (!aIsSeedChoosing)
			{
				if (mApp->GetDialogCount() == 0)
				{
					mApp->mBoard->UpdateCursor();
					mApp->mBoard->ClearCursor();
					mApp->PlaySample(Sexy::SOUND_PAUSE);
					mApp->DoNewOptions(false);
				}
				else
				{
					mApp->KillNewOptionsDialog();
					mApp->KillDialog(Dialogs::DIALOG_PAUSED);
				}
			}
			else
			{
				//crashes when message appears
				if (!mApp->mSeedChooserScreen->mStartButton->mDisabled)
					mApp->mSeedChooserScreen->ButtonDepress(mApp->mSeedChooserScreen->mStartButton->mId);
			}
		}

		if (!mApp->mHasFocus || mApp->mBoard->mPaused)
			return;

		if (mApp->mGameScene == GameScenes::SCENE_PLAYING)
		{
			if (mController->GetButtonDown(SDL_CONTROLLER_BUTTON_LEFTSHOULDER))
			{
				mSeedBankIndex--;
				if (mSeedBankIndex < 0)
					mSeedBankIndex = mApp->mBoard->mSeedBank->mNumPackets - 1;
			}
			if (mController->GetButtonDown(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER))
			{
				mSeedBankIndex++;
				if (mSeedBankIndex >= mApp->mBoard->mSeedBank->mNumPackets)
					mSeedBankIndex = 0;
			}
		}
		else if (aIsSeedChoosing)
		{
			int aRows = mApp->mSeedChooserScreen->GetRows();
			bool aButtonDPadLeft = mController->GetButton(SDL_CONTROLLER_BUTTON_DPAD_LEFT);
			bool aButtonDPadRight = mController->GetButton(SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
			bool aButtonDPadUp = mController->GetButton(SDL_CONTROLLER_BUTTON_DPAD_UP);
			bool aButtonDPadDown = mController->GetButton(SDL_CONTROLLER_BUTTON_DPAD_DOWN);
			bool aIsAxisXActive = mController->IsAxisActive(SDL_CONTROLLER_AXIS_LEFTX);
			bool aIsAxisYActive = mController->IsAxisActive(SDL_CONTROLLER_AXIS_LEFTY);
			if ((mSeedChooserMoveMotion == -1 || mApp->mSeedChooserScreen->mSeedChooserAge >= mSeedChooserMoveMotion) && (aButtonDPadLeft || aButtonDPadRight || aButtonDPadUp || aButtonDPadDown || aIsAxisXActive || aIsAxisYActive))
			{
				float aAxisXValue = mController->GetAxisValue(SDL_CONTROLLER_AXIS_LEFTX);
				float aAxisYValue = mController->GetAxisValue(SDL_CONTROLLER_AXIS_LEFTY);
				if (aButtonDPadLeft || (aIsAxisXActive && aAxisXValue < 0))
				{
					if (!(mSeedChooserSeed % aRows == 0 && mSeedChooserSeed != SEED_IMITATER))
						mSeedChooserSeed = (SeedType)(mSeedChooserSeed - 1);
				}
				if (aButtonDPadRight || (aIsAxisXActive && aAxisXValue > 0))
				{
					int aIsLastRow = (mSeedChooserSeed + 1) % aRows == 0;
					if (aIsLastRow && mApp->SeedTypeAvailable(SEED_IMITATER))
						mSeedChooserSeed = SEED_IMITATER;
					else if (!(aIsLastRow && !mApp->SeedTypeAvailable(SEED_IMITATER)))
						mSeedChooserSeed = (SeedType)(mSeedChooserSeed + 1);
				}
				if (aButtonDPadUp || (aIsAxisYActive && aAxisYValue < 0))
				{
					if (!(mSeedChooserSeed == SEED_IMITATER || mSeedChooserSeed < aRows))
						mSeedChooserSeed = (SeedType)(mSeedChooserSeed - aRows);
				}
				if (aButtonDPadDown || (aIsAxisYActive && aAxisYValue > 0))
				{
					if (!(mSeedChooserSeed >= SEED_IMITATER - aRows))
						mSeedChooserSeed = (SeedType)(mSeedChooserSeed + aRows);
				}
				mSeedChooserSeed = (SeedType)ClampInt(mSeedChooserSeed, SEED_PEASHOOTER, SEED_IMITATER);
				mSeedChooserMoveMotion = mApp->mSeedChooserScreen->mSeedChooserAge + 9;
			}
			else if (!aButtonDPadLeft && !aButtonDPadRight && !aButtonDPadUp && !aButtonDPadDown && !aIsAxisXActive && !aIsAxisYActive)
				mSeedChooserMoveMotion = -1;

			bool aButtonA = mController->GetButton(SDL_CONTROLLER_BUTTON_A);
			bool aButtonB = mController->GetButton(SDL_CONTROLLER_BUTTON_B);
			if ((mSeedChooserButtonMotion == -1 || mApp->mSeedChooserScreen->mSeedChooserAge >= mSeedChooserButtonMotion) && (aButtonA || aButtonB))
			{
				if (aButtonA)
					mApp->mSeedChooserScreen->SelectSeedType(mSeedChooserSeed, mIndex);
				if (aButtonB)
				{
					int aHighestIndex = 0;
					SeedType aHighestSeedType = SEED_NONE;
					for (SeedType aSeedType = SEED_PEASHOOTER; aSeedType < NUM_SEEDS_IN_CHOOSER; aSeedType = (SeedType)(aSeedType + 1))
					{
						if (mApp->SeedTypeAvailable(aSeedType))
						{
							ChosenSeed& aChosenSeed = mApp->mSeedChooserScreen->mChosenSeeds[aSeedType];
							if (!aChosenSeed.mCrazyDavePicked && aHighestIndex <= aChosenSeed.mSeedIndexInBank && (aChosenSeed.mSeedState == SEED_IN_BANK || aChosenSeed.mSeedState == SEED_FLYING_TO_BANK))
							{
								aHighestIndex = aChosenSeed.mSeedIndexInBank;
								aHighestSeedType = aSeedType;
							}
						}
					}
					if (aHighestSeedType == SEED_NONE)
					{
						mApp->LeaveBoard(mIndex);
						return;
					}
					mApp->mSeedChooserScreen->SelectSeedType(aHighestSeedType, mIndex);
				}
				mSeedChooserButtonMotion = mApp->mSeedChooserScreen->mSeedChooserAge + 27;
			}
			else if (!aButtonA && !aButtonB)
				mSeedChooserButtonMotion = -1;
		}
	}
	else if (mSeedBankIndex != -1 || mSeedChooserSeed != SEED_NONE)
	{
		mSeedChooserPrevSeed = mSeedChooserSeed;
		mSeedChooserSeed = SEED_NONE;
		mSeedBankPrevIndex = mSeedBankIndex;
		mSeedBankIndex = -1;
		mArrowStartMotion = -1;
		mArrowEndMotion = -1;
		mSeedChooserMoveMotion = -1;
		mSeedChooserButtonMotion = -1;
	}
}