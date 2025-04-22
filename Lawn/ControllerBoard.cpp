#include "ControllerBoard.h"
#include "System/ControllerManager.h"
#include "../Resources.h"
#include "../../LawnApp.h"
#include "../../Lawn/Board.h"
#include "Cutscene.h"
#include "../../Lawn/Widget/SeedChooserScreen.h"
#include "../../Lawn/SeedPacket.h"
#include "../../Lawn/Widget/GameButton.h"
#include "../../Lawn/CursorObject.h"
#include "../../Lawn/ToolTipWidget.h"

void ControllerBoard::ControllerBoardInitialize(int theIndex)
{
	mIndex = theIndex;
	UpdateColor();
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

void ControllerBoard::Update()
{
	Controller* aController = mApp->mControllerManager->GetController(mIndex);
	if (aController != nullptr)
	{
		bool aIsSeedChoosing = mBoard->mCutScene->mSeedChoosing;
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

		if (!mApp->mHasFocus)
			return;

		if (aController->GetButtonDown(SDL_CONTROLLER_BUTTON_START))
		{
			if (!aIsSeedChoosing)
			{
				if (mApp->GetDialogCount() == 0)
				{
					mBoard->UpdateCursor();
					mBoard->ClearCursor();
					mApp->PlaySample(SOUND_PAUSE);
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
				if (!mApp->mSeedChooserScreen->mStartButton->mDisabled)
					mApp->mSeedChooserScreen->ButtonDepress(mApp->mSeedChooserScreen->mStartButton->mId);
			}
		}

		if (mBoard->mPaused)
			return;

		if (mApp->mGameScene == GameScenes::SCENE_PLAYING)
		{
			if (aController->GetButtonDown(SDL_CONTROLLER_BUTTON_LEFTSHOULDER))
			{
				mSeedBankIndex--;
				if (mSeedBankIndex < 0)
					mSeedBankIndex = mBoard->mSeedBank->mNumPackets - 1;
			}
			if (aController->GetButtonDown(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER))
			{
				mSeedBankIndex++;
				if (mSeedBankIndex >= mBoard->mSeedBank->mNumPackets)
					mSeedBankIndex = 0;
			}
		}
		else if (aIsSeedChoosing)
		{
			int aRows = mApp->mSeedChooserScreen->GetRows();
			bool aButtonDPadLeft = aController->GetButton(SDL_CONTROLLER_BUTTON_DPAD_LEFT);
			bool aButtonDPadRight = aController->GetButton(SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
			bool aButtonDPadUp = aController->GetButton(SDL_CONTROLLER_BUTTON_DPAD_UP);
			bool aButtonDPadDown = aController->GetButton(SDL_CONTROLLER_BUTTON_DPAD_DOWN);
			bool aIsAxisXActive = aController->IsAxisActive(SDL_CONTROLLER_AXIS_LEFTX);
			bool aIsAxisYActive = aController->IsAxisActive(SDL_CONTROLLER_AXIS_LEFTY);
			if ((mSeedChooserMoveMotion == -1 || mApp->mSeedChooserScreen->mSeedChooserAge >= mSeedChooserMoveMotion) && (aButtonDPadLeft || aButtonDPadRight || aButtonDPadUp || aButtonDPadDown || aIsAxisXActive || aIsAxisYActive))
			{
				float aAxisXValue = aController->GetAxisValue(SDL_CONTROLLER_AXIS_LEFTX);
				float aAxisYValue = aController->GetAxisValue(SDL_CONTROLLER_AXIS_LEFTY);
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

			bool aButtonA = aController->GetButton(SDL_CONTROLLER_BUTTON_A);
			bool aButtonB = aController->GetButton(SDL_CONTROLLER_BUTTON_B);
			if ((mSeedChooserButtonMotion == -1 || mApp->mSeedChooserScreen->mSeedChooserAge >= mSeedChooserButtonMotion) && (aButtonA || aButtonB))
			{
				if (aButtonA)
					mApp->mSeedChooserScreen->SelectSeedType(mSeedChooserSeed);
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
					mApp->mSeedChooserScreen->SelectSeedType(aHighestSeedType);
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

void ControllerBoard::UpdateColor()
{
	switch (mIndex)
	{
	case 0:
		mColor = Color(255, 242, 0);
		break;
	case 1:
		mColor = Color(71, 198, 255);
		break;
	case 2:
		mColor = Color(255, 71, 87);
		break;
	case 3:
		mColor = Color(153, 102, 255);
		break;
	case 4:
		mColor = Color(0, 255, 128);
		break;
	case 5:
		mColor = Color(255, 128, 0);
		break;
	case 6:
		mColor = Color(255, 0, 255);
		break;
	case 7:
		mColor = Color(128, 255, 255);
		break;
	default:
		mColor = Color(Rand(255), Rand(255), Rand(255));
		break;
	}
}