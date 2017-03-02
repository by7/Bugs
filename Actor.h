#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
#include "GameConstants.h"

class StudentWorld;
class Compiler;

// Students:  Add code to this file, Actor.cpp, StudentWorld.h, and StudentWorld.cpp

class Actor : public GraphObject
{
public:
	Actor(StudentWorld* world, int startX, int startY, Direction startDir, int imageID, int depth) 
		: GraphObject(imageID, startX, startY, startDir, depth)
	{
		this->world = world;
	}

	// Action to perform each tick.
	virtual void doSomething() = 0;

	// Is this actor dead?
	virtual bool isDead() const { return false; }

	// Does this actor block movement?
	virtual bool blocksMovement() const { return false; }

	// Cause this actor to be be bitten, suffering an amount of damage.
	virtual void getBitten(int amt) {}

	// Cause this actor to be be poisoned.
	virtual void getPoisoned() {}

	// Cause this actor to be be stunned.
	virtual void getStunned() {}

	// Can this actor be picked up to be eaten?
	virtual bool isEdible() const { return false; }

	// Is this actor detected by an ant as a pheromone?
	virtual bool isPheromone(int colony) const { return false; }

	// Is this actor an enemy of an ant of the indicated colony?
	virtual bool isEnemy(int colony) const { return false; }

	// Is this actor detected as dangerous by an ant of the indicated colony?
	virtual bool isDangerous(int colony) const { return false; }

	// Is this actor the anthill of the indicated colony?
	virtual bool isMyHill(int colony) const { return false; }

	// Get this actor's world.
	StudentWorld* getWorld() const { return world; };
private:
	StudentWorld * world;
};

class Pebble : public Actor
{
public:
	Pebble(StudentWorld* sw, int startX, int startY)
		: Actor(sw, startX, startY, Direction(1), IID_ROCK, 0)
	{}
	virtual void doSomething() {}
	virtual bool blocksMovement() const { return true; };
};

class EnergyHolder : public Actor
{
public:
	EnergyHolder(StudentWorld* sw, int startX, int startY, Direction startDir, int energy, int imageID, int depth)
		: Actor(sw, startX, startY, startDir, imageID, depth)
	{
		this->energy = energy;
	}
	virtual bool isDead() const { return getEnergy() <= 0; }

	// Get this actor's amount of energy (for a Pheromone, same as strength).
	int getEnergy() const { return energy; }

	// Adjust this actor's amount of energy upward or downward.
	void updateEnergy(int amt) { energy += amt; }

	// Add an amount of food to this actor's location.
	void addFood(int amt);

	// Have this actor pick up an amount of food.
	int pickupFood(int amt);

	// Have this actor pick up an amount of food and eat it.
	int pickupAndEatFood(int amt);

	int getHeldEnergy() { return held_energy; }

	void dropFood() {
		addFood(held_energy);
		held_energy = 0;
	}

	void eatHeldFood(int amt) {
		if (held_energy <= amt) {
			updateEnergy(held_energy);
			held_energy = 0;
		}
		else {
			updateEnergy(amt);
			held_energy -= amt;
		}
	}

	// Does this actor become food when it dies?
	virtual bool becomesFoodUponDeath() const { return false; }
private:
	int energy;
	int held_energy;
};

class Food : public EnergyHolder
{
public:
	Food(StudentWorld* sw, int startX, int startY, int energy)
		: EnergyHolder(sw, startX, startY, right, energy, IID_FOOD, 2)
	{}
	virtual void doSomething() {}
	virtual bool isEdible() const { return true; };
};

class AntHill : public EnergyHolder
{
public:
	AntHill(StudentWorld* sw, int startX, int startY, int colony, Compiler* program)
		:EnergyHolder(sw, startX, startY, right, 8999, IID_ANT_HILL, 2)
	{
		this->colony = colony;
		this->program = program;
	}
	virtual void doSomething();
	virtual bool isMyHill(int colony) const { return this->colony == colony; }
private:
	int colony;
	Compiler * program;
};

class Pheromone : public EnergyHolder
{
public:
	Pheromone(StudentWorld* sw, int startX, int startY, int colony)
		:EnergyHolder(sw, startX, startY, right, 256, IID_PHEROMONE_TYPE0+colony, 2)
	{}
	virtual void doSomething();
	virtual bool isPheromone(int colony) const { return this->colony == colony; }

	// Increase the strength (i.e., energy) of this pheromone.
	void increaseStrength() {
		if (getEnergy() < 512)
			updateEnergy(256);
		else
			updateEnergy(768 - getEnergy()); //update strength to max of 768
	}
private:
	int colony;
};

class TriggerableActor : public Actor
{
public:
	TriggerableActor(StudentWorld* sw, int x, int y, int imageID)
		:Actor(sw, x, y, right, imageID, 2)
	{}
	virtual bool isDangerous(int colony) const { return true; }
};

class WaterPool : public TriggerableActor
{
public:
	WaterPool(StudentWorld* sw, int x, int y)
		:TriggerableActor(sw, x, y, IID_WATER_POOL)
	{}
	virtual void doSomething();
};

class Poison : public TriggerableActor
{
public:
	Poison(StudentWorld* sw, int x, int y)
		:TriggerableActor(sw, x, y, IID_POISON)
	{}
	virtual void doSomething();
};

class Insect : public EnergyHolder
{
public:
	Insect(StudentWorld* world, int startX, int startY, int energy, int imageID)
		:EnergyHolder(world, startX, startY, Direction(randInt(1,4)), energy, imageID, 1)
	{}
	virtual void doSomething();
	virtual void getBitten(int amt){ updateEnergy(-amt); }
	virtual void getPoisoned() {
		updateEnergy(-150);
	}
	virtual void getStunned() { 
		if (!stunned) {
			increaseSleepTicks(2);
			stunned = true;
		}
	}
	void unStun() { stunned = false; }
	virtual bool isEnemy(int colony) { return true; }
	virtual bool becomesFoodUponDeath() const { return true; }
	virtual bool isDeadOrAsleep();

	// Set x,y to the coordinates of the spot one step in front of this insect.
	typedef std::pair<int, int> Coordinate;
	Coordinate getXYInFrontOfMe(int x, int y) {
		switch (getDirection()) {
			case up: (y)++; break; //check up and down direction
			case down: (y)--; break;
			case right: (x)++; break;
			case left: (x)--; break;
		}
		return std::make_pair(x, y);
	}

	// Move this insect one step forward if possible, and return true;
	// otherwise, return false without moving.
	virtual bool moveForwardIfPossible();

	// Increase the number of ticks this insect will sleep by the indicated amount.
	void increaseSleepTicks(int amt) { sleepTicks += amt; }

private:
	int sleepTicks;
	bool stunned;
};

class Ant : public Insect
{
public:
	Ant(StudentWorld* sw, int startX, int startY, int colony, Compiler* program, int imageID)
		:Insect(sw, startX, startY, 1500, IID_ANT_TYPE0+colony)
	{
		this->colony = colony;
		this->program = program;
	}
	virtual void doSomething();
	virtual void getBitten(int amt) {
		bitten = true;
		updateEnergy(-amt);
	}
	virtual bool isEnemy(int colony) const { return this->colony != colony; }
	//virtual bool moveForwardIfPossible() { return false; }
private:
	bool checkCondition(int i);
	void moveForward();
	void pickup();
	void emitPheromone();
	void rotateClockwise();
	void rotateCounterClockwise();
	void generateRandomNumber(std::string s);
	bool runCommand();
	bool isPheromoneInFront();
	bool isDangerInFront();

	int commands;
	int colony;
	Compiler* program;
	int line;
	bool bitten;
	bool blocked;
	int random;
};

class Grasshopper : public Insect
{
public:
	Grasshopper(StudentWorld* sw, int startX, int startY, int energy, int imageID)
		:Insect(sw, startX, startY, energy, imageID)
	{}
	virtual void doSomething();
private: 
	int distance;
};

class BabyGrasshopper : public Grasshopper
{
public:
	BabyGrasshopper(StudentWorld* sw, int startX, int startY)
		:Grasshopper(sw, startX, startY, 500, IID_BABY_GRASSHOPPER)
	{}
	virtual void doSomething();
	virtual bool isEnemy(int colony) const { return true; }
};

class AdultGrasshopper : public Grasshopper
{
public:
	AdultGrasshopper(StudentWorld* sw, int startX, int startY)
		:Grasshopper(sw, startX, startY, 1600, IID_ADULT_GRASSHOPPER)
	{}
	virtual void getBitten(int amt);
	virtual void getPoisoned() {}
	virtual void getStunned() {}
	virtual void doSomething();
};

#endif // ACTOR_H_
