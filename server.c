/* servTCPConcTh2.c - Exemplu de server TCP concurent care deserveste clientii
   prin crearea unui thread pentru fiecare client.
   Asteapta un numar de la clienti si intoarce clientilor numarul incrementat.
	Intoarce corect identificatorul din program al thread-ului.
  
   
   Autor: Lenuta Alboaie  <adria@infoiasi.ro> (c)2009
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>

/* portul folosit */
#define PORT 2908
#define RANDURI 6
#define COLOANE 7

/* codul de eroare returnat de anumite apeluri */
extern int errno;

typedef struct thData{
	int idThread; //id-ul thread-ului tinut in evidenta de acest program
	int cl; //descriptorul intors de accept
}thData;

static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
void raspunde(void *);
int is_final(char gameboard[RANDURI][COLOANE]);
int verifica_orizontala(char gameboard[RANDURI][COLOANE]);
int verifica_verticala(char gameboard[RANDURI][COLOANE]);
void print_gameboard(char gameboard[RANDURI][COLOANE]);

int main ()
{
  char gameboard[RANDURI][COLOANE];
  /* initializam matricea */
  for (int i = 0; i < RANDURI; i++)
  {
    for (int j = 0; j < COLOANE; j++)
    {
      gameboard[i][j] = ' ';
    }
  }

  gameboard[0][0] = 'A';
  gameboard[0][1] = 'A';
  gameboard[0][2] = 'B';
  gameboard[0][3] = 'A';
  gameboard[2][0] = 'A';
  gameboard[3][0] = 'A';
  gameboard[4][0] = 'A';
  gameboard[5][0] = 'A';

  print_gameboard(gameboard);
  
  if (is_final(gameboard) == 1)
  {
    printf("%s \n","SUPER");
  }
  

  struct sockaddr_in server;	// structura folosita de server
  struct sockaddr_in from;	
  int nr;		//mesajul primit de trimis la client 
  int sd;		//descriptorul de socket 
  int pid;
  pthread_t th[100];    //Identificatorii thread-urilor care se vor crea
	int i=0;
  

  /* crearea unui socket */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("[server]Eroare la socket().\n");
      return errno;
    }
  /* utilizarea optiunii SO_REUSEADDR */
  int on=1;
  setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
  
  /* pregatirea structurilor de date */
  bzero (&server, sizeof (server));
  bzero (&from, sizeof (from));
  
  /* umplem structura folosita de server */
  /* stabilirea familiei de socket-uri */
    server.sin_family = AF_INET;	
  /* acceptam orice adresa */
    server.sin_addr.s_addr = htonl (INADDR_ANY);
  /* utilizam un port utilizator */
    server.sin_port = htons (PORT);
  
  /* atasam socketul */
  if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
      perror ("[server]Eroare la bind().\n");
      return errno;
    }

  /* punem serverul sa asculte daca vin clienti sa se conecteze */
  if (listen (sd, 2) == -1)
    {
      perror ("[server]Eroare la listen().\n");
      return errno;
    }
  /* servim in mod concurent clientii...folosind thread-uri */
  while (1)
    {
      int client;
      thData * td; //parametru functia executata de thread     
      int length = sizeof (from);

      printf ("[server]Asteptam la portul %d...\n",PORT);
      fflush (stdout);

      // client= malloc(sizeof(int));
      /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
      if ( (client = accept (sd, (struct sockaddr *) &from, &length)) < 0)
	{
	  perror ("[server]Eroare la accept().\n");
	  continue;
	}
	
        /* s-a realizat conexiunea, se astepta mesajul */
    
	// int idThread; //id-ul threadului
	// int cl; //descriptorul intors de accept

	td=(struct thData*)malloc(sizeof(struct thData));	
	td->idThread=i++;
	td->cl=client;

	pthread_create(&th[i], NULL, &treat, td);	      
				
	}//while    
};				
static void *treat(void * arg)
{		
		struct thData tdL; 
		tdL= *((struct thData*)arg);	
		printf ("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
		fflush (stdout);		 
		pthread_detach(pthread_self());		
		raspunde((struct thData*)arg);
		/* am terminat cu acest client, inchidem conexiunea */
		close ((intptr_t)arg);
		return(NULL);	
  		
};


void raspunde(void *arg)
{
  int nr, i=0;
	struct thData tdL; 
	tdL= *((struct thData*)arg);
	if (read (tdL.cl, &nr,sizeof(int)) <= 0)
			{
			  printf("[Thread %d]\n",tdL.idThread);
			  perror ("Eroare la read() de la client.\n");
			
			}
  int a[2][3];
  for (int i = 0; i < 2; i++)
  {
    for (int j = 0; j < 3; j++)
    {
      if (i == 0)
      {
        a[i][j] = 0;
      }
      else
      {
        a[i][j] = 1;
      }
    }
  }
  
	printf ("[Thread %d]Mesajul a fost receptionat...%d\n",tdL.idThread, nr);
		      
	/*pregatim mesajul de raspuns */
	nr++;      
	printf("[Thread %d]Trimitem mesajul inapoi...%d\n",tdL.idThread, nr);
		      
		      
	/* returnam mesajul clientului */
	 if (write (tdL.cl, &a, sizeof(a)) <= 0)
		{
		 printf("[Thread %d] ",tdL.idThread);
		 perror ("[Thread]Eroare la write() catre client.\n");
		}
	else
		printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",tdL.idThread);	

}


void print_gameboard(char gameboard[RANDURI][COLOANE])
{
  for (int i = 0; i <= COLOANE + 1; i++)
  {
    printf("%c",'_');
  }
  printf("\n");
  for (int i = 0; i < RANDURI; i++)
  {
    printf("%c",'|');
    for (int j = 0; j < COLOANE; j++)
    {
      printf("%c", gameboard[i][j]);
    }
    printf("%c",'|');
    printf("\n");
  }
  for (int i = 0; i <= COLOANE + 1; i++)
  {
    printf("%c",'^');
  }
  printf("\n");
}

int is_final(char gameboard[RANDURI][COLOANE])
{
  if (verifica_verticala(gameboard) == 1 || verifica_orizontala(gameboard) == 1)
  {
    return 1;
  }
  return 0;
}


int verifica_orizontala(char gameboard[RANDURI][COLOANE])
{
  for (int i = 0; i < RANDURI; i++)
  {
    for (int j = 0; j < COLOANE - 3; j++)
    {
      if(gameboard[i][j] == gameboard[i][j+1] && gameboard[i][j+1] == gameboard[i][j+2] && gameboard[i][j+2] == gameboard[i][j+3] && gameboard[i][j]!=' ')
        return 1;
    }
  }
  return 0;
}

int verifica_verticala(char gameboard[RANDURI][COLOANE])
{
  for (int i = 0; i < RANDURI - 3; i++)
  {
    for (int j = 0; j < COLOANE; j++)
    {
      if (gameboard[i][j] == gameboard[i+1][j] && gameboard[i+1][j] == gameboard[i+2][j] && gameboard[i+2][j] == gameboard[i+3][j] && gameboard[i][j] != ' ')
      {
        return 1;
      }
    }
  }
  return 0;
}