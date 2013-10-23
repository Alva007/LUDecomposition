#include<stdio.h>
#include<stdlib.h>
#include<malloc.h>
#include<sys/time.h>
#include "omp.h"

long extractN(char *);//helper function to extract n
void printMatrix(double **a,int n);//helper to print matrix


void *memset(void *str,int c, size_t t);//too many warnings...so you know.
void *memcpy(void *str1,const void *str2,size_t n);

int main(argc,argv)
     int argc;
     char *argv[];
{
  int i,j,k,t;
  long n;
  int flag;
  double l1,u1;
  double **a;
  double **b;
  double **c;
  struct timeval tv;
  double start;
  double end;

  /*OpenMP related variables*/
  int nprocs=8;
  int rows=n/nprocs;
  int pid; 
  int threads;

  n=extractN(argv[1]);
  threads=extractN(argv[2]);
 
  a=(double **)malloc(sizeof(double *)*n);  
  b=(double **)malloc(sizeof(double *)*n);
  c=(double **)malloc(sizeof(double *)*n);
  
  for(i=0;i<n;i++){
    a[i]=(double *)malloc(sizeof(double)*n); 
    b[i]=(double *)malloc(sizeof(double)*n);
    c[i]=(double *)malloc(sizeof(double)*n);
   }
 
  srand((unsigned)time(NULL));

  for(i=0;i<n;i++){
    for(j=0;j<n;j++){
      a[i][j]=((rand()%10)+1);
    }
    memcpy((void *)b[i],(const void *)a[i],(size_t)(n*sizeof(double)));
  }

  gettimeofday(&tv,NULL);
  start=tv.tv_sec;

  /*Perform LU decomposition*/
#pragma omp parallel num_threads(threads) private(i,j,k,pid)
 {  
 pid=omp_get_thread_num();
 for(k=0;k<n;k++){
 
 if(k%nprocs==pid){
   for(j=k+1;j<n;j++){
      a[k][j]=a[k][j]/a[k][k];//Scaling
    }
  }   
 
#pragma omp barrier
#pragma omp for schedule(static,1)
  for(i=k+1;i<n;i++){ 
    for(j=k+1;j<n;j++){
        a[i][j]=a[i][j]-a[i][k]*a[k][j];
       } 
    }
  }//end of outer for loop
  //end of LU decomposition
}//end of parallel region 

 gettimeofday(&tv,NULL);
 end=tv.tv_sec;
 printf("Operation took %lf\n",end-start);

 /*Inplace Verification step not working...*/
  
 for(i=0;i<n;i++){
   for(j=0;j<n;j++){
     c[i][j]=0;

     // if(i<j)t=j;
     // else t=i; 
     for(k=0;k<n;k++){
       if(i>=k)l1=a[i][k];
       else l1=0;

       if(k==j)u1=1;
       else if(k<j)u1=a[k][j];//figured it out 
       else u1=0.0;
       
        c[i][j]=c[i][j]+(l1*u1);
      
     } 
   }
  }
 flag=0;
 for(i=0;i<n;i++){
   for(j=0;j<n;j++){
     if(abs(c[i][j]-b[i][j])>0.01){
         flag=1;
         break;
     }
    }
 }
 if(flag==1){
     printf("Error : Not a match\n");
 }
 else printf("Match\n");
 


 /*Time to free the memory*/
  for(i=0;i<n;i++){
    free(a[i]);
    free(b[i]);
    free(c[i]);
  }
  free(a);
  free(b);
  free(c);

  return 0;
}


/*Helper functions*/
long extractN(char *c){
  long temp=0;
   while((*c)!='\0'){
    temp=temp*10+((*c)-48);
    c=c++;
   }
  return temp;
 }

void printMatrix(double **mat,int n){
  int i,j;
  for(i=0;i<n;i++){
    for(j=0;j<n;j++){
      printf("%lf ",mat[i][j]);
    }
    printf("\n");
  }
  printf("------------------------------------------------\n");
}
