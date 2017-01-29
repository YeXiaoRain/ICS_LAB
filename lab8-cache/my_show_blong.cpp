#include<stdio.h>
int main(){
	int i,j;
	int n,m;
	scanf("%d %d",&n,&m);
	for(i=0;i<n;i++){
		for(j=0;j<m;j++)
			printf("%2d",((i*m+j)/8)%32);
		printf("\n");
	}
	printf("---\n");
	
	for(i=0;i<n;i++){
		for(j=0;j<m;j++)
			printf("%2d",((i+j*n)/8)%32);
		printf("\n");
	}
	return 0;
}
