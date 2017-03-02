#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "GameConstants.h"
#include <string>
#include "Field.h"
#include <unordered_map>
#include <list>


class Compiler;
class Actor;
// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp

class StudentWorld : public GameWorld
{
public:
	StudentWorld(std::string assetDir)
	 : GameWorld(assetDir)
	{
	}

	virtual ~StudentWorld() {
		cleanUp();
	}

	virtual int init();

	virtual int move();

	virtual void cleanUp();

	// Can an insect move to x,y?
	bool canMoveTo(int x, int y) const;

	// Add an actor to the world
	void addActor(Actor* a);

	// If an item that can be picked up to be eaten is at x,y, return a
	// pointer to it; otherwise, return a null pointer.  (Edible items are
	// only ever going be food.)
	Actor* getEdibleAt(int x, int y) const;

	// If a pheromone of the indicated colony is at x,y, return a pointer
	// to it; otherwise, return a null pointer.
	Actor* getPheromoneAt(int x, int y, int colony) const;

	// Is an enemy of an ant of the indicated colony at x,y?
	bool isEnemyAt(int x, int y, int colony) const;

	// Is something dangerous to an ant of the indicated colony at x,y?
	bool isDangerAt(int x, int y, int colony) const;

	// Is the anthill of the indicated colony at x,y?
	bool isAntHillAt(int x, int y, int colony) const;

	// Bite an enemy of an ant of the indicated colony at me's location
	// (other than me; insects don't bite themselves).  Return true if an
	// enemy was bitten.
	bool biteEnemyAt(Actor* me, int colony, int biteDamage);

	// Poison all poisonable actors at x,y.
	bool poisonAllPoisonableAt(int x, int y);

	// Stun all stunnable actors at x,y.
	bool stunAllStunnableAt(int x, int y);

	// Record another ant birth for the indicated colony.
	void increaseScore(int colony) { 
		if(colony > -1 && colony < 4) 
			score[colony]++; 
	}

private:
	int getWinningAnt() {
		int winner = -1;
		int max = -1;
		for (int i = 0; i < 4; i++) {
			if (score[i] > max) {
				max = score[i];
				winner = i;
			}
			else if (score[i] == max)
				winner = -1;
		}
		return winner;
	}
	std::string formatDisplay(int ticks, int winner);
	void updateTicks() { ticks++; }
	int getTicks() const { return ticks; }
	void setDisplayText();
	typedef std::pair<int, int> Coordinate;
	std::list<Actor*> arr[VIEW_WIDTH][VIEW_HEIGHT]; // check x,y again
	std::vector<Compiler*> programs;
	int ticks;
	int score[4];
};

#endif // STUDENTWORLD_H_
