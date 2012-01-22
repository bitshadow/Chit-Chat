/**
 * Copyright (C)      2011 Jignesh kakadiya<jigneshhk1992@gmail.com>
 *               
 * This file is part of the chat-client in c project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.* 
 *
 */
/* client process */ 
#include <ctype.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <stdlib.h>
#include<stdio.h>
#include <string.h>
#include <pthread.h>
#define SIZE sizeof(struct sockaddr_in) 
void firstconn();
void* recieve();
void* getmsg();
void catcher(int sig); 
int sockfd; 
char msg[256];
pthread_t thrd1;
pthread_t thrd2;

int main(void) 
{ 

struct sockaddr_in server = {AF_INET, 7001}; 
/* convert and store the server's IP address */ 
server.sin_addr.s_addr = inet_addr("10.100.93.58"); 

/* set up the transport end point */ 
if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
{ 
	perror("socket call failed"); 
	exit(1); 
} 
/* connect the socket to the server's address */ 
if ( connect (sockfd, (struct sockaddr *)&server, SIZE) == -1) 
{ 
	perror("connect call failed"); 
	exit(1); 
} 

firstconn();
/* send and receive information with the server */ 
//pthread_create(&thrd1, NULL,getmsg ,NULL);	// handle data from a client
//pthread_join(thrd1, NULL);
recieve();
pthread_create(&thrd2, NULL,getmsg ,NULL);	// handle data from a client
pthread_join(thrd2, NULL);
	
}
void* recieve()
{
 if( fork() == 0)
  { int bytes;   
	for(;;){
	bytes = recv(sockfd,msg,sizeof(msg), 0);
		
	if (bytes > 0) 
		{// printf("recieved..\n");
		 printf("%s\n", msg); } 
	
	if(bytes == 0)
           {printf("server is no more running");
            close(sockfd);
            exit(0);}
      }
  }
}
void* getmsg()
{  
  printf("------------------------MENU---------------------------\n\n");
  printf("To view exiting members type  'list:'\n");
  printf("To multicast type             'multicast:'\n");
  printf("To send message to perticular nick type 'nick:message' \n\n");
  printf("-------------------------------------------------------\n");

  while(1)
     { 
	unsigned int i;int p;
	a://printf("\nENTER nick:msg \n");
	  gets(msg);
          p=-1;
          for(i = 0 ; i < strlen(msg) ; i++)
             if( msg[i] == ':')
               p = 1;
          if(p == 1)
             send(sockfd,msg,sizeof(msg),0);	 
          else goto a;
      }
}

void firstconn()
{    while(1)
        {
         printf("enter your nick : ");
	 gets( msg ); 
         if(strcmp(msg,""))
          { send(sockfd,msg, sizeof(msg), 0); 
	   break;}
	 else continue;
	} 
}


