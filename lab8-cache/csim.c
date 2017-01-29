/*
 * Name:YeXiaoRain
 * ID:5130379000
 */
#include "cachelab.h"
#include<malloc.h>
#include<stdio.h>
#include<stdlib.h>
#include<getopt.h>
typedef unsigned long long ull;

int Bits;	
int s,E,b;
int show_detail;
int hit_times,miss_times,eviction_times;
ull ** datasaver;

void clean_datasaver(){//{{{
	int i,j;
	int maxi=(1<<s);
	ull emptyval=1<<(Bits-s-b);
	for(i=0;i<maxi;i++){
		for(j=0;j<E;j++){
			datasaver[i][j]=emptyval;
		}
	}
	miss_times=0;
	hit_times=0;
	eviction_times=0;
	return ;
}//}}}

void init_lab8_cache(int systembits,int sets,int setE,int setb){//{{{
	Bits=systembits;
	s=sets;
	E=setE;
	b=setb;
	int i,maxi=1<<s;
	datasaver = (ull **) malloc(sizeof(ull *) * maxi);
	for(i=0;i<maxi;i++)
		datasaver[i] = (ull *)malloc(sizeof(ull) * E);
	
	clean_datasaver();
	return ;
}//}}}

void disinit_lab8_cache(){//{{{
	int i;
	for(i=0;i<(1<<s);i++)
		free(datasaver[i]);
	free(datasaver);
	return ;
}//}}}

void divide_code(ull val , ull * tag , ull * sets , ull * size){//{{{
	*tag = val>>(s+b);
	*sets = (val>>b) & ((1<<s)-1);
	*size = val & ((1<<b)-1);
	return ;
}//}}}

int do_code(ull tag,ull sets){//{{{
	int i;
	ull emptyval=1<<(Bits-s-b);
	ull temp;
	for(i=0;i<E;i++){
		temp=datasaver[sets][i];
		if(temp==tag || temp==emptyval)
			break ;
	}
	int return_code=0;
	if(i==E){//miss eviction
		return_code=3;
		i=0;
	}else if(temp==tag)//hit move
		return_code=0;
	else//miss
		return_code=2;
	for(;i<E-1;i++){
		temp=datasaver[sets][i]=datasaver[sets][i+1];	
		if(temp==emptyval)
			break;
	}
	datasaver[sets][i]=tag;
	return return_code;		

}//}}}

void execute_code(char optype,ull x_code,int rdsz){//readsize//{{{
	ull code_Tg,code_st,code_sz;
	divide_code(x_code,&code_Tg,&code_st,&code_sz);
	int result=do_code(code_Tg,code_st);
	if(!result)
		hit_times++;
	if(result >> 1)
		miss_times++;
	if(result & 1)
		eviction_times++;
	if(optype=='M')
		hit_times++;
	if(show_detail){
		printf("%c %llx,%d",optype,x_code,rdsz);
		if(!result)
			printf(" hit");
		if(result >> 1)
			printf(" miss");
		if(result & 1)
			printf(" eviction");
		if(optype=='M')
			printf(" hit");
		printf("\n");
	}
	return ;

}//}}}

void set_show_detail(int tf){//{{{
	show_detail=tf;
	return ;
}//}}}

void show_ans(){//{{{
	printSummary(hit_times,miss_times,eviction_times);
	//cout<<"hit:"<<hit_times<<" miss:"<<miss_times<<" eviction:"<<eviction_times<<endl;
	return ;
}//}}}

void read_code(char * F_PATH){//{{{
	FILE * fin = NULL;
	if((fin=fopen(F_PATH,"r"))==NULL){
		printf("cannot open this file\n");
	}
	char op;
	ull x_val;
	int readsize;
	while(fscanf(fin," %c %llx,%d",&op,&x_val,&readsize)!=EOF){
		if(op=='I')
			continue;
		execute_code(op,x_val,readsize);
	}
	fclose(fin);
	return ;
}//}}}

void show_help(){//{{{
	printf("\
Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>\n\
Options:\n\
\t-h         Print this help message.\n\
\t-v         Optional verbose flag.\n\
\t-s <num>   Number of set index bits.\n\
\t-E <num>   Number of lines per set.\n\
\t-b <num>   Number of block offset bits.\n\
\t-t <file>  Trace file.\n\
\n\
Examples:\n\
\tlinux>  ./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace\n\
\tlinux>  ./csim-ref -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
}//}}}

int main(int argc,char *argv[])
{
	int ch;
	int s_arg=-1,E_arg=-1,b_arg=-1;
	opterr=0;
	int input_index=-1;
	while((ch = getopt(argc,argv,"hvs:E:b:t:"))!=-1){
		switch(ch){
			case 'v':
				set_show_detail(1);
				break;
			case 's':
				s_arg=atoi(optarg);
				break;
			case 'E':
				E_arg=atoi(optarg);
				break;
			case 'b':
				b_arg=atoi(optarg);
				break;
			case 't':
				input_index=optind-1;
				break;
			case 'h':
			default:
				show_help();
				return 0;
				break;
		}
		//printf("optind:%d\n",optind);
		//printf("optarg:%s\n",optarg);
		//printf("opterr:%d\n",opterr);
	}
	if((s_arg | E_arg | b_arg | input_index) == -1 ){
		show_help();
		return 0;
	}
	init_lab8_cache(64,s_arg,E_arg,b_arg);
	read_code(argv[input_index]);
	show_ans();
	disinit_lab8_cache();
	return 0;
}
