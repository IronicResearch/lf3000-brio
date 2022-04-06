#ifndef LF_BRIO_DATASETLOADER_H
#define LF_BRIO_DATASETLOADER_H

#include <string>	
#include <SystemTypes.h>
#include <CoreTypes.h>	

#include "Wrappers.h"

typedef struct tagAppRsrcDataSet {
		tVersion	fmtVersion;	
		tVersion 	cntVersion;	
		U32 		id;
		void* 	pDataSet;
}tAppRsrcDataSet;

/****************************************************************************
 * 																			*
 * Dataset loader functions.												*
 * 		LoadDataset()	-loads the dataset into RAM, fixes up any pointers	*
 * 						 and returns a pointer to the dataset.	            *
 * **************************************************************************/

tAppRsrcDataSet* LoadDataset(const string binPath, const string relinkPath);

#endif

//eof
