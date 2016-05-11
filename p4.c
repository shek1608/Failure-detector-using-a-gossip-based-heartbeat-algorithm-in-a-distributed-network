#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>

#include <fcntl.h>
#include <errno.h>

#define BUFLENGTH 4096


int N, b, c, F, B, P, S,I;
struct ifreq ip;
struct sockaddr_in * sock;
struct sockaddr_in myaddr;
struct sockaddr_in remaddr;
struct sockaddr_in forsize;
socklen_t addrlen;// = sizeof(remaddr);
int recvlen;
unsigned char buffer[BUFLENGTH];
int fd1;
int portno = 0;
char myip[20];
pthread_t thread[1];
int goahead=0,ok=0,lastguy=0;

int local_time;

void init_time()
{
	local_time=0;
}

int get_time()
{
	return local_time;
}

void increment_time()
{
	local_time++;
}


typedef struct nodes
{
	char ip[20];
	int port;
} nodes_t;

void *server(void *args)
{
	addrlen = sizeof(remaddr);
	//write(1,"Created!\n",9);

	if ((fd1 = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		printf("Socket not created! Exiting\n");
        exit(0);			
    }
    memset((char *)&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port = htons(portno);

    if (bind(fd1, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0)
    {
    	printf("Bind failed! Exiting!\n");
    	close(fd1);
        exit(0);
    }
    int ret;
    socklen_t lenforsize;

    int count=0;
    do
    {
    	//printf("Trying to getsockname\n");
    	ret = getsockname(fd1,(struct sockaddr *)&forsize,&lenforsize);
    	count++;
    }while(ret==-1 && count<5);

    /*ret = getsockname(fd1,(struct sockaddr *)&forsize,&lenforsize);*/
    if(ret==-1)
    {
    	printf("Getsockname failed! Exiting!\n");
    	close(fd1);
        exit(0);
    }



    portno=(int)ntohs(forsize.sin_port);
    //printf("Port is %d %d\n",(int)forsize.sin_port,(int)ntohs(forsize.sin_port));
    printf("Port = %d\n", portno);
    goahead=1;
    //printf("Waiting for ok message\n");
    while(1)
    {
    	printf("Waiting again\n");
    	memset(buffer,0,BUFLENGTH);
	    recvlen = recvfrom(fd1, buffer, BUFLENGTH, 0, (struct sockaddr *)&remaddr, &addrlen);
	    ok=1;
	    //printf("received %d bytes\n", recvlen);
	    if (recvlen > 0)
	    {
	        buffer[recvlen] = 0;
	        //printf("Received message: <<%s>>\n", buffer);
	    }
	    if(strcmp(buffer,"OK")==0)
	    {
	    	//printf("Ok received\n");
	    	continue;
	    }

	    //REST OF CODE COMES HERE!

	}	//end of while

    close(fd1);
	
	return NULL;
}

int lines(FILE * file)
{
	fseek(file,0,SEEK_SET);
	int ch=0;
  	int lines=0;
  	while ((ch = fgetc(file)) != EOF)
    {
      if (ch == '\n')
    lines++;
    }
    return lines;
}




void main(int parameterCount, char* argument[])
{

	pthread_create(&thread[0],NULL,server,NULL);

	FILE * endpoints;
	if(parameterCount<8)
	{
		printf("Lesser arguments than expected\n");
		exit(0);
	}
	N = atoi(argument[1]);
	nodes_t nodeObj[N];
	b = atoi(argument[2]);
	nodes_t neighbor[b];
	c = atoi(argument[3]);
	F = atoi(argument[4]);
	B = atoi(argument[5]);
	P = atoi(argument[6]);
	S = atoi(argument[7]);

	//printf("Values are:\nN = %d\nb = %d\nc = %d\nF = %d\nB = %d\nP = %d\nS = %d\n",N,b,c,F,B,P,S);

	//Try to get own IP
	int fd = socket(AF_INET,SOCK_DGRAM,0);
	ip.ifr_addr.sa_family = AF_INET;
	strncpy(ip.ifr_name,"eth0",IFNAMSIZ-1);
	if((ioctl(fd,SIOCGIFADDR, &ip))>=0)
	{
		sock = (struct sockaddr_in *)&ip.ifr_addr;
		//printf("%s\n",inet_ntoa(sock->sin_addr));
	}
	else printf("ERROR!\n");
	close(fd);

	memset(myip,0,20);
	sprintf(myip,"%s",inet_ntoa(sock->sin_addr));
	printf("IP = %s\n",myip);
	//Have gotten self IP

	//Create the UDP socket and

	while(goahead==0);	//wait here till server is ready!
	//Create line to write in file
	char temp[50];
	memset(temp,0,sizeof(temp));
	sprintf(temp,"%s %d\n",inet_ntoa(sock->sin_addr),portno);
	//printf("%s\n", temp);	//temp now contains the line to write in endpoints file


	
	//Now to see if file exists...if yes, open in read/write mode, if no, create and open in write mode
	fd = open("endpoints", O_CREAT | O_WRONLY | O_EXCL, S_IRUSR | S_IWUSR);
	if (fd>=0)	//first guy
	{
		printf("File endpoints created\n");
		write(fd,temp,strlen(temp));
		//sleep(20);
		// printf("Closing!\n");
		close(fd);
	}	//end of first guy
	else
	{
  /* failure */
		if (errno == EEXIST) 
		{
			//printf("The file already existed\n");
			errno=0;
			endpoints = fopen("endpoints","a+");
			if(endpoints==NULL)
			{
				printf("Errno is %d\n",errno);				
			}
			else
			{
				//printf("Opened for append\n");
				fseek(endpoints,0,SEEK_END);				
				fwrite(temp,1,strlen(temp),endpoints);
				int endpoints_len = ftell(endpoints);
				I = lines(endpoints);	//I is the row in endpoints file (as stated in question)			
				fclose(endpoints);		
				I--;		
				//printf("Number of lines = %d\n",I);
				if(I==N-1)
				{
					//printf("I am the last guy!\n");
					ok=1;
					lastguy=1;	//flag used so that after the 'ok', only others have to parse endpoints file
					endpoints = fopen("endpoints","r");
					//fseek(endpoints,0,SEEK_SET);
					char endpoints_buffer[endpoints_len+1];
					fread(endpoints_buffer,sizeof(char),endpoints_len,endpoints);
					fclose(endpoints);
					endpoints_buffer[endpoints_len]='\0';
					//printf("endpoints data:\n<<%s>>\n",endpoints_buffer);
					int num_sent=0;
					int first=1;
					char *token;

					for(;num_sent<N;num_sent++)	//loop through list and send ok message
					{
						//printf("About to send to %d\n",num_sent+1);
						if(first==1)
						{
							token = strtok(endpoints_buffer," ");
							first=0;
						}
						else
						{
							token = strtok(NULL," ");						
						}
						int size1,size2;
						size1 = strlen(token);
						memset(nodeObj[num_sent].ip,0,20);
						snprintf(nodeObj[num_sent].ip,size1+1,token);

						//printf("Bleh: %s %d %s\n",token,size1,nodeObj[num_sent].ip);
						token = strtok(NULL,"\n");
						size2 = strlen(token);
						nodeObj[num_sent].port=atoi(token);
						printf("nodeObj[%d] = %s , %d\n",num_sent,nodeObj[num_sent].ip, nodeObj[num_sent].port);

						//printf("Bleh: %s %d %d\n",token,size2,nodeObj[num_sent].port);
						/*printf("%s\n",nodeObj[num_sent].ip);
						printf("%d\n",strlen(nodeObj[num_sent].ip));
						printf("%s\n",myip);
						printf("%d\n",strlen(myip));
						printf("%d\n",nodeObj[num_sent].port);
						printf("%d\n",portno);*/
						if((nodeObj[num_sent].port==portno)&&(strcmp(nodeObj[num_sent].ip,myip)==0))
						{
							//printf("Its a me! No need to send myself an ok message\n");
						}
						else
						{
							//need to send data
							//printf("Not me, sending data\n");
							struct sockaddr_in serverdata;
							int socket_fd;
							char okmessage[3];


							sprintf(okmessage,"OK");
							okmessage[2]='\0';
							socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
							if(socket_fd == -1)
							{
							printf("\nError creating socket\n");
							exit(0);
							}

							serverdata.sin_addr.s_addr = inet_addr(nodeObj[num_sent].ip);//127.0.0.1
							serverdata.sin_family = AF_INET;
							serverdata.sin_port = htons(nodeObj[num_sent].port);
							//printf("okmessage contains %s of length %d\n",okmessage,strlen(okmessage));
							sendto(socket_fd,okmessage,strlen(okmessage),0,(const struct sockaddr *)&serverdata,sizeof(struct sockaddr_in));
							close(socket_fd);


							//sendto(socket_fd,message,strlen(message),0,(const struct sockaddr *)&serverdata,sizeof(struct sockaddr_in));
						}
					}
				}	//end of code for last guy
			}	//end of endpoint file exists
		}
	}	//code for everyone except first guy

	while(ok==0);	//wait here till ok received;

	printf("Sync point reached\n");
	if(lastguy==0)	//if not last guy, have to populate nodeObj structure;
	{
		endpoints = fopen("endpoints","r");
		fseek(endpoints,0,SEEK_END);
		int endpoints_len = ftell(endpoints);
		fseek(endpoints,0,SEEK_SET);
		char endpoints_buffer[endpoints_len+1];
		fread(endpoints_buffer,sizeof(char),endpoints_len,endpoints);
		fclose(endpoints);
		endpoints_buffer[endpoints_len]='\0';
		int num_sent=0;
		int first=1;
		char *token;

		for(;num_sent<N;num_sent++)
		{
			if(first==1)
			{
				token = strtok(endpoints_buffer," ");
				first=0;
			}
			else
			{
				token = strtok(NULL," ");						
			}
			int size1;
			size1 = strlen(token);
			memset(nodeObj[num_sent].ip,0,20);
			snprintf(nodeObj[num_sent].ip,size1+1,token);
			token = strtok(NULL,"\n");
			nodeObj[num_sent].port=atoi(token);
			printf("nodeObj[%d] = %s , %d\n",num_sent,nodeObj[num_sent].ip, nodeObj[num_sent].port);
		}

	}

	init_time();
	printf("Current time is %d\n",get_time());

	srandom(S+I);
	
	int b1=0;
	printf("b1= %d, b= %d I= %d\n",b1,b,I);
	while(b1<b)
	{
		long int randomvalue = random();
		//printf("Random value is %ld\n",randomvalue);
		int toadd = randomvalue % N;
		//printf("Random value mod is %d, I is %d\n",toadd, I);
		if(toadd!=I)
		{
			int b2=0,alreadyadded=0;
			for(;b2<b1;b2++)
			{
				if((neighbor[b2].port==nodeObj[toadd].port)&&(strcmp(neighbor[b2].ip,nodeObj[toadd].ip)==0))
				{
					alreadyadded=1;
					printf("Already added\n");
					break;
				}
			}
			if(alreadyadded) continue;
			memset(neighbor[b1].ip,0,20);
			neighbor[b1].port=nodeObj[toadd].port;
			snprintf(neighbor[b1].ip,strlen(nodeObj[toadd].ip)+1,nodeObj[toadd].ip);
			printf("Neighbor added[%d]: %s %d\n",toadd,neighbor[b1].ip,neighbor[b1].port);
			b1++;
		}
	}



	while(1);
	close(fd1);
}	//end of main;