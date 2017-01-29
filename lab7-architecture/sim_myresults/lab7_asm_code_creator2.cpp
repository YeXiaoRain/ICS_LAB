//
#include<iostream>
#include<string>
#include<sstream>
using namespace std;
string num2str(int num)
{
	stringstream ss;
	ss<<num;
	return ss.str();
}
/*tag:
 *		mrmovl delta(%ebx), %edi # read val from (src + delta)
 *		rmmovl %edi,delta(%ecx)	# store it to (dst + delta)
 *		andl %edi, %edi			# val > 0?
 *		jg jmptag				# if so, goto jmptag
 *		iaddl $-1, %eax			# count--
 *
 * jmptag:
 * */
void print_Cycle_part(string tag,int delta,string jmptag)
{
	cout<<tag<<":"<<endl;
	cout<<"\tmrmovl "<<delta<<"(\%ebx), \%edi\t\t# read val from (src + "<<delta<<")"<<endl;
	cout<<"\trmmovl \%edi, "<<delta<<"(\%ecx) \t\t# store it to (dst + "<<delta<<")"<<endl;
	cout<<"\tandl \%edi, \%edi\t\t\t# val > 0?"<<endl;
	cout<<"\tjg "<<jmptag<<"\t\t\t# if so, goto "<<jmptag<<";"<<endl;
	cout<<"\tiaddl $-1, \%eax\t\t\t# count--"<<endl;
}
/*
 *		rrmovl %edx, %eax				# count=len
 *
 *		rrmovl %edx, %esi				# for the mod , der = len
 *		irmovl $0xf, %edi				# mod 16
 *		andl %edi, %esi					# get mod, der = len % 16
 *		addl %esi, %esi					# der *= 4
 *		addl %esi, %esi	
 *		mrmovl	jmpTable(%esi), %edi	# jump position = der + jmpTable
 *		pushl %edi						# goto jump position
 *		ret	
 */
void print_start(){
	cout<<"\trrmovl \%edx, \%eax\t\t\t# count=len"<<endl;
	cout<<"\trrmovl \%edx, \%esi\t\t\t# for the mod , der = len"<<endl;
	cout<<"\tirmovl $0xf, \%edi\t\t\t# mod 16"<<endl;
	cout<<"\tandl \%edi, \%esi\t\t\t# get mod, der = len % 16"<<endl;
	cout<<"\taddl \%esi, \%esi\t\t\t# der *= 4"<<endl;
	cout<<"\taddl \%esi, \%esi"<<endl;
	cout<<"\tmrmovl	jmpTable(\%esi), \%edi\t# jump position = der + jmpTable"<<endl;
	cout<<"\tpushl \%edi\t\t\t# goto jump position"<<endl;
	cout<<"\tret"<<endl;
}
/* LoopStart:
 *		addl %esi, %ebx				# ebx += der
 *		addl %esi, %ecx				# ecx += der
 *		irmovl $64, %esi			# der = 4 * 16
 *
 * print_Cycle_part(Left i,i*4,Left i+1)...
 *
 * LoopCycle:
 *		iaddl $-16 , %edx			# len -= 16
 *		jl LoopOut					# if len < 0 , end loop
 *		jmp LoopStart
 * LoopOut:		
 * # Loop End
 * */
void print_body(int my_step){

	cout<<"LoopStart:"<<endl;
	cout<<"\taddl \%esi, \%ebx\t\t\t# ebx += der"<<endl;
	cout<<"\taddl \%esi, \%ecx\t\t\t# ecx += der"<<endl;
	cout<<"\tirmovl $"<<4*my_step<<", \%esi\t\t\t# der = 4 * "<<my_step<<endl<<endl;
	
	int i;
	for(i=my_step;i>1;i--)
		print_Cycle_part("Left"+num2str(i),(i-1)*4,"Left"+num2str(i-1));
	print_Cycle_part("Left1",0,"LoopCycle");
	
	cout<<"LoopCycle:"<<endl;
	cout<<"\tiaddl $-"<<my_step<<" , \%edx\t\t\t# len -= "<<my_step<<endl;
	cout<<"\tjl LoopOut\t\t\t# if len < 0 , end loop"<<endl;
	cout<<"\tjmp LoopStart"<<endl<<endl;
	cout<<"LoopOut:"<<endl;
	cout<<"# Loop End"<<endl;
}
/*
 * ###################
 * #The original code#
 * ###################
 *
 * #jumptable
 * .align 4
 * jmpTable:
 * .long Done
 * .long Left1
 * .long Left2
 * .long Left3
 * ...
 * .long Left(my_step)
 * */
void print_table(int size){
	cout<<"###################"<<endl;
	cout<<"#The original code#"<<endl;
	cout<<"###################"<<endl;
	
	cout<<"# jumptable"<<endl;
	cout<<"\t.align 4"<<endl;
	cout<<"jmpTable:"<<endl;
	cout<<"\t.long LoopCycle"<<endl;
	int i;
	for(i=1;i<size;i++)
		cout<<"\t.long Left"<<i<<endl;
}
void work(){
	print_start();
	print_body(16);
	print_table(16);
}
int main(){
	work();
	return 0;
}
