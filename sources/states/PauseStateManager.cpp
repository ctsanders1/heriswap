#include "PauseStateManager.h"

#include "Game.h"

PauseStateManager::PauseStateManager() {

}

void PauseStateManager::Setup() {		
	eRestart = theTextRenderingSystem.CreateLocalEntity(10);	

	TRANSFORM(eRestart)->position = Vector2(3.5, 0);
		

	TEXT_RENDERING(eRestart)->text = "Reprendre";


	TEXT_RENDERING(eRestart)->hide = true;

	ADD_COMPONENT(eRestart, Button);
	ADD_COMPONENT(eRestart, Rendering);
	TRANSFORM(eRestart)->z = 20;

}

void PauseStateManager::Enter() {
	
	std::cout << __PRETTY_FUNCTION__ << std::endl;
	TEXT_RENDERING(eRestart)->hide = false;
}

GameState PauseStateManager::Update(float dt) {
	if (BUTTON(eRestart)->clicked) {
		return ScoreBoard;
	}
	return Pause;
}

void PauseStateManager::Exit() {
	std::cout << __PRETTY_FUNCTION__ << std::endl;
	TEXT_RENDERING(eRestart)->hide = true;
	BUTTON(eRestart)->clicked = false;

}


