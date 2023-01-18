/*************************************************************************
 *  Code to deal with STL files.                                         *
 *  File description found here:                                         *
 *  http://en.wikipedia.org/wiki/STL_(file_format)                       *
 *                                                                       *
 *  Ioannis Nompelis <nompelis@nobelware.com>         Created: 20130604  *
 *  Ioannis Nompelis <nompelis@nobelware.com>   Last modified: 20230117  *
 *************************************************************************/

/******************************************************************************
 Copyright (c) 2013-2023, Ioannis Nompelis
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
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>

#include "stl.h"


//
// Function to initialize an STL file-data structure
//

void inSTL_InitSTLfile( struct inSTL_s *sp )
{
    memset( sp->header, '\0', 80 );
    sp->ntri = 0;
    sp->triangles = (struct inSTLtri_s *) NULL;
}


//
// Function to open an STL file and return a file handle if successful
//

int inSTL_ProbeSTLfile( char *filename, int *itype )
#define FUNC "inSTL_ProbeSTLfile"
{
   int ierr,handle;
   char data[2*80];
   int i;

   handle = open( filename, O_RDONLY );
   if( handle == -1 ) {
      fprintf( stderr," e [%s]  Failed to open file \"%s\" for reading\n",
               FUNC,filename );
      return 1;
   }

   ierr = (int) read( handle, data, 80 );
   if( ierr < 80 ) {
      fprintf( stderr, " e [%s]  Could not read header (truncated?) \n", FUNC );
      close( handle );
      return 2;
   }

   // see if it is an STL file by checking the header
   if( strncmp(data, "solid ", 6 ) != 0 &&
       strncmp(data, " solid ",7 ) != 0 ) {
      fprintf( stderr," e [%s]  Not an STL file\n", FUNC );
      close( handle );
      return 3;
   } else {
      strncpy( &( data[80] ), data, 80 );
      for(i=80;i<2*80;++i) {
         if( data[i] == '\n' ) data[i] = '\0';
      }
      fprintf( stdout, " i [%s]  File header: \"%s\" \n", FUNC, &(data[80]) );
   }

   // see if it is an ASCII file by running through the first few bytes
   // run through the header and then through one more segment of data
   *itype = 1;                // start by assuming it is a binary file
   for(i=0;i<2*80-12;++i) {
      if( strncmp( &(data[i]), "facet normal", 12 ) == 0 ) {
         *itype = 0;
      }
      if( i == 80 ) {
         ierr = read( handle, &(data[80]), 80 );
         if( ierr < 80-12 ) {
            fprintf( stderr," i [%s]  File may be truncated \n", FUNC );
         }
      }
   }

   close( handle );

   return 0;
}
#undef FUNC


//
// Function to read STL binary data
//

int inSTL_ReadBinarySTL( char *filename, struct inSTL_s *sp, size_t isize )
#define FUNC "inSTL_ReadBinarySTL"
{
   int ierr,handle;
   unsigned int n;


   handle = open( filename, O_RDONLY );
   if( handle == -1 ) {
      fprintf( stderr," e [%s]  Failed to open file \"%s\" for reading\n",
               FUNC,filename);
      return 1;
   }

   ierr = (int) read( handle, sp->header, 80 );
   if( ierr < 80 ) {
      fprintf( stderr," e [%s]  Could not read header of file\n", FUNC );
      close( handle );
      return 2;
   }

   ierr = (int) read( handle, &(sp->ntri), sizeof(unsigned int) );
   if( ierr < (int) sizeof(unsigned int) ) {
      fprintf( stderr, " e [%s]  Could not read number of triangles \n", FUNC );
      fprintf( stderr, "          File may be corrupted/truncated\n" );
      close( handle );
      return 1;
   } else {
      fprintf( stderr, " i [%s]  File has %d triangles \n", FUNC, sp->ntri );
   }

   sp->triangles = (struct inSTLtri_s *)
             malloc( ((size_t) sp->ntri) * sizeof(struct inSTLtri_s) );
   if( sp->triangles == NULL ) {
      fprintf( stderr," e [%s]  Could not allocate space for triangles\n",FUNC);
      close( handle );
      return 2;
   }

   for(n=0;n<sp->ntri;++n) {
      ierr = (int) read( handle, &(sp->triangles[n]), isize );
      if( ierr < (int) isize ) {
         fprintf( stderr," e [%s]  Failed to read all triangles; (truncated?) \n",FUNC);
         close( handle );
         free( sp->triangles );
         sp->triangles = NULL;
         return 3;
      }
/*
printf(" facet normal %f %f %F \n",
sp->triangles[n].normal[0],
sp->triangles[n].normal[1],
sp->triangles[n].normal[2]
);
printf("     vertex1  %f %f %F \n",
sp->triangles[n].vertex1[0],
sp->triangles[n].vertex1[1],
sp->triangles[n].vertex1[2]
);
printf("     vertex1  %f %f %F \n",
sp->triangles[n].vertex2[0],
sp->triangles[n].vertex2[1],
sp->triangles[n].vertex2[2]
);
printf("     vertex1  %f %f %F \n\n",
sp->triangles[n].vertex3[0],
sp->triangles[n].vertex3[1],
sp->triangles[n].vertex3[2]
);
*/

   }

   close( handle );

   return 0;
}
#undef FUNC


//
// Function to read STL ASCII data
//

int inSTL_ReadAsciiSTL( char *filename, struct inSTL_s *sp )
#define FUNC "inSTL_ReadAsciiSTL"
{
   FILE *fp;
   unsigned int n;
   char data[100];
   char name[80];
   int ic,iend,itri=0;


   fp = fopen( filename,"r" );
   if( fp == NULL ) {
      fprintf( stderr," e [%s]  Failed to open file \"%s\" for reading\n",
               FUNC,filename);
      return 1;
   }


   fgets( data, 100, fp );
   ic = sscanf( data, "solid %s", name );
   if( ic != 1 ) {
      fprintf( stderr, " e [%s]  Could not find a valid STL header\n", FUNC );
      fclose( fp );
      return 2;
   }
   fprintf( stderr," i [%s]  Name in file: \"%s\"\n", FUNC, name );


   n = 0;
   iend = 0;
   while( iend == 0 ) {
      if( !feof( fp ) ) {
         fgets( data, 100, fp );
#ifdef _DEBUG_
         fprintf( stdout, " Read: %s", data );
#endif
         if( strstr( data, "endsolid" ) != NULL ) {
#ifdef _DEBUG_
            fprintf( stderr," i [%s]  Found \"endsolid\" in file\n", FUNC );
#endif
            iend = 1;
         }

         if( strstr( data, "facet normal" ) != NULL ) {
            itri = 6;
         }

         if( strstr( data, "outer loop" ) != NULL ) {
            itri = itri - 1;
         }

         if( strstr( data, "vertex" ) != NULL ) {
            itri = itri - 1;
         }

         if( strstr( data, "endloop" ) != NULL ) {
            itri = itri - 1;
         }

         if( strstr( data, "endfacet" ) != NULL ) {
            itri = itri - 1;
            n = n + 1;
         }

      } else {
         iend = -1;
      }
   }

   if( iend == -1 && itri == 0 ) {
       fprintf( stderr, " e [%s]  File seems to be truncated\n", FUNC );
       fclose( fp );
       return 3;
   }
   if( iend == 1 && itri == 0 ) {
       fprintf( stderr, " i [%s]  Counted %i triangles \n", FUNC, n );
   }

   // creating storage for all triangles
   sp->ntri = n;
   sp->triangles = (struct inSTLtri_s *)
             malloc( ((size_t) sp->ntri) * sizeof(struct inSTLtri_s) );
   if( sp->triangles == NULL ) {
      fprintf( stderr, " e [%s]  Could not allocate space for triangles \n", FUNC );
      fclose( fp );
      return 2;
   }

   rewind( fp );

   fgets( data, 100, fp );
   for(n=0;n<sp->ntri;++n) {
      struct inSTLtri_s *tp = &(sp->triangles[n]);

      fgets( data, 100, fp );
      sscanf(data, " facet normal %f %f %f",
        &(tp->normal[0]),
        &(tp->normal[1]),
        &(tp->normal[2]) );

      fgets( data, 100, fp );

      fgets( data, 100, fp );
      sscanf( data, " vertex %f %f %f",
        &(tp->vertex1[0]),
        &(tp->vertex1[1]),
        &(tp->vertex1[2]));

      fgets( data, 100, fp );
      sscanf( data, " vertex %f %f %f",
        &(tp->vertex2[0]),
        &(tp->vertex2[1]),
        &(tp->vertex2[2]));

      fgets( data, 100, fp );
      sscanf( data, " vertex %f %f %f",
        &(tp->vertex3[0]),
        &(tp->vertex3[1]),
        &(tp->vertex3[2]));

      fgets( data, 100, fp );
      fgets( data, 100, fp );
   }

   fclose( fp );

   return 0;
}
#undef FUNC


//
// Function to dump an STL file
//

int inSTL_DumpAsciiSTL( char *filename, struct inSTL_s *sp )
#define FUNC "inSTL_DumpAsciiSTL"
{
   FILE *fp;
   unsigned int n;


   fp = fopen( filename, "w" );
   if( fp == NULL ) {
      fprintf( stderr, " e [%s]  Could not write file: \"%s\"\n",FUNC,filename);
      return 1;
   } else {
      fprintf( stderr, " i [%s]  Writing file: \"%s\"\n", FUNC, filename );
   }

   fprintf( fp, "solid ASCII_STL_by_IN (%d triangles) \n",sp->ntri );
   for(n=0;n<sp->ntri;++n) {
      fprintf( fp, "  facet normal   %f  %f  %f \n",
           sp->triangles[n].normal[0],
           sp->triangles[n].normal[1],
           sp->triangles[n].normal[2] );
      fprintf( fp, "   outer loop\n");
      fprintf( fp, "    vertex  %f  %f  %f\n",
           sp->triangles[n].vertex1[0],
           sp->triangles[n].vertex1[1],
           sp->triangles[n].vertex1[2]);
      fprintf( fp, "    vertex  %f  %f  %f\n",
           sp->triangles[n].vertex2[0],
           sp->triangles[n].vertex2[1],
           sp->triangles[n].vertex2[2]);
      fprintf( fp, "    vertex  %f  %f  %f\n",
           sp->triangles[n].vertex3[0],
           sp->triangles[n].vertex3[1],
           sp->triangles[n].vertex3[2]);
      fprintf( fp, "   endloop\n");
      fprintf( fp, "  endfacet\n");
   }

   fprintf( fp, "endsolid ASCII_STL_by_IN \n" );

   fclose( fp );

   return 0;
}
#undef FUNC

/*
 * Function to dump a TecPlot file
 */

int inSTL_DumpAsciiSTLTecplot(char *filename, struct inSTL_s *sp)
#define FUNC "inSTL_DumpAsciiSTLTecplot"
{
   FILE *fp;
   unsigned int n;


   fp = fopen( filename, "w" );
   if( fp == NULL ) {
      fprintf( stderr, " e [%s]  Could not write file: \"%s\"\n",FUNC,filename);
      return 1;
   } else {
      fprintf( stderr, " i [%s]  Writing file: \"%s\"\n", FUNC, filename );
   }

   fprintf( fp, "variables = x y z nx ny nz \n" );
   fprintf( fp, "ZONE NODES=%d, ELEMENTS=%d, ",sp->ntri*3, sp->ntri );
   fprintf( fp, "     ZONETYPE=FETRIANGLE, DATAPACKING=POINT \n" );

   for(n=0;n<sp->ntri;++n) {
      fprintf( fp, " %f  %f  %f   %lf %lf %lf \n",
           sp->triangles[n].vertex1[0],
           sp->triangles[n].vertex1[1],
           sp->triangles[n].vertex1[2],
           sp->triangles[n].normal[0],
           sp->triangles[n].normal[1],
           sp->triangles[n].normal[2] );
      fprintf( fp, " %f  %f  %f   %lf %lf %lf \n",
           sp->triangles[n].vertex2[0],
           sp->triangles[n].vertex2[1],
           sp->triangles[n].vertex2[2],
           sp->triangles[n].normal[0],
           sp->triangles[n].normal[1],
           sp->triangles[n].normal[2] );
      fprintf( fp, " %f  %f  %f   %lf %lf %lf \n",
           sp->triangles[n].vertex3[0],
           sp->triangles[n].vertex3[1],
           sp->triangles[n].vertex3[2],
           sp->triangles[n].normal[0],
           sp->triangles[n].normal[1],
           sp->triangles[n].normal[2] );
   }
   for(n=0;n<sp->ntri;++n) {
      fprintf( fp, " %d %d %d \n", n*3+1, n*3+2, n*3+3 );
   }

   fclose( fp );

   return 0;
}
#undef FUNC


#ifdef _DRIVER_
int main() {
   int itype;
   struct inSTL_s stl;
   size_t isize = 4*3*4 + 2;

   inSTL_InitSTLfile( &stl );
   inSTL_ProbeSTLfile( "file.stl", &itype );
   if(itype == 0) (void) inSTL_ReadAsciiSTL("file.stl",&stl );
   if(itype == 1) (void) inSTL_ReadBinarySTL("file.stl",&stl, isize);
   (void) inSTL_DumpAsciiSTL("dump.stl",&stl);
   inSTL_DumpAsciiSTLTecplot("dump.dat", &stl );

   return 0;
}
#endif

