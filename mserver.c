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

/* server process */
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#define SIZE sizeof(struct sockaddr_in)
#define MAX_THREADS 5
void multicast ();
int searchnick ();
void *handle_data ();
void *handle_new_connection ();
void catcher (int sig);
void *firstconn ();
int filedes;
fd_set master;			// master file descriptor list
fd_set read_fds;
static char nick_array[10][256];
int mainsocketfd, max, i, j;
int bytes;
char msg[256];
pthread_t threads[MAX_THREADS];
pthread_t thrd;
static char *name[MAX_THREADS];

/* main method starts here */
int
main (void)
{

  struct sockaddr_in serverdetail = { AF_INET, 7001, INADDR_ANY };
  static struct sigaction act;
  act.sa_handler = catcher;
  sigfillset (&(act.sa_mask));
  sigaction (SIGPIPE, &act, NULL);

  FD_ZERO (&master);		// clear the master 
  FD_ZERO (&read_fds);
  char *lst;
  lst = "list";
  strcpy (nick_array[0], lst);
  lst = "multicast";
  strcpy (nick_array[1], lst);

  /* set up the transport end point */
  if ((mainsocketfd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("socket call failed");
      exit (1);
    }


  /* bind an address to the end point */
  if (bind (mainsocketfd, (struct sockaddr *) &serverdetail, SIZE) == -1)
    {
      perror ("bind call failed");
      exit (1);
    }

  printf ("server is running ...have fun :p\n");

  /* start listening for incoming connections */
  if (listen (mainsocketfd, 5) == -1)
    {
      perror ("listen call failed");
      exit (1);
    }

  FD_SET (mainsocketfd, &master);
  max = mainsocketfd;

  while (1)
    {
      read_fds = master;	// copy it
      if (select (max + 1, &read_fds, NULL, NULL, NULL) == -1)
	{
	  perror ("select");
	  exit (4);
	}

      // run through the existing connections looking for data to read
      for (i = mainsocketfd; i <= max; i++)
	{
	  if (FD_ISSET (i, &read_fds))
	    {
	      if (i == mainsocketfd)
		{
		  pthread_create (&threads[i], NULL, handle_new_connection, NULL);	// handle new connections
		  pthread_join (threads[i], NULL);
		}
	      else
		{
		  pthread_create (&threads[i], NULL, handle_data, NULL);	// handle data from a client
		  pthread_join (threads[i], NULL);
		}
	    }
	}
    }
}

/*it will handles the new connection and give new socket to the client to communicate*/

void *
handle_new_connection ()
{
  filedes = accept (mainsocketfd, NULL, NULL);

  if (filedes == -1)
    {
      perror ("accept");
    }
  else
    {
      FD_SET (filedes, &master);	// add to master set
      if (filedes > max)
	{			// keep track of the max
	  max = filedes;
	}
      pthread_create (&thrd, NULL, firstconn, NULL);
      pthread_join (thrd, NULL);

    }

}

/* this method will handles the data send by client and according to the nick it will
   send to that client.*/

void *
handle_data ()
{
  bytes = recv (i, msg, sizeof (msg), 0);

  //send(i,"server::msg recieved...\n",256,0); 
  if (bytes == 0)
    {
      if (bytes == 0)
	{
	  char *p;
	  p = "";
	  printf ("socket %d hung up with nick : %s\n", i, nick_array[i]);
	  strcpy (nick_array[i], p);
	}
      else
	perror ("recv");

      close (i);
      FD_CLR (i, &master);
    }
  else
    {
      char *ptr;
      int nk;
      char niktemp[10], temp[300];
      int t, w;
      int y;
      char msgtemp[256];
      ptr = strchr (msg, ':');	// returns the pointer to :
      nk = ptr - msg;
      //============================seperating nick from msg                  
      for (t = 0; t < nk; t++)
	niktemp[t] = msg[t];
      niktemp[t] = '\0';
      //=====================================================
      //===================seperating actual message form msg                 
      for (t = 0; t < strlen (nick_array[i]); ++t)
	msgtemp[t] = nick_array[i][t];
      msgtemp[t++] = '-';
      msgtemp[t++] = '>';

      for (w = nk + 1, y = t; msg[w] != '\0'; y++, w++)
	msgtemp[y] = msg[w];
      msgtemp[y] = '\0';
      //printf("%s \n",msgtemp);                      
      //=====================================================
      printf ("==========================================\n");

      if (FD_ISSET (i, &master))
	{
	  if (searchnick (niktemp) > mainsocketfd)
	    {
	      //ptr = strcat(temp,ptr);
	      printf ("sending to socket %d\n", searchnick (niktemp));
	      if (send (searchnick (niktemp), msgtemp, sizeof (msg), 0) == -1)
		perror ("send");
	      printf ("sending complete\n");
	    }
	  else if (searchnick (niktemp) == 0)
	    {
	      int ls;
	      char *abc;
	      printf ("sending list to client at %d \n", i);
	      for (ls = mainsocketfd; ls <= max; ++ls)
		send (i, nick_array[ls], sizeof (msg), 0);
	    }
	  else if (searchnick (niktemp) == 1)
	    {
	      printf ("multicast request by client at %d\n", i);
	      multicast ();
	    }
	  else
	    {
	      send (i,
		    "server::nick name is currently not in the list .. :( \n",
		    sizeof (msg), 0);
	      printf ("::: delivering failed no such nick exists :::\n");
	    }

	  printf ("==========================================\n");
	}

    }

}

void
multicast ()
{
  for (j = mainsocketfd + 1; j <= max; j++)
    {
      if (j != mainsocketfd && i != j)
	{
	  printf ("%d sending to socket %d th \n", i, j);
	  //send(j,nick_array[j],sizeof(nick_array),0);                                  
	  if (send (j, msg, bytes, 0) == -1)
	    {
	      perror ("send ");
	    }

	}
    }
}


int
searchnick (char *nik)
{
  int k;

  for (k = 0; k <= max; ++k)
    if (!strcmp (nick_array[k], nik))
      return k;
  return -1;
}

void *
firstconn ()
{
  char nick[256];
  int n = 0;
  bytes = recv (filedes, msg, sizeof (msg), 0);
  while (1)
    {
      if (bytes > 0)
	{
	  strcpy (nick, msg);
	  printf ("=============================================\n");
	  printf ("new connection at %d \n", filedes);
	  printf ("nick : %s \n", nick);
	  strcpy (nick_array[filedes], msg);
	  printf ("SYN_ACK send...\n");
	  send (filedes, "server::connected to server .....\n", 256, 0);
	  printf ("\ncurrent existing members.....\n");
	  for (n = mainsocketfd; n <= filedes; n++)
	    printf ("%s \n", nick_array[n]);

	  printf ("=============================================\n");
	  break;
	}
    }
}


void
catcher (int sig)
{
  close (filedes);
  exit (0);
}
