#pragma once

#include <algorithm>


class Pole;

class Gate {
public:

	Gate(Pole* photodiodepole, Pole* ledpole, std::vector<Gate*>* pos);
	~Gate();

	Pole* getPartnerPole(Pole* pole) { return (pole == _LEDPole) ? _PhotodiodePole : _LEDPole; }
	Pole* getPartnerPole(const Pole* pole) { return (pole == _LEDPole) ? _PhotodiodePole : _LEDPole; }

	int getGatePosition() { return _GatePosition; }
	void setGatePosition(int newPos) { _Gates->push_back(_Gates[0][newPos]); _Gates[0][newPos]->_GatePosition = _Gates->size() - 1; _GatePosition = newPos; }

protected:
	Pole* _LEDPole;
	Pole* _PhotodiodePole;
	int _GatePosition;
	std::vector<Gate*>* _Gates;
}; 