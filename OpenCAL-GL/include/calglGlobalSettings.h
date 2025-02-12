// (C) Copyright University of Calabria and others.
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the GNU Lesser General Public License
// (LGPL) version 2.1 which accompanies this distribution, and is available at
// http://www.gnu.org/licenses/lgpl-2.1.html
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.

#ifndef calglGlobalSettings_h
#define calglGlobalSettings_h

#include <calCommon.h>
#include <calglCommon.h>

#ifndef _WIN32

unsigned int Sleep(unsigned int);

#endif

struct CALGLGlobalSettings {
	char* demPath;
	char* sourcePath;
	char* outputPath;
	char* applicationName;
	const char* iconPath;

	int rows;
	int columns;
	int cellSize;
	int step;	

	int width;
	int height;
	int positionX;
	int positionY;

	int zNear;
	int zFar;


	CALbyte onlyModel;
	enum CALGL_LIGHT lightStatus;

	CALbyte fixedDisplay;
	int fixedStep;

	int refreshTime;
};

/*! Constructor
*/
struct CALGLGlobalSettings* calglCreateGlobalSettings();

/*! Destructor
*/
void calglDestroyGlobalSettings();

void calglSetPathGlobalSettings(char* demPath, char* sourcePath, char* outputPath);

void calglSetApplicationNameGlobalSettings(char* applicationName);

void calglSetRowsAndColumnsGlobalSettings(int rows, int columns);

void calglSetCellSizeGlobalSettings(int cellSize);

void calglSetStepGlobalSettings(int step);

void calglSetWindowDimensionGlobalSettings(int width, int height);

void calglSetWindowPositionGlobalSettings(int positionX, int positionY);

void calglSetClippingFactorGlobalSettings(int zNear, int zFar);

struct CALGLGlobalSettings* calglGetGlobalSettings();

void calglEnableLights();

void calglDisableLights();

enum CALGL_LIGHT calglAreLightsEnable();

float* calglGetPositionLight();

float* calglGetDiffuseLight(); 

float* calglGetSpecularLight(); 

float* calglGetAmbientLight(); 

void calglSetRefreshTime(int time);

void calglSetFixedDisplayStep(int step);

#endif
