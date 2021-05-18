#include <iostream>
#include "math.h"
#include "stdlib.h"
#include <iomanip>
#include <cstring>

#include "grid_cpu.h"
#include "Defines.h"

#define single 77
#if PRECISION==single
#define PRECISION float
#endif
#if OUTPRECISION==single
#define OUTPRECISION float
#endif

#ifndef PRECISION
#define PRECISION double
#endif
#define PASTER(x) x ## 2
#define EVALUATOR(x) PASTER(x)
#define PRECISION2 EVALUATOR(PRECISION)

#ifndef OUTPRECISION
#define OUTPRECISION PRECISION
#endif
#define OUTPRECISION2 EVALUATOR(OUTPRECISION)


void init_gcf(PRECISION2 *gcf, size_t size) {

  for (size_t sub_x=0; sub_x<GCF_GRID; sub_x++ )
   for (size_t sub_y=0; sub_y<GCF_GRID; sub_y++ )
    for(size_t x=0; x<size; x++)
     for(size_t y=0; y<size; y++) {
       //Some nonsense GCF
       PRECISION tmp = sin(6.28*x/size/GCF_GRID)*exp(-(1.0*x*x+1.0*y*y*sub_y)/size/size/2);
       gcf[size*size*(sub_x+sub_y*GCF_GRID)+x+y*size].x = tmp*sin(1.0*x*sub_x/(y+1))+0.4;
       gcf[size*size*(sub_x+sub_y*GCF_GRID)+x+y*size].y = tmp*cos(1.0*x*sub_x/(y+1))-0.2;
       //std::cout << tmp << gcf[x+y*size].x << gcf[x+y*size].y << std::endl;
     }

}

void gridCPU_pz(OUTPRECISION2* out, PRECISION2 *in, PRECISION2 *in_vals, size_t npts, size_t img_dim, PRECISION2 *gcf, size_t gcf_dim) {
//degrid on the CPU
//  out (out) - the output image
//  in  (in)  - the input locations
//  in_vals (in) - input values
//  npts (in) - number of locations
//  img_dim (in) - dimension of the image
//  gcf (in) - the gridding convolution function
//  gcf_dim (in) - dimension of the GCF

   //Zero the output
   //offset gcf to point to the middle for cleaner code later
   gcf += GCF_DIM*(GCF_DIM-1)/2-1;
   double* out1 = (double*)out;
   double* out2 = ((double*)out)+POLARIZATIONS*img_dim*img_dim;
//#pragma acc parallel loop copyout(out[0:NPOINTS]) copyin(in[0:NPOINTS],gcf[0:GCF_GRID*GCF_GRID*GCF_DIM*GCF_DIM],img[IMG_SIZE*IMG_SIZE]) gang
//#pragma omp parallel for
   for(size_t n=0; n<npts; n++) {
      //std::cout << "in = " << in[n].x << ", " << in[n].y << std::endl;
      int sub_x = floorf(GCF_GRID*(in[n].x-floorf(in[n].x)));
      int sub_y = floorf(GCF_GRID*(in[n].y-floorf(in[n].y)));
      //std::cout << "sub = "  << sub_x << ", " << sub_y << std::endl;
      int main_x = floor(in[n].x);
      int main_y = floor(in[n].y);
      //std::cout << "main = " << main_x << ", " << main_y << std::endl;
//      #pragma acc parallel loop collapse(2) reduction(+:sum_r,sum_i) vector
//#pragma omp parallel for collapse(2) reduction(+:sum_r, sum_i)
      for (int a=GCF_DIM/2; a>-GCF_DIM/2 ;a--)
      for (int b=GCF_DIM/2; b>-GCF_DIM/2 ;b--) {
         PRECISION r2 = gcf[GCF_DIM*GCF_DIM*(GCF_GRID*sub_y+sub_x) +
                        GCF_DIM*b+a].x;
         PRECISION i2 = gcf[GCF_DIM*GCF_DIM*(GCF_GRID*sub_y+sub_x) +
                        GCF_DIM*b+a].y;
         PRECISION r1, i1;
         if (main_x+a < 0 || main_y+b < 0 ||
             main_x+a >= IMG_SIZE  || main_y+b >= IMG_SIZE) {
         } else {
            for (int p=0;p< POLARIZATIONS;p++) {
               r1 = in_vals[POLARIZATIONS*n+p].x;
               i1 = in_vals[POLARIZATIONS*n+p].y;
               out1[main_x+a+IMG_SIZE*(main_y+b)+p*IMG_SIZE*IMG_SIZE] += r1*r2-i1*i2;
               out2[main_x+a+IMG_SIZE*(main_y+b)+p*IMG_SIZE*IMG_SIZE] += r1*i2+r2*i1;
            }
         }
      }
      //std::cout << "val = " << out[n].r << "+ i" << out[n].i << std::endl;
   }
   gcf -= GCF_DIM*(GCF_DIM-1)/2-1;
}

template <class T,class Thalf>
int w_comp_main(const void* A, const void* B) {
   Thalf quota, rema, quotb, remb;
   rema = modf((*((T*)A)).x, &quota);
   remb = modf((*((T*)B)).x, &quotb);
   if (quota > quotb) return 1;
   if (quota < quotb) return -1;
   else {
     rema = modf((*((T*)A)).y, &quota);
     remb = modf((*((T*)B)).y, &quotb);
     if (quota > quotb) return 1;
     if (quota < quotb) return -1;
     else return 0;
   }
   return 0;
}

template <class T,class Thalf>
int w_comp_sub(const void* A, const void* B) {
   Thalf quota, rema, quotb, remb;
   rema = modf((*((T*)A)).x, &quota);
   remb = modf((*((T*)B)).x, &quotb);
   int sub_xa = (int) (GCF_GRID*rema);
   int sub_xb = (int) (GCF_GRID*remb);
   rema = modf((*((T*)A)).y, &quota);
   remb = modf((*((T*)B)).y, &quotb);
   int suba = (int) (GCF_GRID*rema) + GCF_GRID*sub_xa;
   int subb = (int) (GCF_GRID*remb) + GCF_GRID*sub_xb;
   if (suba > subb) return 1;
   if (suba < subb) return -1;
   return 0;
}

template <class T,class Thalf>
int w_comp_full(const void* A, const void* B) {
   int result = w_comp_sub<T,Thalf>(A,B);
   if (0==result) return w_comp_main<T,Thalf>(A,B);
   else return result;
}

#if 0
struct comp_grid {
   int blockgrid, blocksize;
   public:
   comp_grid(int img_dim, int gcf_dim) {
      blocksize = gcf_dim/2;
      blockgrid = img_dim/blocksize;
   }
   int __cdecl operator () (const void* A, const void* B) const {
      int gridxa = (*(int2*)A).x/GCF_GRID;
      int gridxb = (*(int2*)B).x/GCF_GRID;
      int gridya = (*(int2*)A).y/GCF_GRID;
      int gridyb = (*(int2*)B).y/GCF_GRID;
      if (gridya > gridyb) return 1;
      if (gridya < gridyb) return -1;
      if (gridxa > gridxb) return 1;
      if (gridxa < gridxb) return  -1;
      int suba = GCF_GRID*((*(int2*)A).x%GCF_GRID) + (*(int2*)A).y%GCF_GRID;
      int subb = GCF_GRID*((*(int2*)B).x%GCF_GRID) + (*(int2*)B).y%GCF_GRID;
      if (suba > subb) return 1;
      if (suba < subb) return -1;
      return  0;
   }
};
#else
template <class T, class Thalf>
int comp_grid (const void* A, const void* B) {
      int blocksize = GCF_DIM/2;
      int mainxa = floorf((*(T*)A).x);
      int mainxb = floorf((*(T*)B).x);
      int mainya = floorf((*(T*)A).y);
      int mainyb = floorf((*(T*)B).y);
      int gridxa = mainxa/blocksize;
      int gridxb = mainxb/blocksize;
      int gridya = mainya/blocksize;
      int gridyb = mainyb/blocksize;
      if (gridya*(IMG_SIZE+blocksize-1)/blocksize+gridxa >
          gridyb*(IMG_SIZE+blocksize-1)/blocksize+gridxb) return 1;
      if (gridya*(IMG_SIZE+blocksize-1)/blocksize+gridxa <
          gridyb*(IMG_SIZE+blocksize-1)/blocksize+gridxb) return -1;
      Thalf suba = GCF_GRID*((*(T*)A).x-mainxa) + (*(T*)A).y-mainya;
      Thalf subb = GCF_GRID*((*(T*)B).x-mainxb) + (*(T*)B).y-mainyb;
      if (suba > subb) return 1;
      if (suba < subb) return -1;
      return  0;
}
#endif


int main(int argc, char** argv) {

   int data_to_be_processed = sizeof(PRECISION2)*(NPOINTS*(1 + POLARIZATIONS) + 64*GCF_DIM*GCF_DIM);
   if(argc == 2 && !strcmp(argv[1], "-d")) {
       std::cout << data_to_be_processed;
       return 0;
   }

   OUTPRECISION2* out = (OUTPRECISION2*) malloc(sizeof(OUTPRECISION2)*(IMG_SIZE*IMG_SIZE+2*IMG_SIZE*GCF_DIM+2*GCF_DIM)*POLARIZATIONS);
   PRECISION2* in = (PRECISION2*) malloc(sizeof(PRECISION2)*NPOINTS);
   PRECISION2* in_vals = (PRECISION2*) malloc(sizeof(PRECISION2)*NPOINTS*POLARIZATIONS);

   PRECISION2 *gcf = (PRECISION2*) malloc(64*GCF_DIM*GCF_DIM*sizeof(PRECISION2));

   int npts=NPOINTS;

//    printf("*** CPU Gridding ***\n");
// #ifdef __GATHER
//    printf("   Gather strategy\n");
// #else
//    #ifdef __MOVING_WINDOW
//       printf("   Moving Window strategy\n");
//    #else
//       printf("   Simple scatter strategy\n");
//    #endif
// #endif

// #if PRECISION==double
//    printf("   Double precision\n");
// #else
//    printf("   Single precision\n");
// #endif
//    printf("   Image size %dx%d\n", IMG_SIZE, IMG_SIZE);
//    printf("   GCF size %dx%d\n", GCF_DIM, GCF_DIM);
//    printf("   %d polarizations\n", POLARIZATIONS);
//    printf("   %d visibilities\n", npts);
//    printf("   Subgrid: 1/%d\n", GCF_GRID);


   init_gcf(gcf, GCF_DIM);

   srand(1541617);
   for(size_t n=0; n<npts; n++) {
      in[n].x = ((float)rand())/RAND_MAX*IMG_SIZE;
      in[n].y = ((float)rand())/RAND_MAX*IMG_SIZE;
      for (int p=0;p<POLARIZATIONS;p++) {
         in_vals[POLARIZATIONS*n+p].x = ((float)rand())/RAND_MAX;
         in_vals[POLARIZATIONS*n+p].y = ((float)rand())/RAND_MAX;
      }
   }


   for (int x=0;x<IMG_SIZE*GCF_DIM*POLARIZATIONS+GCF_DIM*POLARIZATIONS;x++) {
      out[x].x=0.0;
      out[x].y=0.0;
      out[x+(IMG_SIZE*IMG_SIZE+IMG_SIZE*GCF_DIM+GCF_DIM)*POLARIZATIONS].x = 0.0;
      out[x+(IMG_SIZE*IMG_SIZE+IMG_SIZE*GCF_DIM+GCF_DIM)*POLARIZATIONS].y = 0.0;
   }

#ifdef __GATHER
   std::qsort(in, npts, sizeof(PRECISION2), comp_grid<PRECISION2,PRECISION>);
#else
#ifdef __MOVING_WINDOW
   std::qsort(in, npts, sizeof(PRECISION2), w_comp_main<PRECISION2,PRECISION>);
#else
   std::qsort(in, npts, sizeof(PRECISION2), w_comp_sub<PRECISION2,PRECISION>);
#endif
#endif

   // std::cout << "Computing on CPU..." << std::endl;
   OUTPRECISION2 *out_cpu=(OUTPRECISION2*)malloc(sizeof(OUTPRECISION2)*(IMG_SIZE*IMG_SIZE+2*IMG_SIZE*GCF_DIM+2*GCF_DIM)*POLARIZATIONS);
   memset(out_cpu, 0, sizeof(OUTPRECISION2)*(IMG_SIZE*IMG_SIZE+2*IMG_SIZE*GCF_DIM+2*GCF_DIM)*POLARIZATIONS);

   struct timespec begin_cpu, end_cpu;
   clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &begin_cpu);

   gridCPU_pz(out_cpu+IMG_SIZE*GCF_DIM+GCF_DIM,in,in_vals,npts,IMG_SIZE,gcf,GCF_DIM);
   //gridCPU(out+IMG_SIZE*GCF_DIM+GCF_DIM,in,in_vals,npts,IMG_SIZE,gcf,GCF_DIM);

   clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_cpu);
   long seconds = end_cpu.tv_sec - begin_cpu.tv_sec;
   long nanoseconds = end_cpu.tv_nsec - begin_cpu.tv_nsec;
   double elapsed = seconds + nanoseconds*1e-9;

   std::cout << " " << std::setprecision(10) << elapsed;

   free(out);out=NULL;
   free(in);in=NULL;
   free(in_vals);in_vals=NULL;
   free(gcf);gcf=NULL;
}
