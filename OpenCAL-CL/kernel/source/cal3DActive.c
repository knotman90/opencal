#include "cal3DActive.h"

void calAddActiveCell3D(MODEL_DEFINITION3D, int i, int j, int k) {

	if (CAL_FALSE == calGetBufferElement3D(get_active_cells_flags(), get_rows(), get_columns(), i, j, k)){
		calSetBufferElement3D(get_active_cells_flags(), get_rows(), get_columns(), i, j, k, CAL_TRUE);
		CALint chunkNum = (k*get_rows()*get_columns()+ i*get_columns() + j)/CALCLchunk;
		CALCLdiff[chunkNum] = CAL_TRUE;
	}

}
void calAddActiveCellX3D(MODEL_DEFINITION3D, int i, int j, int k, int n) {

	CALint ix;
	CALint jx;
	CALint kx;

	if (get_boundary_condition() == CAL_SPACE_FLAT) {
		ix = i + get_neighborhood()[n].i;
		jx = j + get_neighborhood()[n].j;
		kx = k + get_neighborhood()[n].k;
	} else {
		ix = calGetToroidalX(i + get_neighborhood()[n].i, get_rows());
		jx = calGetToroidalX(j + get_neighborhood()[n].j, get_columns());
		kx = calGetToroidalX(k + get_neighborhood()[n].k, get_slices());
	}

	if (CAL_FALSE == calGetBufferElement3D(get_active_cells_flags(), get_rows(), get_columns(), ix, jx, kx)){
		calSetBufferElement3D(get_active_cells_flags(), get_rows(), get_columns(), ix, jx, kx, CAL_TRUE);
		CALint chunkNum = (kx*get_rows()*get_columns()+ ix*get_columns() + jx)/CALCLchunk;
		CALCLdiff[chunkNum] = CAL_TRUE;
	}
}

void calRemoveActiveCell3D(MODEL_DEFINITION3D, int i, int j, int k) {

	if (CAL_TRUE == calGetBufferElement3D(get_active_cells_flags(), get_rows(), get_columns(), i, j, k)){
		calSetBufferElement3D(get_active_cells_flags(), get_rows(), get_columns(), i, j, k, CAL_FALSE);
		CALint chunkNum = (k*get_rows()*get_columns()+ i*get_columns() + j)/CALCLchunk;
		CALCLdiff[chunkNum] = CAL_TRUE;
	}

}

void calInitSubstateActiveCell3Db(MODEL_DEFINITION3D, CALbyte value, int n, int substateNum) {
	__global CALbyte * current = calGetCurrentSubstate3Db(MODEL3D, substateNum);
	__global CALbyte * next = calGetNextSubstate3Db(MODEL3D, substateNum);
	calSetBufferActiveCells3Db(current, get_rows(), get_columns(), value, get_active_cells(), n);
	calSetBufferActiveCells3Db(next, get_rows(), get_columns(), value, get_active_cells(), n);
}
void calInitSubstateActiveCell3Di(MODEL_DEFINITION3D, CALint value, int n, int substateNum) {
	__global CALint * current = calGetCurrentSubstate3Di(MODEL3D, substateNum);
	__global CALint * next = calGetNextSubstate3Di(MODEL3D, substateNum);
	calSetBufferActiveCells3Di(current, get_rows(), get_columns(), value, get_active_cells(), n);
	calSetBufferActiveCells3Di(next, get_rows(), get_columns(), value, get_active_cells(), n);
}
void calInitSubstateActiveCell3Dr(MODEL_DEFINITION3D, CALreal value, int n, int substateNum) {
	__global CALreal * current = calGetCurrentSubstate3Dr(MODEL3D, substateNum);
	__global CALreal * next = calGetNextSubstate3Dr(MODEL3D, substateNum);
	calSetBufferActiveCells3Dr(current, get_rows(), get_columns(), value, get_active_cells(), n);
	calSetBufferActiveCells3Dr(next, get_rows(), get_columns(), value, get_active_cells(), n);
}





