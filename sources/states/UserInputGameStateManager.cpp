#include "UserInputGameStateManager.h"
#include "CombinationMark.h"

static void activateADSR(Entity e, float a, float s);
static void diffToGridCoords(const Vector2& c, int* i, int* j);

UserInputGameStateManager::UserInputGameStateManager(){
}

UserInputGameStateManager::~UserInputGameStateManager() {
	theEntityManager.DeleteEntity(eSwapper);
}

void UserInputGameStateManager::Setup() {
	// preload sound effect
	theSoundSystem.loadSoundFile("audio/son_monte.ogg", false);
	theSoundSystem.loadSoundFile("audio/son_descend.ogg", false);

	eSwapper = theEntityManager.CreateEntity();
	ADD_COMPONENT(eSwapper, ADSR);
	ADSR(eSwapper)->idleValue = 0;
	ADSR(eSwapper)->attackValue = 1.0;
	ADSR(eSwapper)->attackTiming = 0.1;
	ADSR(eSwapper)->decayTiming = 0;
	ADSR(eSwapper)->sustainValue = 1.0;
	ADSR(eSwapper)->releaseTiming = 0.1;

	ADD_COMPONENT(eSwapper, Sound);
	SOUND(eSwapper)->type = SoundComponent::EFFECT;
	originI = originJ = -1;
}

void UserInputGameStateManager::Enter() {
	LOGI("%s", __PRETTY_FUNCTION__);
	dragged = 0;
	ADSR(eSwapper)->active = false;
	ADSR(eSwapper)->activationTime = 0;
	originI = originJ = -1;
}

GameState UserInputGameStateManager::Update(float dt) {
	// drag/drop of cell
	if (!theTouchInputManager.wasTouched() &&
		theTouchInputManager.isTouched()) {
		// don't start new drag while the previous one isn't finished
		if (!dragged) {
			// start drag: find nearest cell
			const Vector2& pos = theTouchInputManager.getTouchLastPosition();
			int i, j;
			for( i=0; i<theGridSystem.GridSize && !dragged; i++) {
				for(j=0; j<theGridSystem.GridSize; j++) {
					Entity e = theGridSystem.GetOnPos(i,j);
						if(e && ButtonSystem::inside(
						pos,
						TRANSFORM(e)->worldPosition,
						TRANSFORM(e)->size)) {
						dragged = e;
						break;
					}
				}
			}
			if (dragged) {
				i--;
				originI = i;
				originJ = j;

				activateADSR(dragged, 1.4, 1.2);

				// active neighboors
				activateADSR(theGridSystem.GetOnPos(i+1,j), 1.2, 1.1);
				activateADSR(theGridSystem.GetOnPos(i,j+1), 1.2, 1.1);
				activateADSR(theGridSystem.GetOnPos(i-1,j), 1.2, 1.1);
				activateADSR(theGridSystem.GetOnPos(i,j-1), 1.2, 1.1);
			}
		}
	} else if (theTouchInputManager.wasTouched() && dragged && ADSR(dragged)->active) {
		if (theTouchInputManager.isTouched()) {
			// continue drag
			Vector2 diff = theTouchInputManager.getTouchLastPosition()
				- Game::GridCoordsToPosition(originI, originJ);

			if (diff.Length() > 1) {
				int i,j;
				diffToGridCoords(diff, &i, &j);

				if (theGridSystem.IsValidGridPosition(originI + i, originJ + j)) {
					if ((swapI == i && swapJ == j) ||
						(swapI == 0 && swapJ == 0)) {
						ADSR(eSwapper)->active = true;
						swapI = i;
						swapJ = j;

						Entity a = dragged, e = theGridSystem.GetOnPos(originI+swapI, originJ+swapJ);
						if (e && a) {
							GRID(a)->i = originI + swapI;
							GRID(a)->j = originJ + swapJ;
							GRID(e)->i = originI;
							GRID(e)->j = originJ;
						}
						std::vector<Combinais> combinaisons = theGridSystem.LookForCombination(false,false);
						if (e && a) {
							GRID(a)->i = originI;
							GRID(a)->j = originJ;
							GRID(e)->i = originI+swapI;
							GRID(e)->j =  originJ+swapJ;
						}
						if (!combinaisons.empty())
						{
							for ( std::vector<Combinais>::reverse_iterator it = combinaisons.rbegin(); it != combinaisons.rend(); ++it ) {
								for ( std::vector<Vector2>::reverse_iterator itV = (it->points).rbegin(); itV != (it->points).rend(); ++itV )
								{
                                    Entity cell;
                                    if (itV->X == originI && itV->Y == originJ) {
                                        cell = e;
                                    } else if (itV->X == (originI + swapI) && itV->Y == (originJ + swapJ)) {
                                        cell = a;
                                    } else {
                                        cell = theGridSystem.GetOnPos(itV->X, itV->Y);
                                    }
                                    if (cell) {
                                        inCombinationCells.push_back(cell);
                                        CombinationMark::markCellInCombination(cell);
                                    }
								}
							}
                        }
					} else {
                        for (int k=0; k<inCombinationCells.size(); k++) {
                            CombinationMark::clearCellInCombination(inCombinationCells[k]);
                        }
                        inCombinationCells.clear();
						if (ADSR(eSwapper)->activationTime > 0) {
							ADSR(eSwapper)->active = false;
						} else {
							ADSR(eSwapper)->active = true;
							swapI = i;
							swapJ = j;
						}
					}
				} else {
					ADSR(eSwapper)->active = false;
                    for (int k=0; k<inCombinationCells.size(); k++) {
                        CombinationMark::clearCellInCombination(inCombinationCells[k]);
                    }
                    inCombinationCells.clear();
				}
			} else {
				ADSR(eSwapper)->active = false;
                for (int k=0; k<inCombinationCells.size(); k++) {
                    CombinationMark::clearCellInCombination(inCombinationCells[k]);
                }
                inCombinationCells.clear();
			}
		} else {
			LOGI("release");

			// release drag
			ADSR(theGridSystem.GetOnPos(originI,originJ))->active = false;
			if (theGridSystem.IsValidGridPosition(originI+1, originJ))
				ADSR(theGridSystem.GetOnPos(originI+1,originJ))->active = false;
			if (theGridSystem.IsValidGridPosition(originI, originJ+1))
				ADSR(theGridSystem.GetOnPos(originI,originJ+1))->active = false;
			if (theGridSystem.IsValidGridPosition(originI-1,originJ))
				ADSR(theGridSystem.GetOnPos(originI-1,originJ))->active = false;
			if (theGridSystem.IsValidGridPosition(originI,originJ-1))
				ADSR(theGridSystem.GetOnPos(originI,originJ-1))->active = false;
			ADSR(eSwapper)->active = false;

			/* must swap ? */
			if (ADSR(eSwapper)->value >= 0.99) {
				Entity e2 = theGridSystem.GetOnPos(originI+ swapI,originJ+ swapJ);
				GRID(e2)->i = originI;
				GRID(e2)->j = originJ;
				GRID(e2)->checkedH = false;
				GRID(e2)->checkedV = false;

				Entity e1 = dragged ;
				GRID(e1)->i = originI + swapI;
				GRID(e1)->j = originJ + swapJ;
				GRID(e1)->checkedH = false;
				GRID(e1)->checkedV = false;

				std::vector<Combinais> combinaisons = theGridSystem.LookForCombination(false,true);
				if (
			#ifndef ANDROID
				glfwGetMouseButton(GLFW_MOUSE_BUTTON_2) != GLFW_PRESS &&
			#endif
				combinaisons.empty()) {
					// revert swap
					GRID(e1)->i = originI;
					GRID(e1)->j = originJ;
					GRID(e2)->i = originI + swapI;
					GRID(e2)->j = originJ + swapJ;

					SOUND(eSwapper)->sound = theSoundSystem.loadSoundFile("audio/son_descend.ogg", false);
					return UserInput;
				} else {
					// validate position
					TRANSFORM(e1)->position = Game::GridCoordsToPosition(GRID(e1)->i, GRID(e1)->j);
					TRANSFORM(e2)->position = Game::GridCoordsToPosition(GRID(e2)->i, GRID(e2)->j);

					originI = originJ = -1;
					SOUND(eSwapper)->sound = theSoundSystem.loadSoundFile("audio/son_monte.ogg", false);
					return Delete;
				}
			}
			for (int k=0; k<inCombinationCells.size(); k++) {
            	CombinationMark::clearCellInCombination(inCombinationCells[k]);
            }
            inCombinationCells.clear();
		}
	} else {
		ADSR(eSwapper)->active = false;
		if (dragged) ADSR(dragged)->active = false;
	}

	if (dragged) {
		// reset 'dragged' cell only if both anim are ended
		if (!ADSR(dragged)->active && !ADSR(eSwapper)->active) {
			if (ADSR(eSwapper)->activationTime <= 0 && ADSR(dragged)->activationTime <= 0) {
				dragged = 0;
			}
		}
	}
	return UserInput;
}

void UserInputGameStateManager::BackgroundUpdate(float dt) {
	for(int i=0; i<theGridSystem.GridSize; i++) {
		for(int j=0; j<theGridSystem.GridSize; j++) {
			Entity e = theGridSystem.GetOnPos(i,j);
			if (e) {
				TRANSFORM(e)->size = ADSR(e)->value;
			}
		}
	}

	if (ADSR(eSwapper)->activationTime >= 0 && originI >= 0 && originJ >= 0) {

		Vector2 pos1 = Game::GridCoordsToPosition(originI, originJ);
		Vector2 pos2 = Game::GridCoordsToPosition(originI + swapI, originJ + swapJ);

		Vector2 interp1 = MathUtil::Lerp(pos1, pos2, ADSR(eSwapper)->value);
		Vector2 interp2 = MathUtil::Lerp(pos2, pos1, ADSR(eSwapper)->value);

		Entity e1 = theGridSystem.GetOnPos(originI,originJ);
		Entity e2 = theGridSystem.GetOnPos(originI + swapI,originJ + swapJ);

		if (e1)
			TRANSFORM(e1)->position = interp1;
		if (e2)
			TRANSFORM(e2)->position = interp2;
	}
}

void UserInputGameStateManager::Exit() {
	LOGI("%s", __PRETTY_FUNCTION__);
    inCombinationCells.clear();
}

static void activateADSR(Entity e, float a, float s) {
	if (!e)
		return;
	float size = TRANSFORM(e)->size.X;
	ADSRComponent* ac = ADSR(e);
	ac->idleValue = size;
	ac->attackValue = size * a;
	ac->attackTiming = 0.3;
	ac->decayTiming = 0.2;
	ac->sustainValue = size * s;
	ac->releaseTiming = 0.2;
	ac->active = true;
}

void diffToGridCoords(const Vector2& c, int* i, int* j) {
	*i = *j = 0;
	if (MathUtil::Abs(c.X) > MathUtil::Abs(c.Y)) {
		*i = (c.X < 0) ? -1 : 1;
	} else {
		*j = (c.Y < 0) ? -1 : 1;
	}
}
