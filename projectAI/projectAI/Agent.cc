// Agent.cc
//
// Fall 2020 HW5
//
// Author: Larry Holder

#include <iostream>
#include <list>
#include <algorithm>
#include "Agent.h"
#include "Location.h"
#include "math.h"

using namespace std;

Agent::Agent ()
{
	// Initialize new agent based on new, unknown world
    worldState.agentLocation = Location(1,1);
	worldState.agentOrientation = RIGHT;
	worldState.agentHasGold = false;
	lastAction = CLIMB; // dummy action
    worldState.worldSize = 3; // HW5: somewhere between 3x3 and 9x9
    worldSizeKnown = false; 
    worldState.wumpusLocation = Location(0,0); // unknown
	worldState.goldLocation = Location(0,0); // unknown
}

Agent::~Agent ()
{
	
}

void Agent::Initialize ()
{
	// Initialize agent back to the beginning of the world
	worldState.agentLocation = Location(1,1);
	worldState.agentOrientation = RIGHT;
	worldState.agentAlive = true;
	worldState.wumpusAlive = true;
	worldState.agentHasGold = false;
	worldState.agentHasArrow = true;
	lastAction = CLIMB; // dummy action
	actionList.clear();
	if (!(worldState.goldLocation == Location(0, 0))) {
		if (MemberLocation(worldState.wumpusLocation, safeLocations)) {
			searchEngine.RemoveSafeLocation(worldState.wumpusLocation.X, worldState.wumpusLocation.Y);
		}
		if (MemberLocation(worldState.wumpusLocation, visitedLocations)) {
			visitedLocations.remove(worldState.wumpusLocation);
		}
	}
}

Action Agent::Process (Percept& percept)
{
	list<Action> actionList2;
	UpdateState(lastAction, percept);
	if (actionList.empty()) {
		if (percept.Glitter) {
			// If there is gold, then GRAB it
			cout << "Found gold. Grabbing it.\n";
			actionList.push_back(GRAB);
		}else if (worldState.agentHasGold && (worldState.agentLocation == Location(1,1))) {
			// If agent has gold and is in (1,1), then CLIMB
			cout << "Have gold and in (1,1). Climbing.\n";
			actionList.push_back(CLIMB);
		} else if (!worldState.agentHasGold && !(worldState.goldLocation == Location(0, 0))) {
			// If agent doesn't have gold, but knows its location, then navigate to that location
			cout << "Moving to known gold location (" << worldState.goldLocation.X << "," << worldState.goldLocation.Y << ").\n";
			actionList2 = searchEngine.FindPath(worldState.agentLocation, worldState.agentOrientation, worldState.goldLocation, worldState.agentOrientation);
			if (actionList2.empty()) {
				cout << "No path to safe unvisited location, start to kill wumpus and go to wumpus location" << endl;
				KillWumpus();
			}
			actionList.splice(actionList.end(), actionList2);
		} else if (worldState.agentHasGold && !(worldState.agentLocation == Location(1, 1))) {
			// If agent has gold, but isn't in (1,1), then navigate to (1,1)
			cout << "Having gold. Moving to (1,1).\n";
			actionList2 = searchEngine.FindPath(worldState.agentLocation, worldState.agentOrientation, Location(1,1), DOWN);
			actionList.splice(actionList.end(), actionList2);
		} else {
			// Determine safe unvisited location and navigate there (there will always be one for HW5)
			Location safeUnvisitedLocation = SafeUnvisitedLocation();
			cout << "Moving to safe unvisited location (" << safeUnvisitedLocation.X << "," << safeUnvisitedLocation.Y << ").\n";
			actionList2 = searchEngine.FindPath(worldState.agentLocation, worldState.agentOrientation, safeUnvisitedLocation, worldState.agentOrientation);
			if (!(actionList2.empty())) {
				actionList.splice(actionList.end(), actionList2);
			} else {
				if (!worldState.agentHasGold && !(worldState.wumpusLocation == Location(0, 0)) && worldState.agentHasArrow) {
					//if agent does not have the gold, and there are no remaining safe unvisited locations
					//if agent knows the location of the live Wumpus and agent has arrow
					cout << "No path to safe unvisited location, start to kill wumpus and go to wumpus location" << endl;
					KillWumpus();
					//Kill wumpus and go to wumpus location to see if wumpus location has gold
				}
				else {
					cout << "No gold found. Start taking risk and moving to the unknown locations" << endl;
					cout << "Known locations are visited locations.(too many locations so I won't show here)" << endl;
					GetAllFrontiers();					
					BreezeFilter();
					cout << "Breeze locations:" << endl;
					for (list<Location>::iterator it = breezeLocations.begin(); it != breezeLocations.end(); ++it) {
						Location location;
						location = *it;
						cout << "Breeze(" << location.X << "," << location.Y << ")";
					}
					cout << endl;
					TrueFrontiers();
					cout << "Frontier locations:" << endl;
					int i = 0;
					double p = 1.0;
					Location destination;
					for (list<Location>::iterator it = allFrontiers.begin(); it != allFrontiers.end(); ++it) {
						//it is query
						Location location;
						location = *it;
						cout << "Frontier(" << location.X << "," << location.Y << ")";
						possibilities[i]=ComputerPossibility(location);
						if (possibilities[i] <= p) {
							destination = location;							
							p = possibilities[i];
						}
						cout << "Possibility is:" << possibilities[i];
						cout << endl;
						i++;
					}
					cout << "Go To Frontier(" << destination.X << "," << destination.Y << ")" << endl;
					if (!MemberLocation(destination, safeLocations)) {
						searchEngine.AddSafeLocation(destination.X, destination.Y);
					}					
					actionList2 = searchEngine.FindPath(worldState.agentLocation, worldState.agentOrientation, destination, worldState.agentOrientation);
					actionList.splice(actionList.end(), actionList2);
				}
			}
		}
	}
	cout << endl;
	Action action = actionList.front();
	if (actionList.empty()) {
		cout << "EMPTY!" << endl;
	}
	actionList.pop_front();
	lastAction = action;
	return action;
}

void Agent::GameOver (int score)
{
	//Agent dead from pit then store pit location and never go to pit location again
	if (worldState.agentOrientation == UP) {
		pitLocations.push_back(Location(worldState.agentLocation.X, worldState.agentLocation.Y + 1));
	}
	else if (worldState.agentOrientation == DOWN) {
		pitLocations.push_back(Location(worldState.agentLocation.X, worldState.agentLocation.Y - 1));
	}
	else if (worldState.agentOrientation == RIGHT) {
		pitLocations.push_back(Location(worldState.agentLocation.X + 1, worldState.agentLocation.Y));
	}
	else {
		pitLocations.push_back(Location(worldState.agentLocation.X - 1, worldState.agentLocation.Y));
	}	
}

void Agent::UpdateState(Action lastAction, Percept& percept) {
	int X = worldState.agentLocation.X;
	int Y = worldState.agentLocation.Y;
	Orientation orientation = worldState.agentOrientation;

	if (lastAction == TURNLEFT) {
		worldState.agentOrientation = (Orientation) ((orientation + 1) % 4);
	}
	if (lastAction == TURNRIGHT) {
		if (orientation == RIGHT) {
			worldState.agentOrientation = DOWN;
		} else {
			worldState.agentOrientation = (Orientation) (orientation - 1);
		}
	}
	if (lastAction == GOFORWARD) {
		if (percept.Bump) {
			if ((orientation == RIGHT) || (orientation == UP)) {
				cout << "World size known to be " << worldState.worldSize << "x" << worldState.worldSize << endl;
				worldSizeKnown = true;
				RemoveOutsideLocations();
			}
		} else {
			switch (orientation) {
			case UP:
				worldState.agentLocation.Y = Y + 1;
				break;
			case DOWN:
				worldState.agentLocation.Y = Y - 1;
				break;
			case LEFT:
				worldState.agentLocation.X = X - 1;
				break;
			case RIGHT:
				worldState.agentLocation.X = X + 1;
				break;
			}
		}
	}
	if (lastAction == GRAB) { // Assume GRAB only done if Glitter was present
		worldState.agentHasGold = true;
	}
	if (lastAction == CLIMB) {
		// do nothing; if CLIMB worked, this won't be executed anyway
	}
	if (lastAction == SHOOT) {
		worldState.agentHasArrow = false;
	}
	// HW5.2 state update requirements (note: 2g no longer required)
	if (percept.Scream) {
		worldState.wumpusAlive = false;
	}
	if ((percept.Stench) && (! MemberLocation(worldState.agentLocation, stenchLocations))) {
		stenchLocations.push_back(worldState.agentLocation);
	}
	if ((percept.Breeze) && (! MemberLocation(worldState.agentLocation, breezeLocations))) {
		breezeLocations.push_back(worldState.agentLocation);
	}
	if (percept.Glitter) {
		worldState.goldLocation = worldState.agentLocation;
		cout << "Found gold at (" << worldState.goldLocation.X << "," << worldState.goldLocation.Y << ")\n";
	}
	int new_max = max(worldState.agentLocation.X, worldState.agentLocation.Y);
	if (new_max > worldState.worldSize) {
		worldState.worldSize = new_max;
	}
	if (worldState.wumpusLocation == Location(0,0)) {
		LocateWumpus();
	}
	UpdateSafeLocations(worldState.agentLocation);
	if (!(MemberLocation(worldState.agentLocation, visitedLocations))) {
		visitedLocations.push_back(worldState.agentLocation);
	}
	if (MemberLocation(worldState.agentLocation, allFrontiers)) {
		allFrontiers.remove(worldState.agentLocation);
	}
}

bool Agent::MemberLocation(Location& location, list<Location>& locationList) {
	if (find(locationList.begin(), locationList.end(), location) != locationList.end()) {
		return true;
	}
	return false;
}

Location Agent::SafeUnvisitedLocation() {
	// Find and return safe unvisited location.
	list<Location>::iterator loc_itr;
	for (loc_itr = safeLocations.begin(); loc_itr != safeLocations.end(); ++loc_itr) {
		if (!(MemberLocation(*loc_itr, visitedLocations))) {
			return *loc_itr;
		}
	}
	return Location(0,0);
}

void Agent::UpdateSafeLocations(Location& location) {
	// Add location to safe locations. Also add adjacent locations, if no stench or breeze.
	// Or just no breeze, if stench to be ignored.
	if (!(MemberLocation(location, safeLocations)) && !(MemberLocation(location,pitLocations))) {
		safeLocations.push_back(location);
		searchEngine.AddSafeLocation(location.X, location.Y);
	}
	bool ignoreStench = false;
	if (!(worldState.wumpusLocation == Location(0,0))) {
		ignoreStench = true; // we know location of wumpus
	}
	if ((!(MemberLocation(location, stenchLocations)) || ignoreStench) && (!(MemberLocation(location, breezeLocations)))) {
		list<Location> adj_locs;
		AdjacentLocations(location, adj_locs);
		list<Location>::iterator loc_itr;
		for (loc_itr = adj_locs.begin(); loc_itr != adj_locs.end(); ++loc_itr) {
			if ((!(*loc_itr == worldState.wumpusLocation)) && (!(MemberLocation(*loc_itr, safeLocations)))) {
				safeLocations.push_back(*loc_itr);
				searchEngine.AddSafeLocation(loc_itr->X, loc_itr->Y);
			}
		}
	}
	for (list<Location>::iterator it1 = pitLocations.begin(); it1 != pitLocations.end(); ++it1) {
		searchEngine.RemoveSafeLocation(it1->X, it1->Y);
	}
}

void Agent::RemoveOutsideLocations() {
	// Know exact world size, so remove locations outside the world.
	int boundary = worldState.worldSize + 1;
	for (int i = 1; i < boundary; ++i) {
		safeLocations.remove(Location(i,boundary));
		searchEngine.RemoveSafeLocation(i,boundary);
		safeLocations.remove(Location(boundary,i));
		searchEngine.RemoveSafeLocation(boundary,i);
	}
	safeLocations.remove(Location(boundary,boundary));
	searchEngine.RemoveSafeLocation(boundary,boundary);
}

void Agent::AdjacentLocations(Location& location, list<Location>& adjacentLocations) {
	// Append locations adjacent to given location on to give locations list.
	// One row/col beyond unknown world size is okay. Locations outside the world
	// will be removed later.
	int X = location.X;
	int Y = location.Y;
	if (X > 1) {
		adjacentLocations.push_back(Location(X-1,Y));
	}
	if (Y > 1) {
		adjacentLocations.push_back(Location(X,Y-1));
	}
	if (worldSizeKnown) {
		if (X < worldState.worldSize) {
			adjacentLocations.push_back(Location(X+1,Y));
		}
		if (Y < worldState.worldSize) {
			adjacentLocations.push_back(Location(X,Y+1));
		}
	} else {
		adjacentLocations.push_back(Location(X+1,Y));
		adjacentLocations.push_back(Location(X,Y+1));
	}
}

void Agent::LocateWumpus() {
	// Check stench and safe location info to see if wumpus can be located.
	// If so, then stench locations should be safe.
	list<Location>::iterator loc_itr_1;
	list<Location>::iterator loc_itr_2;
	for (loc_itr_1 = stenchLocations.begin(); loc_itr_1 != stenchLocations.end(); ++loc_itr_1) {
		int x1 = loc_itr_1->X;
		int y1 = loc_itr_1->Y;
		for (loc_itr_2 = stenchLocations.begin(); loc_itr_2 != stenchLocations.end(); ++loc_itr_2) {
			int x2 = loc_itr_2->X;
			int y2 = loc_itr_2->Y;
			Location loc = Location(x1+1,y1);
			if ((x1 == x2-1) && (y1 == y2 - 1) && (MemberLocation(loc, safeLocations))) {
				worldState.wumpusLocation = Location(x1,y1+1);
			}
			loc.X = x1;
			loc.Y = y1+1;
			if ((x1 == x2-1) && (y1 == y2 - 1) && (MemberLocation(loc, safeLocations))) {
				worldState.wumpusLocation = Location(x1+1,y1);
			}
			loc.X = x1-1;
			loc.Y = y1;
			if ((x1 == x2+1) && (y1 == y2 - 1) && (MemberLocation(loc, safeLocations))) {
				worldState.wumpusLocation = Location(x1,y1+1);
			}
			loc.X = x1;
			loc.Y = y1+1;
			if ((x1 == x2+1) && (y1 == y2 - 1) && (MemberLocation(loc, safeLocations))) {
				worldState.wumpusLocation = Location(x1-1,y1);
			}
		}
	}
	if (!(worldState.wumpusLocation == Location(0,0))) {
		cout << "Found wumpus at (" << worldState.wumpusLocation.X << "," << worldState.wumpusLocation.Y << ")\n";
	}
	
}

void Agent::KillWumpus() {
	//The most efficent way to kill wumpus and goto its location is find a shortest was to wumpus adjacent location while facing wumpus
	//Then shoot arrow and go forward
	int wx = worldState.wumpusLocation.X;
	int wy = worldState.wumpusLocation.Y;
	int size; // how long from agent to wumpus adjacent location while facing wumpus
	Location location;
	list<Action> actionList2;
	list<Action> actionList1;
	if (wx > 1) {
		location.X = wx - 1;
		location.Y = wy;
		actionList2 = searchEngine.FindPath(worldState.agentLocation, worldState.agentOrientation, location, RIGHT);
		actionList1 = actionList2;
		if (!actionList2.empty()) {
			size = actionList2.size();
		}		
	}
	if (wx < worldState.worldSize) {
		location.X = wx + 1;
		location.Y = wy;
		actionList2 = searchEngine.FindPath(worldState.agentLocation, worldState.agentOrientation, location, LEFT);
		if (actionList2.size() < size && !actionList2.empty()) {
			size = actionList2.size();
			actionList1 = actionList2;
		}
	}
	if (wy > 1) {
		location.X = wx;
		location.Y = wy - 1;
		actionList2 = searchEngine.FindPath(worldState.agentLocation, worldState.agentOrientation, location, UP);
		if (actionList2.size() < size && !actionList2.empty()) {
			size = actionList2.size();
			actionList1 = actionList2;
		}
	}
	if (wy < worldState.worldSize) {
		location.X = wx;
		location.Y = wy + 1;
		actionList2 = searchEngine.FindPath(worldState.agentLocation, worldState.agentOrientation, location, DOWN);
		if (actionList2.size() < size && !actionList2.empty()) {
			actionList1 = actionList2;
		}
	}
	actionList.splice(actionList.end(), actionList1);
	actionList.push_back(SHOOT);
	worldState.wumpusAlive = false;
	searchEngine.AddSafeLocation(worldState.wumpusLocation.X, worldState.wumpusLocation.Y);
	actionList.push_back(GOFORWARD);
}

void Agent::GetAllFrontiers() {
	//Get all frontiers from what we known
	for (int i = 1; i < worldState.worldSize + 1; i++) {
		for (int j = 1; j < worldState.worldSize + 1; j++) {
			//Traverse the map.
			Location location;
			location.X = i;
			location.Y = j;
			if (!MemberLocation(location, visitedLocations) && FrontierAdjacentVisited(location)) {
				//unvisited location && its adjacent locations have been visited
				//add to allfrontier list
				if (!MemberLocation(location, allFrontiers) && !SameLocation(location,worldState.wumpusLocation)) {
					allFrontiers.push_back(location);
				}
			}
		}
	}
}

bool Agent::SameLocation(Location& location1, Location& location2) {
	//if two locations are same return true
	if (location1.X == location2.X && location1.Y == location2.Y) {
		return true;
	}
	return false;
}

bool Agent::FrontierAdjacentVisited(Location& location) {
	//this is the function to check if input location is frontier or others
	//if frontier's adjacent locations have been visited, return true, otherwise r eturn false
	Location frontier;
	int X = location.X;
	int Y = location.Y;
	if (X > 1) {
		frontier.X = X - 1;
		frontier.Y = Y;
		if (MemberLocation(frontier, visitedLocations)) {
			return true;
		}
	}
	if (Y > 1) {
		frontier.X = X;
		frontier.Y = Y - 1;
		if (MemberLocation(frontier, visitedLocations)) {
			return true;
		}
	}
	if (X < worldState.worldSize) {
		frontier.X = X + 1;
		frontier.Y = Y;
		if (MemberLocation(frontier, visitedLocations)) {
			return true;
		}
	}
	if (Y < worldState.worldSize) {
		frontier.X = X;
		frontier.Y = Y + 1;
		if (MemberLocation(frontier, visitedLocations)) {
			return true;
		}
	}
	return false;
}

void Agent::BreezeFilter() {
	//find pit locations from breeze infomation: for those breeze has only one adjacent hasn't been visited, the adjacent location is pit
	for (list<Location>::iterator it1 = breezeLocations.begin(); it1 != breezeLocations.end(); ++it1) {
		int x = 0;
		list<Location> adj_breeze;
		AdjacentLocations(*it1,adj_breeze);
		for (list<Location>::iterator it2 = adj_breeze.begin(); it2 != adj_breeze.end(); ++it2) {
			for (list<Location>::iterator it3 = allFrontiers.begin(); it3 != allFrontiers.end(); ++it3) {
				if (SameLocation(*it2, *it3)) {
					x++;
				}
			}
		}
		if (x == 1) {
			list<Location> adj_breeze;
			AdjacentLocations(*it1, adj_breeze);
			for (list<Location>::iterator it2 = adj_breeze.begin(); it2 != adj_breeze.end(); ++it2) {
				for (list<Location>::iterator it3 = allFrontiers.begin(); it3 != allFrontiers.end(); ++it3) {
					if (SameLocation(*it2, *it3) && !MemberLocation(*it3, pitLocations)) {
						pitLocations.push_back(*it3);						
					}
				}
			}
		}
	}
	cout << "Pit locations:" << endl;
	for (list<Location>::iterator it = pitLocations.begin(); it != pitLocations.end(); ++it) {
		Location location;
		location = *it;
		cout << "Pit(" << location.X << "," << location.Y << ")";
	}
	cout << endl;
}

void Agent::TrueFrontiers() {
	for (list<Location>::iterator it1 = pitLocations.begin(); it1 != pitLocations.end(); ++it1) {
		allFrontiers.remove(*it1);
	}
}

double Agent::ComputerPossibility(Location& location) {
	//input P(Pitx,y)
	double Pt = 0.0;//P(Pitx,y=true) = 0.0
	double Pf = 0.0;//P(Pitx,y=false) = 0.0
	list<Location> frontier;
	frontier = allFrontiers;
	frontier.remove(location);//Frontier¡¯ = Frontier ¨C {(x,y)}
	int arr[frontier.size()];	
	c = 0;
	CreateC(frontier.size(), arr, 0);
	// compute all possible combination C
	for (int i = 0; i < pow(2, frontier.size()); i++) {
		//for each possible combination in C
		int t = 0;//number of pit=true in C
		int f = 0;//number of pit=false in C
		list<Location> tmpPit;
		tmpPit = pitLocations;
		int j = 0;
		for (list<Location>::iterator it = frontier.begin(); it != frontier.end();++it) {
			if (tmpArr[i][j] == 1) {
				t++;
				if (!MemberLocation(*it, tmpPit)) {
					tmpPit.push_back(*it);
				}					
			}
			else {
				f++;
			}
			j++;
		}		
		double p = pow(0.2, t) * pow(0.8, f);
		if (CheckConsistent(tmpPit)) {
			//consistent with P(x,y)=false			
			Pf += p;
		}
		tmpPit.push_back(location);
		if (CheckConsistent(tmpPit)) {
			//consistent with P(x,y)=true
			Pt += p;
		}
	}
	Pt *= 0.2;
	Pf *= 0.8;	
	return Pt / (Pt + Pf);
}

void Agent::CreateC(int nFrontier, int arr[], int i)
{
	if (i == nFrontier) {
		for (int i = 0; i < nFrontier; i++) {
			tmpArr[c][i] = arr[i];			
		}
		c++;
		return;
	};
	arr[i] = 0;
	CreateC(nFrontier, arr, i + 1);
	arr[i] = 1;
	CreateC(nFrontier, arr, i + 1);
}

bool Agent::CheckConsistent(list<Location>& tmpPit) {
	for (list<Location>::iterator it1 = breezeLocations.begin(); it1 != breezeLocations.end(); ++it1) {
		//check all breeze
		list<Location> breeze_adj;
		AdjacentLocations(*it1, breeze_adj);//get breeze's adjacent locations
		int i = 0;
		for (list<Location>::iterator it2 = breeze_adj.begin(); it2 != breeze_adj.end(); ++it2) {
			//for each adjacent location
			for (list<Location>::iterator it3 = tmpPit.begin(); it3 != tmpPit.end(); ++it3) {
				//check if it's frontier locations
				if (SameLocation(*it2, *it3)) {
					i++;
				}
			}
		}
		if (i == 0) {//if no frontier locations
			return false;
		}
	}
	return true;
}