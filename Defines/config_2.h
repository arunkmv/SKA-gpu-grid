#ifndef __DEFINES_H
#define __DEFINES_H

#define NPOINTS 1280000
#define GCF_DIM 128
#define IMG_SIZE 4096
#define GCF_GRID 8
//BLOCK_Y affects only MOVING_WINDOW
#ifndef BLOCK_Y
#define BLOCK_Y 8
//#define BLOCK_Y 11
#endif
//PTS and GCF_STRIPES affect only GATHER
//#define PTS 1
//#define GCF_STRIPES 1
//#define POLARIZATIONS 4
#ifndef PTS
#define PTS 1
#endif
#ifndef GCF_STRIPES
#define GCF_STRIPES 8
//#define GCF_STRIPES 4
#endif
#define POLARIZATIONS 1
//#define DEBUG1
#endif
