#include "StudentWorld.h"
#include <string>
#include "Actor.h"
#include "Compiler.h"
#include <iostream>
#include <sstream>
#include <iomanip>
using namespace std;

GameWorld* createStudentWorld(string assetDir)
{
	return new StudentWorld(assetDir);
}

// Students:  Add code to this file (if you wish), StudentWorld.h, Actor.h and Actor.cpp
int StudentWorld::init()
{
	vector<string> filenames = getFilenamesOfAntPrograms();
	for (int i = 0; i < filenames.size(); i++) {
		Compiler * c = new Compiler();
		string s;
		c->compile(filenames[i], s);
		cerr << s << endl;
		programs.push_back(c);
	}

	Field f;
	std::string fieldFile = getFieldFilename();

	std::string error;
	if (f.loadField(fieldFile, error) != Field::LoadResult::load_success)
	{
		setError(fieldFile + " " + error);
		return GWSTATUS_LEVEL_ERROR; // something bad happened!
	}

	Field::FieldItem item;
	for (int x = 0; x < VIEW_WIDTH; x++) {
		for (int y = 0; y < VIEW_HEIGHT; y++) {
			item = f.getContentsOf(x, y);
			Actor * a = nullptr;
			int colony = 0;
			switch (item) {
			case Field::FieldItem::food:
				a = new Food(this,x,y,6000); break;
			case Field::FieldItem::rock:
				a = new Pebble(this, x, y); break;
			case Field::FieldItem::grasshopper:
				a = new BabyGrasshopper(this, x, y); break;
			case Field::FieldItem::water:
				a = new WaterPool(this, x, y); break;
			case Field::FieldItem::poison:
				a = new Poison(this, x, y); break;
			case Field::FieldItem::anthill3: colony++;
			case Field::FieldItem::anthill2: colony++;
			case Field::FieldItem::anthill1: colony++;
			case Field::FieldItem::anthill0:
				if (programs.size() > colony)
					a = new AntHill(this, x, y, colony, programs[colony]);
				break;
			default: break;
			}
			
			if (a)
				arr[x][y].push_back(a);
		}
	}

	return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp()
{
	for (int x = 0; x < VIEW_WIDTH; x++) {
		for (int y = 0; y < VIEW_HEIGHT; y++) {
			list<Actor*>::iterator it = arr[x][y].begin();
			while (it != arr[x][y].end()) {
				delete *it;
				it++;
			}
			arr[x][y].clear();
		}
	}
	for (int i = 0; i < programs.size(); i++) {
		delete programs[i];
	}
	programs.clear();
}

int StudentWorld::move()
{
	updateTicks();


	//call doSomething for each actor and update position
	for (int x = 0; x < VIEW_WIDTH; x++) {
		for (int y = 0; y < VIEW_HEIGHT; y++) {
			list<Actor*>::iterator it = arr[x][y].begin();
			while (it != arr[x][y].end()) {
				(*it)->doSomething();

				Coordinate toC = make_pair((*it)->getX(), (*it)->getY());
				if (make_pair(x, y) != toC) {
					Actor * a = (*it);
					it = arr[x][y].erase(it);
					arr[toC.first][toC.second].push_back(a);
				}
				if (it != arr[x][y].end())
					it++;
			}
		}
	}

	//resolve all dead actors
	for (int x = 0; x < VIEW_WIDTH; x++) {
		for (int y = 0; y < VIEW_HEIGHT; y++) {
			list<Actor*>::iterator it = arr[x][y].begin();
			while (it != arr[x][y].end()) {
				if ((*it)->isDead()) {
					EnergyHolder* e = dynamic_cast<EnergyHolder*>(*it);
					if (e->becomesFoodUponDeath()) {
						e->addFood(100);
					}
					it = arr[x][y].erase(it);
					delete e;
				}
				if(it != arr[x][y].end())
					it++;
			}
		}
	}

	setDisplayText();

	if (getTicks() == 1000) {
		return GWSTATUS_NO_WINNER;
	}

	return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::setDisplayText() {
	string s = formatDisplay(getTicks(), getWinningAnt());
	setGameStatText(s);
}

string StudentWorld::formatDisplay(int ticks, int winner) {
	ostringstream oss;
	oss << "Ticks:";
	oss << setw(5) << ticks;
	oss << " -";
	vector<string> v = getFilenamesOfAntPrograms();
	for (int i = 0; i < v.size(); i++) {
		oss << "  ";
		oss << v[i].substr(0, v[i].size() - 4);
		if (i == winner)
			oss << "*";
		oss << ": ";
		oss.fill('0');
		oss << setw(2) << score[i];
	}
	return oss.str();
}

bool StudentWorld::canMoveTo(int x, int y) const {
	if (x < 0 || y < 0 || x >= VIEW_WIDTH || y >= VIEW_HEIGHT)
		return false;
	
	list<Actor*>::const_iterator it = arr[x][y].begin();
	while (it != arr[x][y].end()) {
		if ((*it)->blocksMovement())
			return false;
		it++;
	}

	return true;
}

void StudentWorld::addActor(Actor* a) {
	int x = a->getX();
	int y = a->getY();
	arr[x][y].push_back(a);
}

Actor* StudentWorld::getEdibleAt(int x, int y) const{
	if (x < 0 || y < 0 || x >= VIEW_WIDTH || y >= VIEW_HEIGHT)
		return nullptr;

	list<Actor*>::const_iterator it = arr[x][y].begin();
	while (it != arr[x][y].end()) {
		if ((*it)->isEdible())
			return *it;
		it++;
	}

	return nullptr;
}

Actor* StudentWorld::getPheromoneAt(int x, int y, int colony) const{
	if (x < 0 || y < 0 || x >= VIEW_WIDTH || y >= VIEW_HEIGHT)
		return nullptr;

	list<Actor*>::const_iterator it = arr[x][y].begin();
	while (it != arr[x][y].end()) {
		if ((*it)->isPheromone(colony))
			return *it;
		it++;
	}

	return nullptr;
}

bool StudentWorld::isEnemyAt(int x, int y, int colony) const{
	if (x < 0 || y < 0 || x >= VIEW_WIDTH || y >= VIEW_HEIGHT)
		return false;

	list<Actor*>::const_iterator it = arr[x][y].begin();
	while (it != arr[x][y].end()) {
		if ((*it)->isEnemy(colony))
			return true;
		it++;
	}

	return false;
}

bool StudentWorld::isDangerAt(int x, int y, int colony) const{
	if (x < 0 || y < 0 || x >= VIEW_WIDTH || y >= VIEW_HEIGHT)
		return false;

	list<Actor*>::const_iterator it = arr[x][y].begin();
	while (it != arr[x][y].end()) {
		if ((*it)->isDangerous(colony))
			return true;
		it++;
	}

	return false;
}

bool StudentWorld::isAntHillAt(int x, int y, int colony) const{
	if (x < 0 || y < 0 || x >= VIEW_WIDTH || y >= VIEW_HEIGHT)
		return false;

	list<Actor*>::const_iterator it = arr[x][y].begin();
	while (it != arr[x][y].end()) {
		if ((*it)->isMyHill(colony))
			return true;
		it++;
	}

	return false;
}

bool StudentWorld::biteEnemyAt(Actor* me, int colony, int biteDamage){
	vector<Actor*> v;
	list<Actor*>::iterator it = arr[me->getX()][me->getY()].begin();
	while (it != arr[me->getX()][me->getY()].end()) {
		if ((*it)->isEnemy(colony) && *it != me) {
			v.push_back(*it);
		}
		it++;
	}
	if (v.size() == 0)
		return false;
	int random = randInt(0, v.size() - 1);
	v[random]->getBitten(biteDamage);
	return true;
}

bool StudentWorld::poisonAllPoisonableAt(int x, int y){
	if (x < 0 || y < 0 || x >= VIEW_WIDTH || y >= VIEW_HEIGHT)
		return false;

	list<Actor*>::iterator it = arr[x][y].begin();
	while (it != arr[x][y].end()) {
		(*it)->getPoisoned();
		it++;
	}

	return true;
}

bool StudentWorld::stunAllStunnableAt(int x, int y){
	if (x < 0 || y < 0 || x >= VIEW_WIDTH || y >= VIEW_HEIGHT)
		return false;

	list<Actor*>::iterator it = arr[x][y].begin();
	while (it != arr[x][y].end()) {
		(*it)->getStunned();
		it++;
	}

	return true;
}