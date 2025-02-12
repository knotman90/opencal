#ifndef life_h
#define life_h

#include <cal2D.h>
#include <cal2DRun.h>

#define ROWS 1024
#define COLS 1024
#define STATE_DEAD 0
#define STATE_ALIVE 1

//cadef and rundef
struct CellularAutomaton {
	struct CALModel2D* model;		//the cellular automaton
	struct isoSubstates* Q;			//the set of call's states over the whole cellular space
	struct CALRun2D* run;		//the simulartion run
};

extern struct CellularAutomaton life;

void CADef(struct CellularAutomaton* ca);
void Init(struct CellularAutomaton* ca);
void Exit(struct CellularAutomaton* ca);

#endif