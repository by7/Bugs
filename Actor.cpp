#include "Actor.h"
#include "StudentWorld.h"
#include "Compiler.h"
#include <string>
#include <cmath>
using namespace std;

// Students:  Add code to this file (if you wish), Actor.h, StudentWorld.h, and StudentWorld.cpp

void EnergyHolder::addFood(int amt) {
	Food * f;
	f = dynamic_cast<Food*>(getWorld()->getEdibleAt(getX(), getY()));
	if (f)
		f->updateEnergy(amt);
	else {
		f = new Food(getWorld(), getX(), getY(), amt);
		getWorld()->addActor(f);
	}
}

int EnergyHolder::pickupFood(int amt) {
	Food * f = dynamic_cast<Food*>(getWorld()->getEdibleAt(getX(), getY()));
	if (f) {
		int available = f->getEnergy();
		if (available < amt) {
			f->updateEnergy(available);
			held_energy += available;
			return available;
		}
		f->updateEnergy(-amt);
		held_energy += amt;
		return amt;
	}
	return 0;
}

int EnergyHolder::pickupAndEatFood(int amt) {
	int i = pickupFood(amt);
	updateEnergy(held_energy);
	held_energy = 0;
	return i;
}

void AntHill::doSomething() //done
{
	//decrement energy by 1
	//if dead return

	//check for food on square and eat, return
	if (pickupAndEatFood(10000)>0)
		return;

	if (getEnergy() >= 2000) {
		Ant * a = new Ant(getWorld(), getX(), getY(), colony, program, IID_ANT_TYPE0 + colony);
		getWorld()->addActor(a);
		updateEnergy(-1500);
		getWorld()->increaseScore(colony);
	}
	//make a new ant
}

void Pheromone::doSomething()
{
	//decrement energy by 1
	updateEnergy(-1);
}

void WaterPool::doSomething() {
	getWorld()->stunAllStunnableAt(getX(), getY());
}

void Poison::doSomething() {
	getWorld()->poisonAllPoisonableAt(getX(), getY());
}

void Insect::doSomething()
{
	//do stuff
	updateEnergy(-1);
	if (isDead()) {
		return;
	}
	if (sleepTicks > 0) {
		sleepTicks--;
		return;
	}
	//stunned = false;
	//poisoned = false;
}

bool Insect::isDeadOrAsleep() {
	updateEnergy(-1);
	if (isDead()) {
		return true;
	}
	if (sleepTicks > 0) {
		sleepTicks--;
		return true;
	}
	return false;
}

bool Insect::moveForwardIfPossible() {
	pair<int,int> p = getXYInFrontOfMe(getX(), getY());
	if (getWorld()->canMoveTo(p.first, p.second)) {
		moveTo(p.first, p.second);
		return true;
	}
	return false;
}

void Ant::doSomething()
{
	if (isDeadOrAsleep())
		return;
	for (int i = 0; i < 10; i++) {
		if(!runCommand())
			return;
	}
}

bool Ant::runCommand() {
	Compiler::Command c;
	if (!program->getCommand(line, c)) {
		updateEnergy(-5000);
		return false;
	}
	switch (c.opcode) {
	case Compiler::Opcode::moveForward:
		moveForward();
		line++;
		return false;
	case Compiler::Opcode::eatFood:
		eatHeldFood(100);
		line++;
		return false;
	case Compiler::Opcode::dropFood:
		dropFood();
		line++;
		return false;
	case Compiler::Opcode::bite:
		getWorld()->biteEnemyAt(this, colony, 15);
		line++;
		return false;
	case Compiler::Opcode::pickupFood:
		pickup();
		line++;
		return false;
	case Compiler::Opcode::emitPheromone:
		emitPheromone();
		line++;
		return false;
	case Compiler::Opcode::faceRandomDirection:
		setDirection(Direction(randInt(1, 4)));
		line++;
		return false;
	case Compiler::Opcode::rotateClockwise:
		rotateClockwise();
		line++;
		return false;
	case Compiler::Opcode::rotateCounterClockwise:
		rotateCounterClockwise();
		line++;
		return false;
	case Compiler::Opcode::generateRandomNumber:
		generateRandomNumber(c.operand1);
		line++;
		return true;
	case Compiler::Opcode::goto_command:
		line = stoi(c.operand1);
		return true;
	case Compiler::Opcode::if_command:
		if (checkCondition(stoi(c.operand1))) {
			line = stoi(c.operand2);
			return true;
		}
		else {
			line++;
			return false;
		}
	}
}

void Ant::moveForward(){
	if (moveForwardIfPossible()) {
		blocked = false;
		bitten = false;
	}
	else blocked = true;
}

void Ant::pickup(){
	if (1800 - getHeldEnergy() < 400)
		pickupFood(1800 - getHeldEnergy());
	else
		pickupFood(400);
}
void Ant::emitPheromone(){
	Pheromone* p = dynamic_cast<Pheromone*>(getWorld()->getPheromoneAt(getX(), getY(), colony));
	if (p) {
		p->increaseStrength();
	}
	else {
		getWorld()->addActor(new Pheromone(getWorld(), getX(), getY(), colony));
	}
}
void Ant::rotateClockwise(){
	switch (getDirection()) {
		case up:setDirection(right); break;
		case right:setDirection(down); break;
		case down:setDirection(left); break;
		case left:setDirection(up); break;
		default: break;
	}
}
void Ant::rotateCounterClockwise(){
	switch (getDirection()) {
		case up:setDirection(left); break;
		case right:setDirection(up); break;
		case down:setDirection(right); break;
		case left:setDirection(down); break;
		default: break;
	}
}
void Ant::generateRandomNumber(string s){
	int i = stoi(s);
	if (i == 0)
		random = 0;
	else
		random = randInt(0, i - 1);
}

bool Ant::checkCondition(int i) {

	switch (i) {
		case Compiler::Condition::last_random_number_was_zero: 
			return random == 0; 
		case Compiler::Condition::i_am_carrying_food: 
			return getHeldEnergy() > 0;
		case Compiler::Condition::i_am_hungry: 
			return getEnergy() <= 25;
		case Compiler::Condition::i_am_standing_with_an_enemy: 
			return getWorld()->isEnemyAt(getX(), getY(), colony);
		case Compiler::Condition::i_am_standing_on_food: 
			return getWorld()->getEdibleAt(getX(), getY());
		case Compiler::Condition::i_am_standing_on_my_anthill: 
			return getWorld()->isAntHillAt(getX(), getY(),colony);
		case Compiler::Condition::i_smell_pheromone_in_front_of_me: 
			return isPheromoneInFront();
		case Compiler::Condition::i_smell_danger_in_front_of_me:
			return isDangerInFront();
		case Compiler::Condition::i_was_bit: 
			return bitten;
		case Compiler::Condition::i_was_blocked_from_moving: 
			return blocked;
		default: break;
	}
	return false;
}

bool Ant::isPheromoneInFront() {
	pair<int, int> p = getXYInFrontOfMe(getX(), getY());
	return (getWorld()->getPheromoneAt(p.first, p.second, colony));
}

bool Ant::isDangerInFront() {
	pair<int, int> p = getXYInFrontOfMe(getX(), getY());
	return (getWorld()->isDangerAt(p.first, p.second, colony));
}

void Grasshopper::doSomething()
{
	//try to eat food
	if (pickupAndEatFood(200) > 0 && randInt(0, 1) == 0)
	{

	}
	//change direction and set new random distance
	else if (distance == 0) {
		setDirection(Direction(randInt(1, 4)));
		distance = randInt(2, 10);
	}

	else if (moveForwardIfPossible()) {
		unStun();
		distance--;
	}
	else
		distance = 0;

	increaseSleepTicks(2);
}

void BabyGrasshopper::doSomething() {
	if (isDeadOrAsleep())
		return;
	if (getEnergy() >= 1600) {
		AdultGrasshopper* a = new AdultGrasshopper(getWorld(), getX(), getY());
		getWorld()->addActor(a);
		updateEnergy(-5000); //set to dead
		return;
	}
	Grasshopper::doSomething();
}

void AdultGrasshopper::getBitten(int amt) {
	updateEnergy(-amt);
	if (!isDead() && randInt(1, 2) == 1) {
		getWorld()->biteEnemyAt(this, -1, 50);
	}
}

void AdultGrasshopper::doSomething() {
	if (isDeadOrAsleep())
		return;

	if (getWorld()->isEnemyAt(getX(), getY(), -1) && randInt(1, 3) == 1) {
		getWorld()->biteEnemyAt(this, -1, 50);
	}
	else if (randInt(1, 10) == 1) {
		int r, ang, x = 0, y = 0;
		do {
			r = randInt(1, 10);
			ang = randInt(1, 360);
			x = getX() + r*sin(ang*3.1415 / 180);
			y = getY() + r*sin(ang*3.1415 / 180);
		} while (!getWorld()->canMoveTo(x, y));
		moveTo(x, y);
	}
	else {
		Grasshopper::doSomething();
	}

	increaseSleepTicks(2);
}