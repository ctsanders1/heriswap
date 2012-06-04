#include "NormalModeManager.h"

#include <sstream>

#include <base/Vector2.h>
#include <base/MathUtil.h>
#include <base/PlacementHelper.h>
#include <base/MathUtil.h>

#include "systems/ScrollingSystem.h"
#include "systems/ContainerSystem.h"
#include "systems/ButtonSystem.h"
#include "systems/TextRenderingSystem.h"
#include "systems/ADSRSystem.h"
#include "systems/MusicSystem.h"

#include "CombinationMark.h"
#include "Game.h"
#include "GridSystem.h"

#define SKY_SPEED 2.3
#define DECOR2_SPEED 1.6
#define DECOR1_SPEED 1

NormalGameModeManager::NormalGameModeManager(Game* game, SuccessManager* SuccessMgr) : GameModeManager(game,SuccessMgr) {
	pts.push_back(Vector2(0,0));
	pts.push_back(Vector2(15,0.125));
	pts.push_back(Vector2(25,0.25));
	pts.push_back(Vector2(35,0.5));
	pts.push_back(Vector2(45,1));
}

NormalGameModeManager::~NormalGameModeManager() {
}

void NormalGameModeManager::Setup() {
	GameModeManager::Setup();
}

void NormalGameModeManager::Enter() {
	limit = 45;
	time = 0;
	points = 0;
	level = 1;
	bonus = MathUtil::RandomInt(theGridSystem.Types);
	for (int i=0;i<theGridSystem.Types;i++) remain[i]=3;
	nextHerissonSpeed = 1;
	levelMoveDuration = 0;

	generateLeaves(0, theGridSystem.Types);

	GameModeManager::Enter();
}

void NormalGameModeManager::Exit() {
	successMgr->sTakeYourTime(time);
	successMgr->s666Loser(level);

    MUSIC(stressTrack)->volume = 0;
    ADSR(stressTrack)->active = false;
    ADSR(stressTrack)->value = 0;

	GameModeManager::Exit();
}

void NormalGameModeManager::TogglePauseDisplay(bool paused) {
	GameModeManager::TogglePauseDisplay(paused);
}

void NormalGameModeManager::GameUpdate(float dt) {
	time += dt;
}

float NormalGameModeManager::GameProgressPercent() {
	return MathUtil::Min(1.0f, (float)time/limit);
}


void NormalGameModeManager::UiUpdate(float dt) {
	ADSR(stressTrack)->active = (time > 37.5);
    if (ADSR(stressTrack)->active) {
        ADSR(stressTrack)->attackValue = ADSR(stressTrack)->sustainValue = MathUtil::Min((time - 37.5) / (limit - 37.5), 1.0);
    }

	//Score
	{
	std::stringstream a;
	a.precision(0);
	a << std::fixed << points;
	TEXT_RENDERING(uiHelper.scoreProgress)->text = a.str();
	}

	//Level
	{
	std::stringstream a;
	a << level;
	TEXT_RENDERING(uiHelper.smallLevel)->text = a.str();
	}

	if (levelMoveDuration > 0) {
		updateHerisson(dt, time, nextHerissonSpeed);
		levelMoveDuration -= dt;
		if (levelMoveDuration <= 0) {
			// stop scrolling
			SCROLLING(decor1er)->speed.X = 0;
		}
	} else {
		updateHerisson(dt, time, 0);
	}
	
#ifdef DEBUG
	if (_debug) {
		for(int i=0; i<8; i++) {
			std::stringstream text;
			text << (int)remain[i] << "," << (int)(level+2) << "," <<  countBranchLeavesOfType(i);
			TEXT_RENDERING(debugEntities[2*i+1])->text = text.str();
			TEXT_RENDERING(debugEntities[2*i+1])->hide = false;
			TEXT_RENDERING(debugEntities[2*i+1])->color = Color(0.2, 0.2, 0.2);
		}
	}
#endif
}


int NormalGameModeManager::levelToLeaveToDelete(int nb, int maxRemain, int done) {
	// done is updated later, so done is the currently removed leaves
	int previousRemovalCount = (int) (6 * done)/maxRemain;
	// what should be removed at this step
	int totalTheoricallyRemoved = (int) (6 * (done + nb))/maxRemain;

	// so, we have to removed the difference
	return totalTheoricallyRemoved - previousRemovalCount;
}

static float timeGain(int nb, float time) {
	return MathUtil::Min(time, 2.f*nb/theGridSystem.GridSize);
}

void NormalGameModeManager::WillScore(int count, int type, std::vector<Entity>& out) {
    int nb = levelToLeaveToDelete(count, level+2, level+2 - remain[type]);
    for (unsigned int i=0; nb>0 && i<branchLeaves.size(); i++) {
        if (type== branchLeaves[i].type) {
            CombinationMark::markCellInCombination(branchLeaves[i].e);
            out.push_back(branchLeaves[i].e);
            nb--;
        }
    }

    // move background during delete/spawn sequence (+ fall ?)
    float deleteDuration = 0.3;
    float spawnDuration = 0.2;
    // herisson distance
    float currentPos = TRANSFORM(herisson)->position.X;
    float newPos = GameModeManager::position(time - timeGain(count, time));
    // update herisson and decor at the same time.
    levelMoveDuration = deleteDuration + spawnDuration;
    nextHerissonSpeed = (newPos - currentPos) / levelMoveDuration;

    SCROLLING(decor1er)->speed.X = nextHerissonSpeed;
    // SCROLLING(decor2nd)->speed.X = nextHerissonSpeed * DECOR2_SPEED;
    // SCROLLING(sky)->speed.X = nextHerissonSpeed * SKY_SPEED;

}

void NormalGameModeManager::ScoreCalc(int nb, unsigned int type) {
	if (type == bonus)
		points += 10*level*2*nb*nb*nb/6;
	else
		points += 10*level*nb*nb*nb/6;

	deleteLeaves(type, levelToLeaveToDelete(nb, level+2, remain[type]));
	remain[type] -= nb;
	time -= timeGain(nb, time);

	if (remain[type]<0)
		remain[type]=0;

	successMgr->sRainbow(type);

	successMgr->sBonusToExcess(type, bonus, nb);

	successMgr->sExterminaScore(points);

}

bool NormalGameModeManager::LevelUp() {
	int match = 1, i=0;
	while (match && i<theGridSystem.Types) {
		if (remain[i] != 0)	match=0;
		i++;
	}
	//si on a tous les objectifs
	if (match) {
		successMgr->sLevel1For1K(level, points);

		level++;

		successMgr->sLevel10(level);

		time -= MathUtil::Min(20*8.f/theGridSystem.GridSize,time);

		std::cout << "Level up to level " << level << std::endl;

		for (int i=0;i<theGridSystem.Types;i++)
			remain[i] = 2+level;

		//reput hedgehog on first animation position
		c->ind = 0;
		bonus = MathUtil::RandomInt(theGridSystem.Types);
		LoadHerissonTexture(bonus+1);
		RENDERING(herisson)->texture = theRenderingSystem.loadTextureFile(c->anim[1]);
		SCROLLING(decor1er)->speed.X = 0;
	}
	return match;
}

GameMode NormalGameModeManager::GetMode() {
	return Normal;
}

Entity NormalGameModeManager::getSmallLevelEntity() {
    return uiHelper.smallLevel;
}

int NormalGameModeManager::saveInternalState(uint8_t** out) {
    uint8_t* tmp;
    int parent = GameModeManager::saveInternalState(&tmp);
    int s = sizeof(level) + sizeof(remain);
    uint8_t* ptr = *out = new uint8_t[parent + s];
    ptr = (uint8_t*) mempcpy(ptr, tmp, parent);
    ptr = (uint8_t*) mempcpy(ptr, &level, sizeof(level));
    ptr = (uint8_t*) mempcpy(ptr, &remain[0], sizeof(remain));

    delete[] tmp;
    return (parent + s);
}

const uint8_t* NormalGameModeManager::restoreInternalState(const uint8_t* in, int size) {
    in = GameModeManager::restoreInternalState(in, size);
    memcpy(&level, in, sizeof(level)); in += sizeof(level);
    memcpy(&remain[0], in, sizeof(remain)); in += sizeof(remain);
    return in;
}
