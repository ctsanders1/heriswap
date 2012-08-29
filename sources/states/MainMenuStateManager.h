/*
	This file is part of Heriswap.

	@author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
	@author Soupe au Caillou - Gautier Pelloux-Prayer

	Heriswap is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, version 3.

	Heriswap is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Heriswap.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#include "StateManager.h"
#include "modes/GameModeManager.h"
#include "AnimedActor.h"
#include "Game.h"

class MainMenuGameStateManager : public GameStateManager {
	public:
		MainMenuGameStateManager(LocalizeAPI* lAPI, SuccessAPI* sAPI) : localizeAPI(lAPI), successAPI(sAPI) { };
		void Setup();
		void Enter();
		GameState Update(float dt);
		void Exit();

		GameMode choosenGameMode;
		Entity eStart[3];
		Entity menufg, menubg;
		AnimatedActor* herisson;
	Entity modeTitleToReset;
	private:
		Entity bStart[3];
		Entity quitButton[3];

		LocalizeAPI* localizeAPI;
        SuccessAPI* successAPI;
};