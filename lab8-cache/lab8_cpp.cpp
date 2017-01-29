#include<iostream>
#include<stdio.h>
#include<string>
using namespace std;
typedef unsigned long long ull;
class lab8_cache{	
	private:
		int Bits;	
		int s,E,b;
		bool show_detail;
		int hit_times,miss_times,eviction_times;
		ull **datasaver;
		void divide_code(ull val,ull * tag,ull * sets, ull * size);
		void clean_datasaver();
		int do_code(ull tag,ull sets);
	public:
		lab8_cache(int systembits,int sets,int setE,int setb);
		~lab8_cache();
		void execute_code(char optype,ull x_code,int rdsz);//readsize
		void set_show_detail(bool tf);
		void show_ans();
};

void lab8_cache::divide_code(ull val,ull * tag,ull * sets,ull * size){//{{{
	*tag = val>>(s+b);
	*sets = (val>>b) & ((1<<s)-1);
	*size = val & ((1<<b)-1);
	return ;
}//}}}

void lab8_cache::clean_datasaver(){//{{{
	int i,j;
	int maxi=(1<<s),emptyval=1<<(Bits-s-b);
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

int lab8_cache::do_code(ull tag,ull sets){//{{{
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

lab8_cache::lab8_cache(int systembits,int sets,int setE,int setb)//{{{
:Bits(systembits),s(sets),E(setE),b(setb),show_detail(false)
{
	int i,maxi=1<<s;
	datasaver = new ull * [1<<s];
	for(i=0;i<maxi;i++)
		datasaver[i] = new ull [E];
	this->clean_datasaver();
	
}//}}}

lab8_cache::~lab8_cache(){//{{{
	int i;
	for(i=0;i<(1<<s);i++)
		delete [] datasaver[i];
	delete [] datasaver;
}///}}}

void lab8_cache::execute_code(char optype,ull x_code,int rdsz){//{{{
	ull code_Tg,code_st,code_sz;
	this->divide_code(x_code,&code_Tg,&code_st,&code_sz);
	int result=this->do_code(code_Tg,code_st);
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
			cout<<" hit";
		if(result >> 1)
			cout<<" miss";
		if(result & 1)
			cout<<" eviction";
		if(optype=='M')
			cout<<" hit";
		cout<<endl;
	}

	return ;
}//}}}

void lab8_cache::show_ans(){//{{{
	//printSummary(hit_times,miss_times,eviction_times);
	cout<<"hit:"<<hit_times<<" miss:"<<miss_times<<" eviction:"<<eviction_times<<endl;
	return ;
}//}}}

void lab8_cache::set_show_detail(bool tf){//{{{
	show_detail=tf;
	return ;
}//}}}

int main()
{
	ull num;
	int readsize;
	char instruction;
	scanf(" %c %llx,%d",&instruction,&num,&readsize);
	lab8_cache test(64,4,1,4);
	test.set_show_detail(true);
	test.execute_code('L',0x10,1);
	test.execute_code('M',0x20,1);
	test.execute_code('L',0x22,1);
	test.execute_code('S',0x18,1);
	test.execute_code('L',0x110,1);
	test.execute_code('L',0x210,1);
	test.execute_code('M',0x12,1);
	test.show_ans();
	return 0;
}
