#include "Bush.h"
#include "Board.h"
#include "../LawnApp.h"
#include "../Resources.h"
#include "../GameConstants.h"
#include "../Sexy.TodLib/TodFoley.h"
#include "../Sexy.TodLib/TodDebug.h"
#include "../Sexy.TodLib/Reanimator.h"

const ReanimationType cBushReanims[] = { ReanimationType::REANIM_BUSH3, ReanimationType::REANIM_BUSH5, ReanimationType::REANIM_BUSH4, ReanimationType::REANIM_BUSH3_NIGHT, ReanimationType::REANIM_BUSH5_NIGHT, ReanimationType::REANIM_BUSH4_NIGHT };

void Bush::BushInitialize(int theX, int theY, int theRow, bool theNight)
{
    int aId = theRow - 1;
    int aIndex = (aId + 3) % 3;
    if (theNight)
        aIndex += 3;
    mPosX = theX;
    mPosY = theY;
    mID = aId;
    mBushIndex = aIndex;
    mRenderOrder = Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_ZOMBIE, theRow, 0);
    Reanimation* aBodyReanim = mApp->AddReanimation(mPosX, mPosY, mRenderOrder, cBushReanims[mBushIndex]);
    mReanimID = mApp->ReanimationGetID(aBodyReanim);
    aBodyReanim->PlayReanim("base bush", REANIM_PLAY_ONCE_AND_HOLD, 0, 0.0001f);
}

void Bush::AnimateBush()
{
    Reanimation* aReanim = mApp->ReanimationTryToGet(mReanimID);
    if (aReanim)
        aReanim->PlayReanim("anim_rustle", REANIM_PLAY_ONCE_AND_HOLD, 10, RandRangeFloat(8.0f, 10.0f));
}

void Bush::Update()
{
    Reanimation* aReanim = mApp->ReanimationTryToGet(mReanimID);
    if (aReanim)
        aReanim->Update();
}

void Bush::Draw(Graphics* g) {
    Reanimation* aReanim = mApp->ReanimationTryToGet(mReanimID);
    if (aReanim)
        aReanim->Draw(g);
}