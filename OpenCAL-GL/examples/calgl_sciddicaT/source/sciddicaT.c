#include <cal2D.h>
#include <cal2DIO.h>
#include <cal2DReduction.h>
#include <cal2DRun.h>
#include <calgl2D.h>
#include <calgl2DWindow.h>
#include <stdlib.h>


//-----------------------------------------------------------------------
//	   THE sciddicaT(oy) cellular automaton definition section
//-----------------------------------------------------------------------

#define P_R 0.5
#define P_EPSILON 0.001
#define NUMBER_OF_OUTFLOWS 4

struct sciddicaTSubstates {
	struct CALSubstate2Dr *z;
	struct CALSubstate2Dr *h;
	struct CALSubstate2Dr *f[NUMBER_OF_OUTFLOWS];
};

struct sciddicaTParameters {
	CALParameterr epsilon;
	CALParameterr r;
};

struct CALModel2D* sciddicaT;						//the cellular automaton
struct sciddicaTSubstates Q;						//the substates
struct sciddicaTParameters P;						//the parameters
struct CALRun2D* sciddicaTsimulation;				//the simulartion run


//------------------------------------------------------------------------------
//					sciddicaT transition function
//------------------------------------------------------------------------------

//first elementary process
void sciddicaT_flows_computation(struct CALModel2D* sciddicaT, int i, int j)
{
	CALbyte eliminated_cells[5]={CAL_FALSE,CAL_FALSE,CAL_FALSE,CAL_FALSE,CAL_FALSE};
	CALbyte again;
	CALint cells_count;
	CALreal average;
	CALreal m;
	CALreal u[5];
	CALint n;
	CALreal z, h;


	if (calGet2Dr(sciddicaT, Q.h, i, j) <= P.epsilon)
		return;

	m = calGet2Dr(sciddicaT, Q.h, i, j) - P.epsilon;
	u[0] = calGet2Dr(sciddicaT, Q.z, i, j) + P.epsilon;
	for (n=1; n<sciddicaT->sizeof_X; n++)
	{
		z = calGetX2Dr(sciddicaT, Q.z, i, j, n);
		h = calGetX2Dr(sciddicaT, Q.h, i, j, n);
		u[n] = z + h;
	}

	//computes outflows
	do{
		again = CAL_FALSE;
		average = m;
		cells_count = 0;

		for (n=0; n<sciddicaT->sizeof_X; n++)
			if (!eliminated_cells[n]){
				average += u[n];
				cells_count++;
			}

			if (cells_count != 0)
				average /= cells_count;

			for (n=0; n<sciddicaT->sizeof_X; n++)
				if( (average<=u[n]) && (!eliminated_cells[n]) ){
					eliminated_cells[n]=CAL_TRUE;
					again=CAL_TRUE;
				}

	}while (again); 

	for (n=1; n<sciddicaT->sizeof_X; n++)
		if (eliminated_cells[n])
			calSet2Dr(sciddicaT, Q.f[n-1], i, j, 0.0);
		else
			calSet2Dr(sciddicaT, Q.f[n-1], i, j, (average-u[n])*P.r);
}

//second (and last) elementary process
void sciddicaT_width_update(struct CALModel2D* sciddicaT, int i, int j)
{
	CALreal h_next;
	CALint n;

	h_next = calGet2Dr(sciddicaT, Q.h, i, j);
	for(n=1; n<sciddicaT->sizeof_X; n++)
		h_next +=  calGetX2Dr(sciddicaT, Q.f[NUMBER_OF_OUTFLOWS - n], i, j, n) - calGet2Dr(sciddicaT, Q.f[n-1], i, j);

	calSet2Dr(sciddicaT, Q.h, i, j, h_next);
}

//------------------------------------------------------------------------------
//					sciddicaT simulation functions
//------------------------------------------------------------------------------

void sciddicaTSimulationInit(struct CALModel2D* sciddicaT)
{
	CALreal z, h;
	CALint i, j;

	//initializing substates to 0
	calInitSubstate2Dr(sciddicaT, Q.f[0], 0);
	calInitSubstate2Dr(sciddicaT, Q.f[1], 0);
	calInitSubstate2Dr(sciddicaT, Q.f[2], 0);
	calInitSubstate2Dr(sciddicaT, Q.f[3], 0);

	//sciddicaT parameters setting
	P.r = P_R;
	P.epsilon = P_EPSILON;

	//sciddicaT source initialization
	for (i=0; i<sciddicaT->rows; i++)
		for (j=0; j<sciddicaT->columns; j++)
		{
			h = calGet2Dr(sciddicaT, Q.h, i, j);

			if ( h > 0.0 ) {
				z = calGet2Dr(sciddicaT, Q.z, i, j);
				calSet2Dr(sciddicaT, Q.z, i, j, z-h);
			}
		}
}

void sciddicaTSteering(struct CALModel2D* sciddicaT)
{
	CALreal value = 0;

	//initializing substates to 0
	calInitSubstate2Dr(sciddicaT, Q.f[0], 0);
	calInitSubstate2Dr(sciddicaT, Q.f[1], 0);
	calInitSubstate2Dr(sciddicaT, Q.f[2], 0);
	calInitSubstate2Dr(sciddicaT, Q.f[3], 0);

	value = calReductionComputeMax2Dr(sciddicaT, Q.z);
	printf("Max: %g\t", value);
}

CALbyte sciddicaTSimulationStopCondition(struct CALModel2D* sciddicaT)
{
	if (sciddicaTsimulation->step >= calglGetGlobalSettings()->step)
		return CAL_TRUE;
	return CAL_FALSE;
}

//------------------------------------------------------------------------------
//					sciddicaT main function
//------------------------------------------------------------------------------

int main(int argc, char** argv)
{
	struct CALDrawModel2D* model1 = NULL;
	//struct CALDrawModel2D* model2;
	//struct CALDrawModel2D* model3;
	//struct CALDrawModel2D* model4;
	
	
	calglSetPathGlobalSettings("./data/dem.txt", "./data/source.txt", "./data/width_final.txt");
	calglSetApplicationNameGlobalSettings("RealDraw");
	calglSetRowsAndColumnsGlobalSettings(610, 496);
	calglSetCellSizeGlobalSettings(5);
	calglSetStepGlobalSettings(4000);
	calglSetWindowDimensionGlobalSettings(800, 600);
	calglSetWindowPositionGlobalSettings(10, 10);
	calglEnableLights();
	//calglSetFixedDisplayStep(50);


	//cadef and rundef
	sciddicaT = calCADef2D(calglGetGlobalSettings()->rows, calglGetGlobalSettings()->columns, CAL_VON_NEUMANN_NEIGHBORHOOD_2D, CAL_SPACE_TOROIDAL, CAL_NO_OPT);
	sciddicaTsimulation = calRunDef2D(sciddicaT, 1, CAL_RUN_LOOP, CAL_UPDATE_IMPLICIT);
	//add substates
	Q.z = calAddSubstate2Dr(sciddicaT);
	Q.h = calAddSubstate2Dr(sciddicaT);
	Q.f[0] = calAddSubstate2Dr(sciddicaT);
	Q.f[1] = calAddSubstate2Dr(sciddicaT);
	Q.f[2] = calAddSubstate2Dr(sciddicaT);
	Q.f[3] = calAddSubstate2Dr(sciddicaT);
	//add transition function's elementary processes
	calAddElementaryProcess2D(sciddicaT, sciddicaT_flows_computation);
	calAddElementaryProcess2D(sciddicaT, sciddicaT_width_update);

	//load configuration
	calLoadSubstate2Dr(sciddicaT, Q.z, calglGetGlobalSettings()->demPath);
	calLoadSubstate2Dr(sciddicaT, Q.h, calglGetGlobalSettings()->sourcePath);
	//calSaveSubstate2Dr(sciddicaT, Q.h, OUTPUT_PATH);

	//simulation run setup
	calRunAddInitFunc2D(sciddicaTsimulation, sciddicaTSimulationInit); 
/**/calRunInitSimulation2D(sciddicaTsimulation);	//It is required in the case the simulation main loop is explicitated; similarly for calRunFinalizeSimulation2D
	calRunAddSteeringFunc2D(sciddicaTsimulation, sciddicaTSteering);
	calRunAddStopConditionFunc2D(sciddicaTsimulation, sciddicaTSimulationStopCondition);
	
	
	
	// model1 definition
	model1 = calglDefDrawModel2D(CALGL_DRAW_MODE_SURFACE, "SciddicaT", sciddicaT, sciddicaTsimulation);
	// Add nodes
	calglAddToDrawModel2Dr(model1, NULL, &Q.z, CALGL_TYPE_INFO_VERTEX_DATA, CALGL_TYPE_INFO_USE_DEFAULT, CALGL_DATA_TYPE_STATIC);
	calglColor2D(model1, 0.5, 0.5, 0.5, 1.0);
	calglAddToDrawModel2Dr(model1, Q.z, &Q.z, CALGL_TYPE_INFO_COLOR_DATA, CALGL_TYPE_INFO_USE_CONST_VALUE, CALGL_DATA_TYPE_DYNAMIC);
	calglAddToDrawModel2Dr(model1, Q.z, &Q.z, CALGL_TYPE_INFO_NORMAL_DATA, CALGL_TYPE_INFO_USE_DEFAULT, CALGL_DATA_TYPE_DYNAMIC);
	calglAddToDrawModel2Dr(model1, Q.z, &Q.h, CALGL_TYPE_INFO_VERTEX_DATA, CALGL_TYPE_INFO_USE_DEFAULT, CALGL_DATA_TYPE_DYNAMIC);
	calglAddToDrawModel2Dr(model1, Q.h, &Q.h, CALGL_TYPE_INFO_COLOR_DATA, CALGL_TYPE_INFO_USE_RED_SCALE, CALGL_DATA_TYPE_DYNAMIC);
	calglAddToDrawModel2Dr(model1, Q.h, &Q.h, CALGL_TYPE_INFO_NORMAL_DATA, CALGL_TYPE_INFO_USE_DEFAULT, CALGL_DATA_TYPE_DYNAMIC);
	// InfoBar
	calglInfoBar2Dr(model1, Q.h, "Debris thickness", CALGL_TYPE_INFO_USE_RED_SCALE, CALGL_INFO_BAR_VERTICAL);

	/*model2 = calglDefDrawModel2D(CALGL_DRAW_MODE_FLAT, "model2", sciddicaT, sciddicaTsimulation);
	model2->realModel = model1->realModel;
	calglInfoBar2Dr(model2, Q.h, "Debris", CALGL_TYPE_INFO_USE_BLUE_SCALE, CALGL_INFO_BAR_HORIZONTAL);

	model3 = calglDefDrawModel2D(CALGL_DRAW_MODE_FLAT, "model3", sciddicaT, sciddicaTsimulation);
	model3->realModel = model1->realModel;
	calglInfoBar2Dr(model3, Q.h, "Debris", CALGL_TYPE_INFO_USE_GREEN_SCALE, CALGL_INFO_BAR_VERTICAL);

	model4 = calglDefDrawModel2D(CALGL_DRAW_MODE_SURFACE, "model4", sciddicaT, sciddicaTsimulation);
	model4->realModel = model1->realModel;
	calglInfoBar2Dr(model4, Q.z, "height", CALGL_TYPE_INFO_USE_GRAY_SCALE, CALGL_INFO_BAR_HORIZONTAL);*/

	calglStartProcessWindow2D(argc, argv);


	//finalizations
	calRunFinalize2D(sciddicaTsimulation);
	calFinalize2D(sciddicaT);

	calglDestroyGlobalSettings();
	
	return 0;
}
