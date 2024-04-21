#include "pole.h"
#include "gate.h"

Gate::Gate(Pole* photodiodepole, Pole* ledpole, std::vector<Gate*>* pos) : _PhotodiodePole(photodiodepole), _LEDPole(ledpole), _Gates(pos), _GatePosition(pos->size()) { _PhotodiodePole->setGate(this); _LEDPole->setGate(this);};


Gate::~Gate() {

}
