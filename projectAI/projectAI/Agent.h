// Agent.h
//
// Fall 2020 HW5
//
// Author: Larry Holder

#ifndef AGENT_H
#define AGENT_H

#include "Action.h"
#include "Percept.h"

#include "WorldState.h"
#include "Location.h"
#include "Orientation.h"
#include "Search.h"
#include <list>

class Agent
{
public:
	Agent ();
	~Agent ();
	void Initialize ();
	Action Process (Percept& percept);//One of major API to main function
	void GameOver (int score);//One of major API to main function
	void UpdateState(Action lastAction, Percept& percept);

private:
	Location SafeUnvisitedLocation();
	bool MemberLocation(Location& location, list<Location>& locationList);//check if a location in a list
	void UpdateSafeLocations(Location& location);
	void RemoveOutsideLocations();
	void AdjacentLocations(Location& location, list<Location>& adjacentLocations);//Return adjacent location
	void LocateWumpus();
	void KillWumpus();//kill wumpus and go to wumpus's location
	void GetAllFrontiers();//get all frontier(may be some of them are pit locations)
	bool FrontierAdjacentVisited(Location& location);
	bool SameLocation(Location& location1, Location& location2);//check two locations are same
	double ComputerPossibility(Location& location);//You are encouraged to implement uncertainty reasoning so that the agent can determine the probability of a pit in a location.
	void BreezeFilter();//infer pit locations from breeze information
	void TrueFrontiers();//get actual frontiers
	bool CheckConsistent(list<Location>& tmpPit);//check if in combination C condition, breeze keep consistent
	void CreateC(int nFrontier, int arr[], int i);//// compute all possible combination C

	bool worldSizeKnown;
	list<Location> stenchLocations;
	list<Location> breezeLocations;
	list<Location> visitedLocations;
	list<Location> safeLocations;
	list<Location> pitLocations;
	list<Location> allFrontiers;
	double possibilities[20] = { 1.0 };
	list<Location> usefulFrontiers;
	list<Action> actionList;
	int tmpArr[2048][20];//combination C
	int c;//support variable 

	WorldState worldState;
	Action lastAction;
	SearchEngine searchEngine;
};




#endif // AGENT_H
