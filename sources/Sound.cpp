#include "base/EntityManager.h"

#include "systems/SoundSystem.h"

#include "Sound.h"
#include <sstream>

std::vector<std::string> newMusics() {
	static std::vector<char> ppal;
	if (ppal.size()==0) {
		ppal.push_back('A');
		ppal.push_back('B');
	}

	int count = MathUtil::RandomInt(4) + 1;
	std::vector<char> l; // all songs id
	std::vector<std::string> res;

	char c = ppal[MathUtil::RandomInt(ppal.size())]; // letter from main music
	if (count==1) c='A';

	std::stringstream s;
	s<<"audio/"<<c<<".ogg";
	l.push_back(c);
	res.push_back(s.str());

	for (int i=1; i<count; i++) {
		do {
			c = MathUtil::RandomInt(4)+'A';
		} while (std::find(l.begin(), l.end(), c) != l.end());
		l.push_back(c);
		std::stringstream s;
		s << "audio/" << c << ".ogg";
		res.push_back(s.str());
	}
	std::cout <<"starting " << count <<" musics : ";
	for (int i=0; i<res.size(); i++) std::cout << res[i] <<", ";
	std::cout<<std::endl;
	return res;
}

bool updateMusic(Canal* canal, Canal* canalStress1, Canal* canalStress2, float percentDone, float dt) {
	bool end, endv[4];
	for (int i=0; i<4; i++) endv[i] = canal[i].update(dt);
	end=endv[0] && endv[1] && endv[2] && endv[3];
	if (end) {
		std::vector<std::string> nouv = newMusics();
		for (int i=0; i<4; i++) {
			if (!nouv.empty()) {
				canal[i].name=nouv[0];
				nouv.erase(nouv.begin());
			} else {
				canal[i].name="";
			}
		}
	}
	float pos = 0;
	int count = 0;
	for (int i=0; i<4; i++) {
		if (SOUND(canal[i].sounds[canal[i].indice])->sound != InvalidSoundRef) {
			pos += SOUND(canal[i].sounds[canal[i].indice])->position;
			count++;
		}
	}

	if (percentDone > 25./45 && count) {
		pos /= count;
		if (SOUND(canalStress1->sounds[0])->sound==InvalidSoundRef && SOUND(canalStress1->sounds[1])->sound==InvalidSoundRef) {
			LOGI("starting clochettes n° one!");
			std::cout << pos << " != " << SOUND(canal[0].sounds[canal[0].indice])->position << std::endl;
			SOUND(canalStress1->sounds[canalStress1->indice])->position= pos + 2*dt / 18.;
			SOUND(canalStress1->sounds[canalStress1->indice])->masterTrack = SOUND(canal[0].sounds[canal[0].indice]);
		}
		canalStress1->update(dt);
	}
	if (percentDone > 35./45 && count) {
		if (SOUND(canalStress2->sounds[0])->sound==InvalidSoundRef && SOUND(canalStress2->sounds[1])->sound==InvalidSoundRef) {
			LOGI("starting clochettes n° two!");
			std::cout << pos << " != " << SOUND(canal[0].sounds[canal[0].indice])->position << std::endl;
			SOUND(canalStress2->sounds[canalStress2->indice])->position=pos + 2*dt / 18.;
			SOUND(canalStress2->sounds[canalStress2->indice])->masterTrack = SOUND(canal[0].sounds[canal[0].indice]);
		}
		canalStress2->update(dt);
	}
}
