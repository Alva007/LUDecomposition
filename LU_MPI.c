#include<stdio.h>
#include<stdlib.h>
#include<sys/time.h>
#include<time.h>
#include "mpi.h"


/*Helper functions*/
void LU(double **b,int rows,int n,int my_id);
int malloc2Ddouble(double ***array, int n, int m);
int extractN(char *c);/*Helper to extract N from command line ags*/

int main(argc,argv)
int argc;
char **argv;
{
	

  MPI_Init(&argc,&argv);
  int i,j,k;
  int my_id;
	
  struct timeval tv;

   double **a;
   double **b;
   double **c;
   double **d;
   double l;
   double u;
 
   int nprocs;
   double start;
   double end;
   int rows;
   int flag=1;
   int n=extractN(argv[1]);

  // printf("Hello");
   MPI_Comm_size(MPI_COMM_WORLD,&nprocs);
   MPI_Comm_rank(MPI_COMM_WORLD,&my_id);

  rows=(n/nprocs);//chunk size

  malloc2Ddouble(&b,rows,n);//matrix to store local chunk
 // printf("HEllo2");
  if(my_id==0){
     malloc2Ddouble(&a,n, n);
     malloc2Ddouble(&c, n, n);

     srand(time(NULL));
      for(i=0;i<n;i++)  {
	for(j=0;j<n;j++){
	   a[i][j]=rand()%10+1;
	   c[i][j]=a[i][j];
	}
      }
	
   }// end of master thread's work

 // printf("hello3\n");
  MPI_Barrier(MPI_COMM_WORLD);
   gettimeofday(&tv,NULL);
   start=tv.tv_sec;

   //MPI_Scatter(&(a[0][0]),(n*rows),MPI_DOUBLE,&(b[0][0],MPI_DOUBLE,0,MPI_COMM_WORLD); 
   // not working...
 
   if(my_id==0)
   MPI_Scatter(a[0],(n*rows),MPI_DOUBLE,b[0],(n*rows),MPI_DOUBLE,0,MPI_COMM_WORLD);
   else	
   MPI_Scatter(NULL,(n*rows),MPI_DOUBLE,b[0],(n*rows),MPI_DOUBLE,0,MPI_COMM_WORLD);
 //printf("hello4\n");
   MPI_Barrier(MPI_COMM_WORLD);
   LU(b,rows,n,my_id);
   MPI_Barrier(MPI_COMM_WORLD);
 //printf("hello5\n");

   if(my_id==0)
   MPI_Gather(b[0],(n*rows),MPI_DOUBLE,c[0],(n*rows),MPI_DOUBLE,0,MPI_COMM_WORLD);
   else 
   MPI_Gather(b[0],(n*rows),MPI_DOUBLE,NULL,(n*rows),MPI_DOUBLE,0,MPI_COMM_WORLD);

   MPI_Barrier(MPI_COMM_WORLD);
   if(my_id==0){
      gettimeofday(&tv,NULL);
       end=tv.tv_sec;
     printf("Operation took %lf seconds \n",end-start);
     /*As we are not scattering or gathering "d" we dont need contiguous allocation*/
     d=(double**)malloc(n*sizeof(double*));
     for(i=0;i<n;i++) d[i]=(double*)malloc(n*sizeof(double));

      /*Inplace verification*/	
      for(i=0;i<n;i++){
       for(j=0;j<n;j++){
          d[i][j]=0;
         for(k=0;k<n;k++){
	       if(i>=k) l=c[i][k];
               else l=0;

        	if(k<j) u=c[k][j];
		else if(k==j) u=1;
	        else u=0;
		d[i][j]+=(l*u);
		}
        }
      }	
     for(i=0;i<n;i++){
      for(j=0;j<n;j++){
        if(abs(d[i][j]-a[i][j])>0.01){
            flag=0;
            break;
         }
       }
     }     
				
     if(flag==1)
     printf("Match");
     else
     printf("Erro : Not a match");
     
  }// end of book keeping of master process....

  MPI_Finalize();
  return 0;
}// end of main function 


/*Helper functions*/
void LU(double **a,int rows,int n,int my_id )
{
   int my_min=my_id*rows;
   int my_max=my_min+rows-1;
   int i1,k1,j1;
   int i,j,k;
   int root;
 
   /*buffer to store the scaled row and broadcast it*/	
   double *buffer=(double*)malloc(n*sizeof(double));
   for(k=0;k<n;k++){
     k1=k%rows;
     root=k/rows;
    /*Scaling step*/
    if(my_min<=k && k<=my_max){
	for(j=k+1;j<n;j++){
          a[k1][j]=a[k1][j]/a[k1][k];
	  buffer[j]=a[k1][j];
	}
     //memcpy((void *)buffer,(const void *)a[k1],n);
     //not working....throwing sig 11 segfault
     }
     /*Broadcast the scaled down row*/		
     MPI_Bcast(buffer,n,MPI_DOUBLE,root,MPI_COMM_WORLD);

     /*Reduction step */
     for(i=((k+1)>my_min ? (k+1) : my_min);i<=my_max;i++){
	i1=i%rows;
	 for(j=k+1;j<n;j++){
	   a[i1][j]=a[i1][j]-a[i1][k]*buffer[j];
	  }
      }
   }

}// end of LU decomposition


/*Helper function to extract N*/
int extractN(char *c){
	int temp=0;
        while((*c)!='\0'){
        temp=temp*10 + ((*c)-48);
        c=c+1;
        }
        return temp;
}

/*As suggested by Johnathan Dursi on stackoverflow for
 *contiguous allocation of memory (to be used in scatter and 
 *gather calls).
 * */
int malloc2Ddouble(double ***array, int n, int m) {

    int i;
    double *p = (double *)malloc(n*m*sizeof(double));
    if (!p) return -1;

    (*array) = (double **)malloc(n*sizeof(double*));
    if (!(*array)) {
       free(p);
       return -1;
    }

    for (i=0; i<n; i++) 
       (*array)[i] = &(p[i*m]);

    return 0;
}


