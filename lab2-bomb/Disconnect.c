#include <stdio.h> //标准输入输入出的头文件,printf和scanf都在这里了
#include <stdlib.h>  //标准库头文件，内存分配、数学运算等都在这里了
#include <string.h>
#define MAXLEN 1024 //approximately
#define INIT_TIMES 1
#define SEND_TIMES 2
const char NOP='\220';

char ori_adr[16]={"202.120.40.188"},pos_adr[16]={"127.0.0.1\0\0\0\0\0"};
char ori_sed[SEND_TIMES][7];
char ori_init[INIT_TIMES][7];

void analyse_send_msg()
{
	FILE * fp;

	char buffer[80];//approximately
	fp=popen("objdump -D bomb |grep '<send_msg>'","r");//use shell to find the send_msg
	int i;
	i=0;
	while(fgets(buffer,sizeof(buffer),fp))
	{
		int num[5];
		if(strlen(buffer)<30)continue;//approximately
		char trash[100];//approximately
		sscanf(buffer,"%s %x %x %x %x %x",trash,&num[0],&num[1],&num[2],&num[3],&num[4]);//got_the_specific_code
		sprintf(ori_sed[i++],"%c%c%c%c%c",num[0],num[1],num[2],num[3],num[4]);//save in ori_init
	}
	pclose(fp);

	fp=popen("objdump -D bomb |grep '<initialize_bomb>'","r");//use shell to find the initialize_bomb
	i=0;
	while(fgets(buffer,sizeof(buffer),fp))
	{
		int num[5];
		if(strlen(buffer)<30)continue;//approximately
		char trash[100];//approximately
		sscanf(buffer,"%s %x %x %x %x %x",trash,&num[0],&num[1],&num[2],&num[3],&num[4]);//got_the_specific_code
		sprintf(ori_init[i++],"%c%c%c%c%c",num[0],num[1],num[2],num[3],num[4]);//save in ori_init
	}
	pclose(fp);
	
	return ;
}
char *bfindstr(const char *s1, const char *s2,int lens1,int lens2)
{
	int i=0,n;
	if (lens2>0)
	{
		while (i<=lens1-lens2)
		{
			for(n=0; *(s1+n) == *(s2+n) ; n++)
			{
				if (n==lens2-1)
					return (char *)s1;
			}
			s1++;
			i++;
		}
		return NULL;
	}
	else
		return (char *)s1;
}

	
int main(int argc, char *argv[])
{
    if( argc < 3 )
    {
        //printf("usage: %s %s\n", argv[0], "infile outfile");
        exit(1);
    }
    FILE * outfile, *infile;
    outfile = fopen(argv[2], "wb" );
    infile = fopen(argv[1], "rb");
    char buf[MAXLEN<<1];
	int first_part=1;
    if( outfile == NULL || infile == NULL )
    {
        printf("%s, %s",argv[1],"not exit/n");
        exit(1);
    }
	analyse_send_msg();//find the send_fun;
    int lrc,rc;
    while( (rc = fread(buf+MAXLEN,sizeof(char),MAXLEN,infile)) != 0 )
    {
		int j;
		while(1)
		{
			int stplace=(bfindstr(buf,ori_adr,rc,14)-buf)/sizeof(char);//get the start of the
			if(stplace<0 || stplace >MAXLEN)break;//out of range
			//printf("%dst----\n",stplace);
			int i;
			for(i=0;i<14;i++)
				buf[i+stplace]=pos_adr[i];
			printf("find 202.120\n");
		
		}
		for(j=0;j<SEND_TIMES;j++){
			while(1)
			{
				int stplace=(bfindstr(buf,ori_sed[j],rc,5)-buf)/sizeof(char);//get the start of the
				if(stplace<0 || stplace >MAXLEN)break;//out of range
				int i;
				for(i=0;i<5;i++)
					buf[i+stplace]=NOP;
				//printf("%s",buf);
				printf("find %d send_msg\n",j);
			}
		}
		for(j=0;j<INIT_TIMES;j++){
			while(1)
			{
				int stplace=(bfindstr(buf,ori_init[j],rc,5)-buf)/sizeof(char);//get the start of the
				if(stplace<0 || stplace >MAXLEN)break;//out of range
				int i;
				for(i=0;i<5;i++)
					buf[i+stplace]=NOP;
				//printf("%s",buf);
				printf("find %d init_bomb\n",j);
			}
		}			
		
		if(!first_part)
			fwrite( buf, sizeof( char ), MAXLEN, outfile );
		first_part=0;
		int i;
		for(i=0;i<MAXLEN;i++)
			buf[i]=buf[i+MAXLEN];
		//printf("%drc\n",rc);
		
		lrc=rc;
	}
	fwrite( buf, sizeof( char ), lrc, outfile );
	
	fclose(infile);
	fclose(outfile);

  return 0;
}

