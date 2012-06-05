#include "GameModeManager.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <fstream>

#include "base/PlacementHelper.h"

#include "systems/TextRenderingSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/System.h"
#include "systems/ScrollingSystem.h"
#include "systems/ButtonSystem.h"


#include "DepthLayer.h"
#include "Game.h"
#include "InGameUiHelper.h"
#include "AnimedEntity.h"
#include "TwitchSystem.h"
#include "GridSystem.h"

#include <algorithm>
#include <functional>

//FCRR : FPS Calculation Refresh Rate
#define FCRR 1.


static float initialHerissonPosition(Entity herisson) {
    return -PlacementHelper::ScreenWidth * 0.5 - TRANSFORM(herisson)->size.X * 0.;
}

static float finalHerissonPosition(Entity herisson) {
    return PlacementHelper::ScreenWidth * 0.5 + TRANSFORM(herisson)->size.X * 0.5;
}

GameModeManager::GameModeManager(Game* game, SuccessManager* sMgr) {
	uiHelper.game = game;
	successMgr = sMgr;
}


float GameModeManager::position(float t) {
	float p = 0;

	if (t<=pts[0].X) {
		p = pts[0].Y;
	} else {
		unsigned int i;
		for (i = 0; i<pts.size()-1; i++) {
			if (t>pts[i].X && t<pts[i+1].X) {
				p = MathUtil::Lerp(pts[i].Y, pts[i+1].Y, (t-pts[i].X)/(pts[i+1].X-pts[i].X));
				break;
			}
		}
		if (i == pts.size()-1) {
			p = pts[pts.size()-1].Y;
		}
	}
	return MathUtil::Lerp(initialHerissonPosition(herisson), finalHerissonPosition(herisson), p);
}

void GameModeManager::LoadHerissonTexture(int type) {
    loadHerissonTexture(type, c);
}

void GameModeManager::Setup() {
	herisson = theEntityManager.CreateEntity();
	ADD_COMPONENT(herisson, Transformation);
	ADD_COMPONENT(herisson, Rendering);
	TRANSFORM(herisson)->z = DL_Animal;
	TRANSFORM(herisson)->size = Vector2(PlacementHelper::GimpWidthToScreen(142),PlacementHelper::GimpHeightToScreen(116));
	TransformationSystem::setPosition(TRANSFORM(herisson), Vector2(0, PlacementHelper::GimpYToScreen(1058)), TransformationSystem::N);
	c = new AnimatedActor();
	c->actor.e = herisson;
	c->frames=0;
	c->actor.speed = 4.1;
	RENDERING(herisson)->hide = true;

	branch = theEntityManager.CreateEntity();
	ADD_COMPONENT(branch, Transformation);
	TRANSFORM(branch)->z = DL_Branch;
	TRANSFORM(branch)->size = Vector2(PlacementHelper::GimpWidthToScreen(800), PlacementHelper::GimpHeightToScreen(400.0));
	TransformationSystem::setPosition(TRANSFORM(branch), Vector2(0, PlacementHelper::GimpYToScreen(0)), TransformationSystem::N);
	ADD_COMPONENT(branch, Rendering);
	RENDERING(branch)->hide = true;
	RENDERING(branch)->texture = theRenderingSystem.loadTextureFile("branche");

	decor2nd = theEntityManager.CreateEntity();
	ADD_COMPONENT(decor2nd, Transformation);
	TRANSFORM(decor2nd)->z = DL_Decor2nd;
	TRANSFORM(decor2nd)->size = Vector2(PlacementHelper::GimpWidthToScreen(800), PlacementHelper::GimpWidthToScreen(470));
	TransformationSystem::setPosition(TRANSFORM(decor2nd), Vector2(0, PlacementHelper::GimpYToScreen(610)), TransformationSystem::N);
	ADD_COMPONENT(decor2nd, Scrolling);
	SCROLLING(decor2nd)->images.push_back("decor2nd_0");
	SCROLLING(decor2nd)->images.push_back("decor2nd_1");
	SCROLLING(decor2nd)->images.push_back("decor2nd_2");
	SCROLLING(decor2nd)->images.push_back("decor2nd_3");
	SCROLLING(decor2nd)->speed = Vector2(-.15, 0);
	SCROLLING(decor2nd)->displaySize = Vector2(TRANSFORM(decor2nd)->size.X * 1.01, TRANSFORM(decor2nd)->size.Y);
	SCROLLING(decor2nd)->hide = true;

	decor1er = theEntityManager.CreateEntity();
	ADD_COMPONENT(decor1er, Transformation);
	TRANSFORM(decor1er)->z = DL_Decor1er;
	TRANSFORM(decor1er)->size = Vector2(PlacementHelper::GimpWidthToScreen(800), PlacementHelper::GimpWidthToScreen(300));
	TransformationSystem::setPosition(TRANSFORM(decor1er), Vector2(0, PlacementHelper::GimpYToScreen(1280)), TransformationSystem::S);
	ADD_COMPONENT(decor1er, Scrolling);
	SCROLLING(decor1er)->images.push_back("decor1er_0");
	SCROLLING(decor1er)->images.push_back("decor1er_1");
	SCROLLING(decor1er)->images.push_back("decor1er_2");
	SCROLLING(decor1er)->images.push_back("decor1er_3");
	SCROLLING(decor1er)->speed = Vector2(-0.01, 0);
	SCROLLING(decor1er)->displaySize = Vector2(TRANSFORM(decor1er)->size.X * 1.01, TRANSFORM(decor1er)->size.Y);
	SCROLLING(decor1er)->hide = true;

	fillVec();

	uiHelper.build();
	
	#ifdef DEBUG
	_debug = false;
	for(int i=0; i<8; i++) {
		debugEntities[2*i] = theEntityManager.CreateEntity();
		ADD_COMPONENT(debugEntities[2*i], Rendering);
		ADD_COMPONENT(debugEntities[2*i], Transformation);
		RENDERING(debugEntities[2*i])->texture = theRenderingSystem.loadTextureFile(Game::cellTypeToTextureNameAndRotation(i, 0));
		TransformationSystem::setPosition(TRANSFORM(debugEntities[2*i]), Vector2(PlacementHelper::GimpXToScreen(0) + i * PlacementHelper::GimpWidthToScreen(80), PlacementHelper::GimpYToScreen(1280)), TransformationSystem::SW);
		TRANSFORM(debugEntities[2*i])->z = DL_DebugLayer;
		TRANSFORM(debugEntities[2*i])->size = Vector2(PlacementHelper::GimpWidthToScreen(80));
		
		debugEntities[2*i + 1] = theEntityManager.CreateEntity();
		ADD_COMPONENT(debugEntities[2*i + 1], TextRendering);
		TEXT_RENDERING(debugEntities[2*i + 1])->positioning = TextRenderingComponent::CENTER;
		ADD_COMPONENT(debugEntities[2*i + 1], Transformation);
		TRANSFORM(debugEntities[2*i + 1])->position = TRANSFORM(debugEntities[2*i])->position;
		TRANSFORM(debugEntities[2*i+1])->z = DL_DebugLayer+0.01;
		TEXT_RENDERING(debugEntities[2*i + 1])->fontName = "typo";
		TEXT_RENDERING(debugEntities[2*i + 1])->charHeight = PlacementHelper::GimpHeightToScreen(25);
	}
	#endif
}

void GameModeManager::Enter() {
	RENDERING(herisson)->hide = false;
	RENDERING(herisson)->color.a = 1;
	RENDERING(branch)->hide = false;
	SCROLLING(decor2nd)->hide = false;
	SCROLLING(decor1er)->hide = false;
    LoadHerissonTexture(bonus+1);

	uiHelper.show();
    theGridSystem.HideAll(false);
    TRANSFORM(herisson)->position.X = initialHerissonPosition(herisson);
    RENDERING(herisson)->texture = theRenderingSystem.loadTextureFile(c->anim[0]);
}

void GameModeManager::Exit() {
	RENDERING(herisson)->hide = true;
	RENDERING(branch)->hide = true;
	SCROLLING(decor2nd)->hide = true;
	SCROLLING(decor1er)->hide = true;

	uiHelper.hide();

	// delete leaves
	for (unsigned int az=0;az<branchLeaves.size();az++) {
		theEntityManager.DeleteEntity(branchLeaves[az].e);
	}
	branchLeaves.clear();
    theGridSystem.DeleteAll();
}

void GameModeManager::TogglePauseDisplay(bool paused) {
	theGridSystem.HideAll(paused);

    BUTTON(uiHelper.pauseButton)->enabled=!paused;;
    RENDERING(uiHelper.pauseButton)->hide = paused;
}

void GameModeManager::generateLeaves(int* nb, int type) {
	for (unsigned int az=0;az<branchLeaves.size();az++)
		theEntityManager.DeleteEntity(branchLeaves[az].e);

	branchLeaves.clear();
	fillVec();

    for (int j=0;j<type;j++) {
	    for (int i=0 ; i < (nb ? nb[j] : 6);i++) {
			Entity e = theEntityManager.CreateEntity();
			ADD_COMPONENT(e, Transformation);
			ADD_COMPONENT(e, Rendering);
            ADD_COMPONENT(e, Twitch);
			RENDERING(e)->texture = theRenderingSystem.loadTextureFile(Game::cellTypeToTextureNameAndRotation(j, 0));
			RENDERING(e)->hide = false;
			TRANSFORM(e)->size.X = TRANSFORM(e)->size.Y = Game::CellSize(8) * Game::CellContentScale();

			int rand = MathUtil::RandomInt(posBranch.size());
			TRANSFORM(e)->position = posBranch[rand].v;
			TRANSFORM(e)->rotation = posBranch[rand].rot;

			posBranch.erase(posBranch.begin()+rand);

			TRANSFORM(e)->z = MathUtil::Lerp(DL_LeafMin, DL_LeafMax, MathUtil::RandomFloat());
			BranchLeaf bl;
			bl.e = e;
			bl.type=j;
			branchLeaves.push_back(bl);
		}
	}
	//shuffle pour éviter que les mêmes couleurs soient à coté dans la liste :
	//ça sert en 100 tiles, pour que les feuilles supprimées soient d'abord ttes les
	//rouges, ensuites les jaunes etc..
	std::random_shuffle(branchLeaves.begin(), branchLeaves.end());
}

void GameModeManager::deleteLeaves(unsigned int type, int nb) {
	if (type == (unsigned int)~0b0) {
		while (branchLeaves.size()>0 && nb) {
			theEntityManager.DeleteEntity(branchLeaves[0].e);
			branchLeaves.erase(branchLeaves.begin());
			nb--;
		}
	} else {
		for (unsigned int i=0; nb>0 && i<branchLeaves.size(); i++) {
			if (type == branchLeaves[i].type) {
				theEntityManager.DeleteEntity(branchLeaves[i].e);
				branchLeaves.erase(branchLeaves.begin()+i);
				nb--;
				i--;
			}
		}
	}
}

void GameModeManager::fillVec() {
#if 0
	int t;
	std::ifstream file("position_feuilles.txt");
	int count = 0;
	std::string line;
	posBranch.clear(); // a la place de swapper ...
	while ( getline( file, line ) ){
		++count;
		if (count>t) {
			Vector2 v = Vector2::Zero;
			float r = 0.f;
			sscanf(line.c_str(), "%f %f %f", &v.X, &v.Y, &r);
			v.X =PlacementHelper::GimpXToScreen(v.X);
			v.Y =PlacementHelper::GimpYToScreen(v.Y);
			r =MathUtil::ToRadians(r);
			Render truc = {v,r};
			posBranch.push_back(truc);
			t++;
		}
	}
	while (count < t) {
		posBranch.pop_back();
		t--;
	}
#else
	posBranch.clear();
	#include "PositionFeuilles.h"
	for (int i=0; i<8*6; i++) {
		Vector2 v(PlacementHelper::GimpXToScreen(pos[3*i]), PlacementHelper::GimpYToScreen(pos[3*i+1]));
		Render truc = {v, MathUtil::ToRadians(pos[3*i+2])};
		posBranch.push_back(truc);
	}
#endif
}

void GameModeManager::updateHerisson(float dt, float obj, float herissonSpeed) {
	// default herisson behavior: move to
	TransformationComponent* tc = TRANSFORM(herisson);
	float newPos = tc->position.X;
	if (herissonSpeed == 0) {
		float targetPosX = position(obj);
    	float distance = targetPosX - tc->position.X;
		if (distance != 0.f) {
			herissonSpeed = (distance > 0) ? 1.0f : -1.0f;
			if (distance < 0) {
				newPos = MathUtil::Max(targetPosX, tc->position.X + herissonSpeed*dt);
			} else {
				newPos = MathUtil::Min(targetPosX, tc->position.X + herissonSpeed*dt);
			}
		}
	} else {
		newPos = tc->position.X + herissonSpeed * dt;
	}
	//set animation speed
	c->actor.speed = 15*(newPos - tc->position.X)/dt;
	if (c->actor.speed < 1.4) c->actor.speed = 2.5;
	if (c->actor.speed > 4.5) c->actor.speed = 4.5;

	tc->position.X = newPos;

	if (herissonSpeed > 0) {
		switchAnim(c);
	} else {
		RENDERING(herisson)->texture = theRenderingSystem.loadTextureFile(c->anim[1]);
	}
	uiHelper.update(dt);
}

int GameModeManager::saveInternalState(uint8_t** out) {
    int s = sizeof(time) + sizeof(limit) + sizeof(points) + sizeof(bonus) + 8 * sizeof(uint8_t);
    uint8_t* ptr = *out = new uint8_t[s];
    ptr = (uint8_t*) mempcpy(ptr, &time, sizeof(time));
    ptr = (uint8_t*) mempcpy(ptr, &limit, sizeof(limit));
    ptr = (uint8_t*) mempcpy(ptr, &points, sizeof(points));
    ptr = (uint8_t*) mempcpy(ptr, &bonus, sizeof(bonus));
    for (int i=1; i<=8; i++) {
        uint8_t count = 0;
        for (unsigned int j=0; j<branchLeaves.size(); j++) {
            if (branchLeaves[j].type == (unsigned int)i)
                count++;
        }
        ptr = (uint8_t*) mempcpy(ptr, &count, sizeof(count));
    }
    return s;
}

const uint8_t* GameModeManager::restoreInternalState(const uint8_t* in, int size) {
    int index = 0;
    memcpy(&time, &in[index], sizeof(time)); index += sizeof(time);
    memcpy(&limit, &in[index], sizeof(limit)); index += sizeof(limit);
    memcpy(&points, &in[index], sizeof(points)); index += sizeof(points);
    memcpy(&bonus, &in[index], sizeof(bonus)); index += sizeof(bonus);
    int nb[8];
    for (int i=0; i<8; i++) {
        uint8_t c;
        memcpy(&c, &in[index], sizeof(uint8_t)); index += sizeof(uint8_t);
        nb[i] = c;
    }
    generateLeaves(nb,8);
    /*
    //ici ça depend du mode...
    //mode tiles
    generateLeaves(nb,8);
	//mode normal
	generateLeaves(nb, theGridSystem.Types);
	*/
    return &in[index];
}

#ifdef DEBUG
void GameModeManager::toggleDebugDisplay() { 
	_debug = !_debug;
	for(int i=0; i<8; i++) {
		RENDERING(debugEntities[2*i])->hide = !_debug;
		TEXT_RENDERING(debugEntities[2*i+1])->hide = !_debug;
	}
}
#endif

int GameModeManager::countBranchLeavesOfType(int t) const {
	int count = 0;
	for (unsigned int i=0; i<branchLeaves.size(); i++) {
        count += (t == branchLeaves[i].type);
	}
	return count;       
}
