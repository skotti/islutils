
#define N 1024

int A[N][N];
int B[N][N];
int C[N][N];

int main(){
  #pragma scop
  for(int i=0; i<N; ++i)
    for(int j=0; j<N; ++j) {
      //C[i][j] = 0;
      for(int k=0; k<N; ++k)
        C[i+1][j] += B[k][j] * A[i][k];
    }
  #pragma endscop
}
