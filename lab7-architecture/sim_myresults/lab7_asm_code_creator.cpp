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
 *		mrmovl delta(%ebx), %esi # read val from (src + delta)
 *		rmmovl %esi,delta(%ecx)	# store it to (dst + delta)
 *		andl %esi, %esi			# val > 0?
 *		jg jmptag				# if so, goto jmptag
 *		iaddl $-1, %eax			# count--
 *
 * jmptag:
 * */
void print_Cycle_part(string tag,int delta,string jmptag)
{
	cout<<tag<<":"<<endl;
	cout<<"\tmrmovl "<<delta<<"(\%ebx), \%esi\t\t# read val from (src + "<<delta<<")"<<endl;
	cout<<"\trmmovl \%esi, "<<delta<<"(\%ecx) \t\t# store it to (dst + "<<delta<<")"<<endl;
	cout<<"\tandl \%esi, \%esi\t\t\t# val > 0?"<<endl;
	cout<<"\tjg "<<jmptag<<"\t\t\t# if so, goto "<<jmptag<<";"<<endl;
	cout<<"\tiaddl $-1, \%eax\t\t\t# count--"<<endl;
}
/*tag:
 *		mrmovl delta1(%ebx), %esi # read val from (src + delta1)
 *		mrmovl delta2(%ebx), %edi # read val from (src + delta2)
 *		rmmovl %esi,delta1(%ecx)	# store it to (dst + delta1)
 *		rmmovl %edi,delta2(%ecx)	# store it to (dst + delta2)
 *		andl %esi, %esi			# val > 0?
 *		jg jmptag2				# if so, goto jmptag1
 *		iaddl $-1, %eax			# count--
 * jmptag1:
 *		andl %edi, %edi			# val > 0?
 *		jg jmptag2				# if so, goto jmptag2
 *		iaddl $-1, %eax			# count--
 *
 * jmptag2:
 * */
void d_print_Cycle_part(string tag,int delta1,string jmptag1,int delta2,string jmptag2)
{
	cout<<tag<<":"<<endl;
	cout<<"\tmrmovl "<<delta1<<"(\%ebx), \%esi\t\t# read val from (src + "<<delta1<<")"<<endl;
	cout<<"\tmrmovl "<<delta2<<"(\%ebx), \%edi\t\t# read val from (src + "<<delta2<<")"<<endl;
	cout<<"\trmmovl \%esi, "<<delta1<<"(\%ecx) \t\t# store it to (dst + "<<delta1<<")"<<endl;
	cout<<"\trmmovl \%edi, "<<delta2<<"(\%ecx) \t\t# store it to (dst + "<<delta2<<")"<<endl;
	cout<<"\tandl \%esi, \%esi\t\t\t# val > 0?"<<endl;
	cout<<"\tjg "<<jmptag1<<"\t\t\t# if so, goto "<<jmptag1<<";"<<endl;
	cout<<"\tiaddl $-1, \%eax\t\t\t# count--"<<endl;
	cout<<jmptag1<<":"<<endl;
	cout<<"\tandl \%edi, \%edi\t\t\t# val > 0?"<<endl;
	cout<<"\tjg "<<jmptag2<<"\t\t\t# if so, goto "<<jmptag2<<";"<<endl;
	cout<<"\tiaddl $-1, \%eax\t\t\t# count--"<<endl;
}
/*
 * # Loop Start	
 *		rrmovl %edx, %eax				# count=len
 *		iaddl $-my_step, %edx			# len-=my_step
 *		jle switchTag					# if len<=0, goto switchTag
 *
 * for(i=0~my_step-1)
 * * print_Cycle_part(Expand i,i*4,Expand i+1)...
 *
 * Expand(my_step):		
 *		iaddl $4*my_step, %ebx					# src+=my_step
 *		iaddl $4*my_step, %ecx					# dst+=my_step
 *		iaddl $-my_step, %edx					# len-=my_step
 *		jg looptag
 * # Loop End
 * */
void print_loop_part(string switchTag,int my_step){
	cout<<"\trrmovl \%edx, \%eax\t\t\t# count=len"<<endl;
	cout<<"\tiaddl $-"<<my_step<<", \%edx\t\t\t# len-="<<my_step<<endl;
	cout<<"\tjle "<<switchTag<<"  \t\t\t# if len<=0, goto "<<switchTag<<endl;
	
	cout<<"# Loop Start"<<endl;
	int i=0;
	if(my_step%2){
		print_Cycle_part("Expand"+num2str(i),i*4,"Expand"+num2str(i+1));
		i=1;
	}
	for(;i<my_step;i+=2)
		d_print_Cycle_part("Expand"+num2str(i),i*4,"Expand"+num2str(i+1),(i+1)*4,"Expand"+num2str(i+2));
	
	cout<<"Expand"+num2str(my_step)<<":"<<endl;
	cout<<"\tiaddl $"<<4*my_step<<", \%ebx\t\t\t# src+="<<my_step<<endl;
	cout<<"\tiaddl $"<<4*my_step<<", \%ecx\t\t\t# dst+="<<my_step<<endl;
	cout<<"\tiaddl $-"<<my_step<<", \%edx\t\t\t# len-="<<my_step<<endl;
	cout<<"\tjg Expand0"<<endl;
	cout<<"# Loop End"<<endl;
}
/*
 * switchTag:
 *		iaddl $my_step, %edx				# len+=my_step
 *		addl %edx,%edx					# len*=4
 *		addl %edx,%edx
 *		mrmovl	jmpTable(%edx), %edx	# jump position=4*len+jmpTable
 *		pushl %edx						# goto jump position
 *		ret
 * 
 * for(i=my_step~2)
 * * print_Cycle_part(case i,(i-1)*4,case i-1)
 * print_Cycle_part(case1,0,Done)
 * ###################
 * #The original code#
 * ###################
 *
 * #jumptable
 * .align 4
 * jmpTable:
 * .long Done
 * .long case1
 * .long case2
 * ...
 * .long case(my_step)
 * */
void print_switch_part(string switchTag,int my_step){
	cout<<switchTag<<":"<<endl;
	cout<<"\tiaddl $"<<my_step<<", \%edx\t\t\t# len+="<<my_step<<endl;
	cout<<"\taddl \%edx,\%edx\t\t\t# len*=4"<<endl;
	cout<<"\taddl \%edx,\%edx"<<endl;
	cout<<"\tmrmovl jmpTable(\%edx), \%edx\t\t# jump position=4*len+jmpTable"<<endl;
	cout<<"\tpushl \%edx\t\t\t# goto jump position"<<endl;
	cout<<"\tret"<<endl;
	
	int i;
	for(i=my_step;i>1;i--)
		print_Cycle_part("case"+num2str(i),(i-1)*4,"case"+num2str(i-1));
	print_Cycle_part("case1",0,"Done");
	
	cout<<"###################"<<endl;
	cout<<"#The original code#"<<endl;
	cout<<"###################"<<endl;
	
	cout<<"# jumptable"<<endl;
	cout<<"\t.align 4"<<endl;
	cout<<"jmpTable:"<<endl;
	cout<<"\t.long Done"<<endl;
	for(i=1;i<=my_step;i++)
		cout<<"\t.long case"<<i<<endl;
}
void print_code(string switchTag,int my_step){
	print_loop_part(switchTag,my_step);
	print_switch_part(switchTag,my_step);
}
void work(){
	int my_step;
	cin>>my_step;
	print_code("Switch",my_step);
}
int main(){
	work();
	return 0;
}
