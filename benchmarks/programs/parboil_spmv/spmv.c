/* Beyond Correctness source packaging notice, 2026-10-25.
 * This is the research-adapted/amalgamated benchmark version.
 * Its upstream copyright and license notices are retained below.
 * This release adds only this notice and a line-number directive.
 * See ../../licenses/ and the benchmark catalog for notices.
 */
#line 1
/***************************************************************************
 *cr
 *cr            (C) Copyright 2007 The Board of Trustees of the
 *cr                        University of Illinois
 *cr                         All Rights Reserved
 *cr
 ***************************************************************************/

#include <endian.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdio.h>
#include <inttypes.h>

#if __BYTE_ORDER != __LITTLE_ENDIAN
# error "File I/O is not implemented for this system: wrong endianness."
#endif

void inputData(char* fName, int* len, int* depth, int* dim,int *nzcnt_len,int *pad,
               float** h_data, int** h_indices, int** h_ptr,
               int** h_perm, int** h_nzcnt)
{
  FILE* fid = fopen(fName, "rb");

  if (fid == NULL)
    {
      fprintf(stderr, "Cannot open input file\n");
      exit(-1);
    }

  fscanf(fid, "%d %d %d %d %d\n",len,depth,nzcnt_len,dim,pad);
  int _len=len[0];
  int _depth=depth[0];
  int _dim=dim[0];
  int _pad=pad[0];
  int _nzcnt_len=nzcnt_len[0];
  
  *h_data = (float *) malloc(_len * sizeof (float));
  fread (*h_data, sizeof (float), _len, fid);
  
  *h_indices = (int *) malloc(_len * sizeof (int));
  fread (*h_indices, sizeof (int), _len, fid);
  
  *h_ptr = (int *) malloc(_depth * sizeof (int));
  fread (*h_ptr, sizeof (int), _depth, fid);
  
  *h_perm = (int *) malloc(_dim * sizeof (int));
  fread (*h_perm, sizeof (int), _dim, fid);
  
  *h_nzcnt = (int *) malloc(_nzcnt_len * sizeof (int));
  fread (*h_nzcnt, sizeof (int), _nzcnt_len, fid);

  fclose (fid); 
}

void input_vec(char *fName,float *h_vec,int dim)
{
  FILE* fid = fopen(fName, "rb");
  fread (h_vec, sizeof (float), dim, fid);
  fclose(fid);
  
}

void outputData(char* fName, float *h_Ax_vector,int dim)
{
  FILE* fid = fopen(fName, "w");
  uint32_t tmp32;
  if (fid == NULL)
    {
      fprintf(stderr, "Cannot open output file\n");
      exit(-1);
    }
  tmp32 = dim;
  fwrite(&tmp32, sizeof(uint32_t), 1, fid);
  fwrite(h_Ax_vector, sizeof(float), dim, fid);

  fclose (fid);
}

/***************************************************************************
 *cr
 *cr            (C) Copyright 2010 The Board of Trustees of the
 *cr                        University of Illinois
 *cr                         All Rights Reserved
 *cr
 ***************************************************************************/


#include <stdio.h>
#include <stdlib.h>

/***************************************************************************
 *cr
 *cr            (C) Copyright 2007 The Board of Trustees of the
 *cr                        University of Illinois
 *cr                         All Rights Reserved
 *cr
 ***************************************************************************/

void inputData(char* fName, int* len, int* depth, int* dim,int *nzcnt_len,int *pad,
               float** h_data, int** h_indices, int** h_ptr,
               int** h_perm, int** h_nzcnt);

void input_vec(char* fNanme, float *h_vec,int dim);
void outputData(char* fName, float *h_Ax_vector,int dim);





int main(int argc, char** argv) {
	
	if (argc != 3) return 2;
	
	
	
	
	
	printf("CPU-based sparse matrix vector multiplication****\n");
	printf("Original version by Li-Wen Chang <lchang20@illinois.edu> and Shengzhao Wu<wu14@illinois.edu>\n");
	printf("This version maintained by Chris Rodrigues  ***********\n");
	
	//parameters declaration
	int len;
	int depth;
	int dim;
	int pad=1;
	int nzcnt_len;
	
	//host memory allocation
	//matrix
	float *h_data;
	int *h_indices;
	int *h_ptr;
	int *h_perm;
	int *h_nzcnt;
	//vector
	float *h_Ax_vector;
    float *h_x_vector;
	
	
    //load matrix from files
	//inputData(parameters->inpFiles[0], &len, &depth, &dim,&nzcnt_len,&pad,
	//    &h_data, &h_indices, &h_ptr,
	//    &h_perm, &h_nzcnt);
	int col_count;
	inputData(argv[1], &len, &depth, &dim, &nzcnt_len, &pad, &h_data, &h_indices, &h_ptr, &h_perm, &h_nzcnt);		


  h_Ax_vector=(float*)malloc(sizeof(float)*dim);
  h_x_vector=(float*)malloc(sizeof(float)*dim);
  input_vec( argv[2], h_x_vector,dim);


	
  int p, i, k;
	//main execution
	for(p=0;p<50;p++)
	{
		for (i = 0; i < dim; i++) {
		  float sum = 0.0f;
		  //int  bound = h_nzcnt[i / 32];
		  int  bound = h_nzcnt[i];
		  for(k=0;k<bound;k++ ) {
			int j = h_ptr[k] + i;
			int in = h_indices[j];

			float d = h_data[j];
			float t = h_x_vector[in];

			sum += d*t;
		  }
		  h_Ax_vector[h_perm[i]] = sum;
		}
	}	

	outputData("spmv.bin", h_Ax_vector, dim);
	
	free (h_data);
	free (h_indices);
	free (h_ptr);
	free (h_perm);
	free (h_nzcnt);
	free (h_Ax_vector);
	free (h_x_vector);

	
	

	return 0;

}
