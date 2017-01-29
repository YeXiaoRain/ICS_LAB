/*
 * proxy.c - CS:APP Web proxy
 *
 * TEAM MEMBERS:
 *     Andrew Carnegie, ac00@cs.cmu.edu 
 *     Harry Q. Bovik, bovik@cs.cmu.edu
 * 
 * IMPORTANT: Give a high level description of your code here. You
 * must also provide a header comment at the beginning of each
 * function that describes what that function does.
 */ 

#include "csapp.h"
#define NTHREADS 16
#define SBUFSIZE 512
//--------------------------------SBUF--------------------------------
typedef struct{
	int *buf;
	int n;
	int front;
	int rear;
	sem_t mutex;
	sem_t slots;
	sem_t items;
}sbuf_t;

void sbuf_init(sbuf_t *sp,int n){
	sp->buf = Calloc(n,sizeof(int));
	sp->n = n;
	sp->front = sp->rear = 0;
	Sem_init(&sp->mutex,0,1);
	Sem_init(&sp->slots,0,n);
	Sem_init(&sp->items,0,0);
}

void sbuf_deinit(sbuf_t *sp){
	Free(sp->buf);
}

void sbuf_insert(sbuf_t *sp,int item){
	P(&sp->slots);
	P(&sp->mutex);
	sp->buf[(++sp->rear)%(sp->n)]=item;
	V(&sp->mutex);
	V(&sp->items);
}

int sbuf_remove(sbuf_t *sp){
	int item;
	P(&sp->items);
	P(&sp->mutex);
	item=sp->buf[(++sp->front)%(sp->n)];
	V(&sp->mutex);
	V(&sp->slots);
	return item;
}
//--------------------------------SBUF--------------------------------

sbuf_t sbuf;
sem_t mutexlog;
int logfd;
struct sockaddr_in browserlist[SBUFSIZE+3];
//map <int,(struct sockaddr_in)>browserlist;
/*
 * Function prototypes
 */

int minnum(int num1,int num2){return num1<num2?num1:num2;}
void echo_cnt(int connfd);
void * thread(void *vargp);
void clienterror(int fd,char *cause,char *errnum,char *shortmsg,char *longlong);
int parse_uri(char *uri, char *target_addr, char *path, int  *port);
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr, char *uri, int size);

/* 
 * main - Main routine for the proxy program 
 */
int main(int argc, char **argv)
{
	int i,listenfd,connfd,port;
	socklen_t clientlen= sizeof(struct sockaddr_in);
	struct sockaddr_in clientaddr;
	pthread_t tid;

    /* Check arguments */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port number>\n", argv[0]);
        exit(0);
    }

	port = atoi(argv[1]);
	sbuf_init(&sbuf,SBUFSIZE);
	Sem_init(&mutexlog,0,1);
	//##################add handler SIGINT SIGTSTP sbuf_deinit;
	listenfd = Open_listenfd(port);
	
	for(i=0;i<NTHREADS;i++)/* Create worker threads */
		Pthread_create(&tid,NULL,thread,NULL);
	
	logfd = Open("proxy.log", O_CREAT|O_WRONLY|O_APPEND,0);

	while(1){
		connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
		browserlist[connfd]=clientaddr;
		sbuf_insert(&sbuf,connfd);/* Insert connfd in buffer */
	}
    exit(0);
}

void * thread(void * vargp){//{{{
	Pthread_detach(pthread_self());
	while(1){
		int connfd = sbuf_remove(&sbuf);
		echo_cnt(connfd);
		Close(connfd);
	}
}//}}}

void echo_cnt(int connfd){//{{{
	char buf[MAXLINE],logString[MAXLINE];
	
	//as client connect to web server
	sem_t mutex_cnt;
	int cnt=0;
	size_t nWeb;
	int clientfd,port=-1;
	int cttlen=0;//content-length
	char host[MAXLINE],pathname[MAXLINE];
	char method[MAXLINE],uri[MAXLINE],version[MAXLINE];
	rio_t rio2net;
	
	//as server connect to user
	size_t nUsr;
	rio_t rio2usr;

	//init
	Sem_init(&mutex_cnt,0,1);
	Rio_readinitb(&rio2usr,connfd);
	
	if((nUsr=rio_readlineb(&rio2usr,buf,MAXLINE))<= 0 ){//what a fuck about Rio_readlineb();
		clienterror(connfd,NULL,"404","Not found","=.= NOOOT FOUND!!!!!!!!!");
		return ;
	}

	if(sscanf(buf,"%s %s %s",method,uri,version)!=3){
		clienterror(connfd,NULL,"400","Bad request","0.0 bad request");
		return ;
	}
	printf("#####_%s %s %s\n",method,uri,version);	
	
	parse_uri(uri,host,pathname,&port);
	
	if((clientfd = open_clientfd(host,port))<0)
		return;//how to check while no net is here

	do{
		printf("send:{%s }",buf);
		if((int)rio_writen(clientfd,buf,nUsr)!=nUsr)
			break;

		if( ( nUsr == 2 && (strcmp(buf, "\r\n")==0)) || (nUsr == 1 && ( buf[0]=='\r' || buf[0]=='\n'))){//empty line check?
			printf("request empty line Exit\n");
			break ;
		}
	}while((nUsr=rio_readlineb(&rio2usr,buf,MAXLINE))>0 );
	
	Rio_readinitb(&rio2net,clientfd);

	printf("MAXLINE:%d\n",MAXLINE);
	printf("read response\n");
	//read response
	while((nWeb=rio_readlineb(&rio2net,buf,MAXLINE))>0){
		cnt+=nWeb;
		if((int)rio_writen(connfd,buf,nWeb)!=nWeb)
			break;
		
		if(strstr(buf,"Content-Length")!=NULL)
			sscanf(buf,"%*s %d",&cttlen);

		printf("response:{%s}",buf);
		if( ( nUsr == 2 && (strcmp(buf, "\r\n")==0)) || (nUsr == 1 && ( buf[0]=='\r' || buf[0]=='\n'))){//empty line check?
			printf("response empty line Exit\n");
			break ;
		}
	
	
	}
	printf("read body\n");
	
	//read body

	while((nWeb=rio_readnb(&rio2net,buf,MAXLINE))>0){//what a fuck about Rio_readlineb();
		cnt+=nWeb;
		if((int)rio_writen(connfd,buf,nWeb)!=nWeb)
			break;
	}

	printf("finish body\n");

	//log here;
	printf("cnt=%d\n",cnt);

	format_log_entry(logString, &browserlist[connfd], uri, cnt);
	P(&mutexlog);
	rio_writen(logfd,logString,strlen(logString));
	V(&mutexlog);

	printf("close!\n");
	Close(clientfd);
	return ;
}//}}}
/*
 *  P641
 */
void clienterror(int fd, char *cause, char *errnum,char *shortmsg, char *longmsg){//{{{
	char buf[MAXLINE], body[MAXBUF];
	
	/* Build the HTTP response body */
	sprintf(body, "<html><title>404 Error</title>");
	sprintf(body, "%s<body bgcolor=""FFFFFF"">\r\n", body);
	sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
	sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
	sprintf(body, "%s<hr><em>The lab server</em>\r\n", body);

	/* Print the HTTP response */
	sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
	rio_writen(fd, buf, strlen(buf));
	sprintf(buf, "Content-type: text/html\r\n");
	rio_writen(fd, buf, strlen(buf));
	sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
	rio_writen(fd, buf, strlen(buf));
	rio_writen(fd, body, strlen(body));
}//}}}

/*
 * parse_uri - URI parser
 * 
 * Given a URI from an HTTP proxy GET request (i.e., a URL), extract
 * the host name, path name, and port.  The memory for hostname and
 * pathname must already be allocated and should be at least MAXLINE
 * bytes. Return -1 if there are any problems.
 */
int parse_uri(char *uri, char *hostname, char *pathname, int *port)
{
    char *hostbegin;
    char *hostend;
    char *pathbegin;
    int len;

    if (strncasecmp(uri, "http://", 7) != 0) {
        hostname[0] = '\0';
        return -1;
    }

    /* Extract the host name */
    hostbegin = uri + 7;
    hostend = strpbrk(hostbegin, " :/\r\n\0");
    len = hostend - hostbegin;
    strncpy(hostname, hostbegin, len);
    hostname[len] = '\0';

    /* Extract the port number */
    *port = 80; /* default */
    if (*hostend == ':')   
        *port = atoi(hostend + 1);

    /* Extract the path */
    pathbegin = strchr(hostbegin, '/');
    if (pathbegin == NULL) {
        pathname[0] = '\0';
    }
    else {
        pathbegin++;	
        strcpy(pathname, pathbegin);
    }

    return 0;
}

/*
 * format_log_entry - Create a formatted log entry in logstring. 
 * 
 * The inputs are the socket address of the requesting client
 * (sockaddr), the URI from the request (uri), and the size in bytes
 * of the response from the server (size).
 */
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr, 
        char *uri, int size)
{
    time_t now;
    char time_str[MAXLINE];
    unsigned long host;
    unsigned char a, b, c, d;

    /* Get a formatted time string */
    now = time(NULL);
    strftime(time_str, MAXLINE, "%a %d %b %Y %H:%M:%S %Z", localtime(&now));

    /* 
     * Convert the IP address in network byte order to dotted decimal
     * form. Note that we could have used inet_ntoa, but chose not to
     * because inet_ntoa is a Class 3 thread unsafe function that
     * returns a pointer to a static variable (Ch 13, CS:APP).
     */
    host = ntohl(sockaddr->sin_addr.s_addr);
    a = host >> 24;
    b = (host >> 16) & 0xff;
    c = (host >> 8) & 0xff;
    d = host & 0xff;


    /* Return the formatted log entry string */
    sprintf(logstring, "%s: %d.%d.%d.%d %s %d\n", time_str, a, b, c, d, uri, size);
}


