#include "OpenCL_Utility.h"

CALOpenCL * calclCreateCALOpenCL() {
	CALOpenCL * calOpenCL = (CALOpenCL*) malloc(sizeof(CALOpenCL));
	return calOpenCL;
}

void calclInitializePlatforms(CALOpenCL * opencl) {

	opencl->num_platforms = 0;
	CALCLint err = clGetPlatformIDs(0, NULL, &opencl->num_platforms);
	calclHandleError(err);
	opencl->platforms = (CALCLplatform*) malloc(sizeof(CALCLplatform) * opencl->num_platforms);
	err = clGetPlatformIDs(opencl->num_platforms, opencl->platforms, NULL);
	calclHandleError(err);
}

void calclInitializeDevices(CALOpenCL * opencl) {
	opencl->devices = (CALCLdevice**) malloc(sizeof(CALCLdevice*) * opencl->num_platforms);
	opencl->num_platforms_devices = (int*) malloc(sizeof(int) * opencl->num_platforms);
	CALCLuint num_devices;
	unsigned i = 0;
	for (i = 0; i < opencl->num_platforms; i++) {
		CALCLint err = clGetDeviceIDs(opencl->platforms[i], CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);
		calclHandleError(err);
		opencl->devices[i] = (CALCLdevice*) malloc(sizeof(CALCLdevice) * num_devices);
		opencl->num_platforms_devices[i] = num_devices;
		err = clGetDeviceIDs(opencl->platforms[i], CL_DEVICE_TYPE_ALL, num_devices, opencl->devices[i], NULL);
		calclHandleError(err);
	}
}

CALCLcontext calclcreateContext(CALCLdevice * devices, CALCLuint num_devices) {
	CALCLint err;
	CALCLcontext context = clCreateContext(NULL, num_devices, devices, NULL, NULL, &err);
	calclHandleError(err);
	return context;
}

CALCLmem calclCreateBuffer(CALCLcontext context, void* host_ptr, size_t size) {
	CALCLint err;
	CALCLmem out = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, size, host_ptr, &err);
	calclHandleError(err);

	return out;
}

CALCLqueue calclCreateCommandQueue(CALCLcontext context, CALCLdevice device) {
	CALCLint err;
	CALCLqueue out = clCreateCommandQueue(context, device, (cl_command_queue_properties)NULL, &err);
	calclHandleError(err);
	return out;
}

CALCLdevice calclGetDevice(CALOpenCL * calOpenCL, int platformIndex, int deviceIndex) {
	return calOpenCL->devices[platformIndex][deviceIndex];
}

void calclFinalizeCALOpencl(CALOpenCL * opencl) {
	free(opencl->platforms);
	unsigned i = 0;
	for (i = 0; i < opencl->num_platforms; i++)
		free(opencl->devices[i]);
	free(opencl->devices);
	free(opencl->num_platforms_devices);
	free(opencl);
}

void calclGetDirFiles(char ** paths, int pathsNum, char *** files_names, int * num_files) {

	DIR *dir;
	struct dirent *ent;

	(*num_files) = 0;
	int i;
	for (i = 0; i < pathsNum; i++) {
		if ((dir = opendir(paths[i])) != NULL) {
			while ((ent = readdir(dir)) != NULL) {
				if (ent->d_name[0] != '.')
					(*num_files)++;
			}
			closedir(dir);

		} else {
			perror("could not open directory\n");
			exit(1);
		}
	}

	(*files_names) = (char**) malloc(sizeof(char*) * (*num_files));
	int count = 0;
	for (i = 0; i < pathsNum; i++) {
		if ((dir = opendir(paths[i])) != NULL) {
			while ((ent = readdir(dir)) != NULL) {
				if (ent->d_name[0] != '.') {
					(*files_names)[count] = (char*) malloc(1 + sizeof(char) * (strlen(paths[i]) + strlen(ent->d_name)));
					strcpy((*files_names)[count], paths[i]);
					strcat((*files_names)[count], ent->d_name);
					count++;
				}
			}
			closedir(dir);
		} else {
			perror("could not open directory\n");
			exit(1);
		}

	}


}

void calclReadFile(char * fileName, char ** programBuffer, size_t * program_size) {

	FILE * file = fopen(fileName, "r");
	if (file == NULL) {
		char * error = "Error while opening the file: ";
		char * errorMessage = (char*) malloc(strlen(error) + strlen(fileName) + 1);
		strcpy(errorMessage, error);
		strcat(errorMessage, fileName);
		strcat(errorMessage, '\0');
		perror(errorMessage);
		exit(EXIT_FAILURE);
	}
	long fileSize;
	fseek(file, 0, SEEK_END);
	fileSize = ftell(file);
	rewind(file);
	(*programBuffer) = (char*) malloc(fileSize * sizeof(char));
	fread(*programBuffer, sizeof(char), fileSize, file);
	fclose(file);
	*program_size = fileSize;

}

CALCLprogram calclGetProgramFromFiles(char** filesNames, unsigned filesNum, char * compilerOptions, CALCLcontext context, CALCLdevice * devices, CALCLuint num_devices) {

	char ** programBuffers = (char**) malloc(sizeof(char*) * filesNum);
	size_t * program_size = (size_t*) malloc(sizeof(size_t) * filesNum);

	unsigned i;
	for (i = 0; i < filesNum; i++)
		calclReadFile(filesNames[i], &programBuffers[i], &program_size[i]);

	//loading & building program
	CALCLprogram program;
	CALCLint err;
	program = clCreateProgramWithSource(context, filesNum, (const char**) programBuffers, program_size, &err);
	calclHandleError(err);
	err = clBuildProgram(program, num_devices, devices, compilerOptions, NULL, NULL);
//	printf("BUILDING\n");

//print error log
	if (err < 0) {
		for (i = 0; i < num_devices; i++) {
			size_t log_size;
			clGetProgramBuildInfo(program, devices[i], CL_PROGRAM_BUILD_LOG, 0,
			NULL, &log_size);
			//char program_log[log_size];

			char* program_log = (char*) malloc(sizeof(char) * log_size);

			clGetProgramBuildInfo(program, devices[i], CL_PROGRAM_BUILD_LOG, log_size + 1, program_log, &log_size);
			char device_name[100];
			clGetDeviceInfo(devices[i], CL_DEVICE_NAME, sizeof(device_name), device_name, NULL);
			printf("BUILDING KERNEL ON %s\n", device_name);
			printf("%s", (calclGetErrorString(err)));
			printf("%s", program_log);
			free(program_log);
		}
		calclHandleError(err);
	}

	return program;
}

CALCLkernel calclGetKernelFromProgram(CALCLprogram * program, char * kernelName) {
	//kernel creation
	CALCLint err;
	CALCLkernel kernel = clCreateKernel(*program, kernelName, &err);
	calclHandleError(err);
	return kernel;
}

void calclGetPlatforms(CALCLplatform ** platforms, CALCLuint * platformNum) {
	CALCLuint err;
	err = clGetPlatformIDs(1, NULL, platformNum);
	calclHandleError(err);
	*platforms = (CALCLplatform*) malloc(sizeof(CALCLplatform) * (*platformNum));
	err = clGetPlatformIDs(*platformNum, *platforms, NULL);
	calclHandleError(err);
}

void calclGetPlatformByVendor(CALCLplatform * platform, char* vendor) {

	CALCLuint err;
	CALCLuint size;
	CALCLplatform * platforms;
	err = clGetPlatformIDs(1, NULL, &size);
	calclHandleError(err);
	platforms = (CALCLplatform*) malloc(sizeof(CALCLplatform) * size);
	err = clGetPlatformIDs(size, platforms, NULL);
	calclHandleError(err);

	unsigned int i;

	for (i = 0; i < size; i++)
		if (strstr(calclGetPlatformVendor(platforms[i]), vendor) != NULL) {
			*platform = platforms[i];
			return;
		}

	printf("No %s platform found", vendor);
	calclHandleError(CL_INVALID_PLATFORM);
}

void calclGetPlatformNVIDIA(CALCLplatform * platform) {
	calclGetPlatformByVendor(platform, "NVIDIA");
}
void calclGetPlatformINTEL(CALCLplatform * platform) {
	calclGetPlatformByVendor(platform, "Intel");
}
void calclGetPlatformAMD(CALCLplatform * platform) {
	calclGetPlatformByVendor(platform, "AMD");
}

char* calclGetPlatformName(CALCLplatform platform) {
	size_t size = 0;
	CALCLint err = 0;
	err = clGetPlatformInfo(platform, CL_PLATFORM_NAME, 0, NULL, &size);
	calclHandleError(err);
	char* tmp = (char*) malloc(size * sizeof(char));
	err = clGetPlatformInfo(platform, CL_PLATFORM_NAME, sizeof(char) * size, tmp, NULL);
	calclHandleError(err);
	return tmp;
}

char* calclGetPlatformVendor(CALCLplatform platform) {
	size_t size = 0;
	CALCLint err = 0;
	err = clGetPlatformInfo(platform, CL_PLATFORM_VENDOR, 0, NULL, &size);
	calclHandleError(err);
	char* tmp = (char*) malloc(size * sizeof(char));
	err = clGetPlatformInfo(platform, CL_PLATFORM_VENDOR, sizeof(char) * size, tmp, NULL);
	calclHandleError(err);
	return tmp;
}

char* calclGetPlatformProfile(CALCLplatform platform) {
	size_t size = 0;
	CALCLint err = 0;
	err = clGetPlatformInfo(platform, CL_PLATFORM_PROFILE, 0, NULL, &size);
	calclHandleError(err);
	char* tmp = (char*) malloc(size * sizeof(char));
	err = clGetPlatformInfo(platform, CL_PLATFORM_PROFILE, sizeof(char) * size, tmp, NULL);
	calclHandleError(err);
	return tmp;
}

char* calclGetPlatformVersion(CALCLplatform platform) {
	size_t size = 0;
	CALCLint err = 0;
	err = clGetPlatformInfo(platform, CL_PLATFORM_VERSION, 0, NULL, &size);
	calclHandleError(err);
	char* tmp = (char*) malloc(size * sizeof(char));
	err = clGetPlatformInfo(platform, CL_PLATFORM_VERSION, sizeof(char) * size, tmp, NULL);
	calclHandleError(err);
	return tmp;
}

char* calclGetPlatformExtensions(CALCLplatform platform) {
	size_t size = 0;
	CALCLint err = 0;
	err = clGetPlatformInfo(platform, CL_PLATFORM_EXTENSIONS, 0, NULL, &size);
	calclHandleError(err);
	char* tmp = (char*) malloc(size * sizeof(char));
	err = clGetPlatformInfo(platform, CL_PLATFORM_EXTENSIONS, sizeof(char) * size, tmp, NULL);
	calclHandleError(err);
	return tmp;
}

void calclPrintAllPlatformInfo(CALCLplatform platform) {
	printf("PLATFORM  Name: %s\n	Vendor: %s\n	Version: %s\n	Profile: %s\n	Extentions: %s\n\n", calclGetPlatformName(platform), calclGetPlatformVendor(platform), calclGetPlatformVersion(platform), calclGetPlatformProfile(platform), calclGetPlatformExtensions(platform));
}

void calclGetDevices(CALCLplatform plat, CALCLdeviceType type, CALCLdevice ** devices, CALCLuint * size) {
	CALCLint err = 0;
	err = clGetDeviceIDs(plat, type, 1, NULL, size);
	calclHandleError(err);
	*devices = (CALCLdevice*) malloc(sizeof(CALCLdevice) * (*size));
	err = clGetDeviceIDs(plat, type, *size, *devices, NULL);
	calclHandleError(err);
}

void calclGetDevicesByName(CALCLplatform plat, CALCLdevice ** devices, CALCLuint * size, char* name) {
	CALCLdeviceType type = CL_DEVICE_TYPE_ALL;
	CALCLdevice * all_devices;
	CALCLuint all_size;
	*size = 0;
	calclGetDevices(plat, type, &all_devices, &all_size);

	unsigned int i;
	for (i = 0; i < all_size; i++)
		if (strstr(calclGetDeviceName(all_devices[i]), name) != NULL)
			(*size)++;
	int k = 0;
	*devices = (CALCLdevice*) malloc(sizeof(CALCLdevice) * (*size));
	for (i = 0; i < all_size; i++)
		if (strstr(calclGetDeviceName(all_devices[i]), name) != NULL) {
			*devices[k] = all_devices[i];
			k++;
		}

}

char* calclGetDeviceName(CALCLdevice device) {
	size_t size = 0;
	CALCLint err = 0;
	err = clGetDeviceInfo(device, CL_DEVICE_NAME, 0, NULL, &size);
	calclHandleError(err);
	char* tmp = (char*) malloc(size * sizeof(char));
	err = clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(char) * size, tmp, NULL);
	calclHandleError(err);
	return tmp;
}

char* calclGetDeviceVendor(CALCLdevice device) {
	size_t size = 0;
	CALCLint err = 0;
	err = clGetDeviceInfo(device, CL_DEVICE_VENDOR, 0, NULL, &size);
	calclHandleError(err);
	char* tmp = (char*) malloc(size * sizeof(char));
	err = clGetDeviceInfo(device, CL_DEVICE_VENDOR, sizeof(char) * size, tmp, NULL);
	calclHandleError(err);
	return tmp;
}

char* calclGetDeviceExtensions(CALCLdevice device) {
	size_t size = 0;
	CALCLint err = 0;
	err = clGetDeviceInfo(device, CL_DEVICE_EXTENSIONS, 0, NULL, &size);
	calclHandleError(err);
	char* tmp = (char*) malloc(size * sizeof(char));
	err = clGetDeviceInfo(device, CL_DEVICE_EXTENSIONS, sizeof(char) * size, tmp, NULL);
	calclHandleError(err);
	return tmp;
}

CALCLulong calclGetDeviceGlobalMemSize(CALCLdevice device) {
	CALCLint err = 0;
	CALCLulong tmp;
	err = clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(CALCLulong), &tmp, NULL);
	calclHandleError(err);
	return tmp;
}

CALCLuint calclGetDeviceMaxWorkItemDimensions(CALCLdevice device) {
	CALCLint err = 0;
	CALCLuint tmp;
	err = clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(CALCLuint), &tmp, NULL);
	calclHandleError(err);
	return tmp;
}

void calclPrintAllDeviceInfo(CALCLdevice device) {

	printf("--------DEVICE------- \n  Name: %s\n	Vendor: %s\nExtentions: %s \nGlobal Memory: %lu bytes\n MaxDim: %u \n--------------------- \n", calclGetDeviceName(device), calclGetDeviceVendor(device), calclGetDeviceExtensions(device), calclGetDeviceGlobalMemSize(device), calclGetDeviceMaxWorkItemDimensions);
}

void calclHandleError(CALCLint err) {
	if (err < 0) {
		perror(calclGetErrorString(err));
		exit(1);
	}
}

void calclPrintAllPlatformAndDevices(CALOpenCL * opencl){
	unsigned int i;
	int j;
	for (i = 0; i < opencl->num_platforms; i++) {

		        printf("Platform: %d \n", i);
		        calclPrintAllPlatformInfo(opencl->platforms[i]);
		        // for each device print critical attributes
		        for (j = 0; j < opencl->num_platforms_devices[i]; j++) {
		        	char* value;
		        	size_t valueSize;
		            // print device name
		            clGetDeviceInfo(opencl->devices[i][j], CL_DEVICE_NAME, 0, NULL, &valueSize);
		            value = (char*) malloc(valueSize);
		            clGetDeviceInfo(opencl->devices[i][j], CL_DEVICE_NAME, valueSize, value, NULL);
		            printf("%d. Device: %s \n", j, value);
		            free(value);

		        }
		    }
}

void calclGetPlatformAndDeviceFromStandardInput(CALOpenCL * opencl,CALCLdevice * device){
	calclPrintAllPlatformAndDevices(opencl);
	CALCLuint num_platform;
	CALCLuint num_device;
	char line[256];
	int i;
	printf("Insert platform :\n" );
	if (fgets(line, sizeof(line), stdin)) {
	    if (1 == sscanf(line, "%d", &i)) {
	    	num_platform=i;
	    }
	}
	printf("Insert device : \n" );
	int j;
	if (fgets(line, sizeof(line), stdin)) {
		    if (1 == sscanf(line, "%d", &j)) {
		    	num_device=j;
		    }
	}
	*device= calclGetDevice(opencl, num_platform, num_device);
}


const char * calclGetErrorString(CALCLint err) {
	switch (err) {
	case 0:
		return "CL_SUCCESS";
	case -1:
		return "CL_DEVICE_NOT_FOUND";
	case -2:
		return "CL_DEVICE_NOT_AVAILABLE";
	case -3:
		return "CL_COMPILER_NOT_AVAILABLE";
	case -4:
		return "CALCLmem_OBJECT_ALLOCATION_FAILURE";
	case -5:
		return "CL_OUT_OF_RESOURCES";
	case -6:
		return "CL_OUT_OF_HOST_MEMORY";
	case -7:
		return "CL_PROFILING_INFO_NOT_AVAILABLE";
	case -8:
		return "CL_MEM_COPY_OVERLAP";
	case -9:
		return "CL_IMAGE_FORMAT_MISMATCH";
	case -10:
		return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
	case -11:
		return "CL_BUILD_PROGRAM_FAILURE";
	case -12:
		return "CL_MAP_FAILURE";
	case -30:
		return "CL_INVALID_VALUE";
	case -31:
		return "CL_INVALID_DEVICE_TYPE";
	case -32:
		return "CL_INVALID_PLATFORM";
	case -33:
		return "CL_INVALID_DEVICE";
	case -34:
		return "CL_INVALID_CONTEXT";
	case -35:
		return "CL_INVALID_QUEUE_PROPERTIES";
	case -36:
		return "CL_INVALID_COMMAND_QUEUE";
	case -37:
		return "CL_INVALID_HOST_PTR";
	case -38:
		return "CL_INVALID_MEM_OBJECT";
	case -39:
		return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
	case -40:
		return "CL_INVALID_IMAGE_SIZE";
	case -41:
		return "CL_INVALID_SAMPLER";
	case -42:
		return "CL_INVALID_BINARY";
	case -43:
		return "CL_INVALID_BUILD_OPTIONS";
	case -44:
		return "CL_INVALID_PROGRAM";
	case -45:
		return "CL_INVALID_PROGRAM_EXECUTABLE";
	case -46:
		return "CL_INVALID_KERNEL_NAME";
	case -47:
		return "CL_INVALID_KERNEL_DEFINITION";
	case -48:
		return "CL_INVALID_KERNEL";
	case -49:
		return "CL_INVALID_ARG_INDEX";
	case -50:
		return "CL_INVALID_ARG_VALUE";
	case -51:
		return "CL_INVALID_ARG_SIZE";
	case -52:
		return "CL_INVALID_KERNEL_ARGS";
	case -53:
		return "CL_INVALID_WORK_DIMENSION";
	case -54:
		return "CL_INVALID_WORK_GROUP_SIZE";
	case -55:
		return "CL_INVALID_WORK_ITEM_SIZE";
	case -56:
		return "CL_INVALID_GLOBAL_OFFSET";
	case -57:
		return "CL_INVALID_EVENT_WAIT_LIST";
	case -58:
		return "CL_INVALID_EVENT";
	case -59:
		return "CL_INVALID_OPERATION";
	case -60:
		return "CL_INVALID_GL_OBJECT";
	case -61:
		return "CL_INVALID_BUFFER_SIZE";
	case -62:
		return "CL_INVALID_MIP_LEVEL";
	case -63:
		return "CL_INVALID_GLOBAL_WORK_SIZE";
	case -101:
		return "FILE_NOT_FOUND";
	default:
		return "Unknown OpenCL error";
	}
}
