/***************************************************************************
 This is a demo code and a tool to convert 2D files that are intended to be
 read by AMTEC's Tecplot into STL files that can be used in other platforms.
 There are a lot of hard-wired parts in this code, so use it with care and
 ask for help!

 The code makes use of some utilities that are in a small library that I am
 building in order to handle STL files. Treat this as a "pre-release" then.

  Ioannis Nompelis <nompelis@nobelware.com>   Last modified: 20180421
 ***************************************************************************/

/******************************************************************************
 Copyright (c) 2013-2018, Ioannis Nompelis
 All rights reserved.

 Redistribution and use in source and binary forms, with or without any
 modification, are permitted provided that the following conditions are met:
 1. Redistribution of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
 2. Redistribution in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
 3. All advertising materials mentioning features or use of this software
    must display the following acknowledgement:
    "This product includes software developed by Ioannis Nompelis."
 4. Neither the name of Ioannis Nompelis and his partners/affiliates nor the
    names of other contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.
 5. Redistribution or use of source code and binary forms for profit must
    have written permission of the copyright holder.
 
 THIS SOFTWARE IS PROVIDED BY IOANNIS NOMPELIS ''AS IS'' AND ANY
 EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL IOANNIS NOMPELIS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/select.h>
#include <pthread.h>

#include "stl.h"



int read_file( int *nn, int *ne, int **icon, double **x )
{
   FILE *fp;
   char data[100];
   size_t isize;
   int n;
   double xx,yy,zz;
   int i1,i2,i3,i4;


   fp = fopen("waverider.dat", "r");

   fgets(data, 100, fp);
   printf("%s", data);
   fgets(data, 100, fp);
   printf("%s", data);
   fgets(data, 100, fp);
   printf("%s", data);
   fgets(data, 100, fp);
   printf("%s", data);

   sscanf(data, " Nodes=%d, Elements=%d, ",nn,ne);
   printf("Parsed nn=%d ne=%d \n", *nn,*ne);

   isize = (size_t) (*nn);
   *x = (double *) malloc(isize*3*sizeof(double));
   isize = (size_t) (*ne);
   *icon = (int *) malloc(isize*4*sizeof(int));

   fgets(data, 100, fp);
   printf("%s", data);
   fgets(data, 100, fp);
   printf("%s", data);

   for(n=0;n<(*nn);++n) {
      fgets(data, 100, fp);
      sscanf(data, "%lf %lf %lf\n",&xx,&yy,&zz);
   // printf("%lf %lf %lf \n",xx,yy,zz);
      (*x)[3*n+0] = xx;
      (*x)[3*n+1] = yy;
      (*x)[3*n+2] = zz;
   }

   for(n=0;n<(*ne);++n) {
      fgets(data, 100, fp);
      sscanf(data, "%d %d %d %d\n", &i1,&i2,&i3,&i4);
   // printf("%d %d %d %d \n",i1,i2,i3,i4);
      (*icon)[4*n+0] = i1;
      (*icon)[4*n+1] = i2;
      (*icon)[4*n+2] = i3;
      (*icon)[4*n+3] = i4;
   }

   fclose(fp);

   return(0);
}



int main( int argc, char *argv[] )
{
   struct my_STLfile *sf;
   int nn,ne,*icon;
   double *x;
   int n,m;


   (void) read_file( &nn, &ne, &icon, &x );

   sf = (struct my_STLfile *) malloc(sizeof(struct my_STLfile));

   sf->ntri = 2*ne;
   sf-> triangles = (struct my_STLtri *)
              malloc(((size_t) ne)*sizeof(struct my_STLtri));

   for(m=0;m<ne;++m) {
      int mm = m*2;
      double xx,yy,zz;


      n = m*4+0;
      sf->triangles[mm].vertex1[0] = x[ 3*n+0 ];
      sf->triangles[mm].vertex1[1] = x[ 3*n+1 ];
      sf->triangles[mm].vertex1[2] = x[ 3*n+2 ];

      n = m*4+1;
      sf->triangles[mm].vertex2[0] = x[ 3*n+0 ];
      sf->triangles[mm].vertex2[1] = x[ 3*n+1 ];
      sf->triangles[mm].vertex2[2] = x[ 3*n+2 ];

      n = m*4+3;
      sf->triangles[mm].vertex3[0] = x[ 3*n+0 ];
      sf->triangles[mm].vertex3[1] = x[ 3*n+1 ];
      sf->triangles[mm].vertex3[2] = x[ 3*n+2 ];

// float normal[3];      // these 12 numbers are little endian !!!!!
      sf->triangles[mm].normal[0] = 0.0;
      sf->triangles[mm].normal[1] = 0.0;
      sf->triangles[mm].normal[2] = 0.0;

      mm += 1;

      n = m*4+2;
      sf->triangles[mm].vertex1[0] = x[ 3*n+0 ];
      sf->triangles[mm].vertex1[1] = x[ 3*n+1 ];
      sf->triangles[mm].vertex1[2] = x[ 3*n+2 ];

      n = m*4+1;
      sf->triangles[mm].vertex2[0] = x[ 3*n+0 ];
      sf->triangles[mm].vertex2[1] = x[ 3*n+1 ];
      sf->triangles[mm].vertex2[2] = x[ 3*n+2 ];

      n = m*4+3;
      sf->triangles[mm].vertex3[0] = x[ 3*n+0 ];
      sf->triangles[mm].vertex3[1] = x[ 3*n+1 ];
      sf->triangles[mm].vertex3[2] = x[ 3*n+2 ];

// float normal[3];      // these 12 numbers are little endian !!!!!
      sf->triangles[mm].normal[0] = 0.0;
      sf->triangles[mm].normal[1] = 0.0;
      sf->triangles[mm].normal[2] = 0.0;
   }


   (void) inSTL_DumpAsciiSTL("waverider.stl", sf);

   return(0);
}

