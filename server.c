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


#define PORT 2008
#define RANDURI 6
#define COLOANE 7
#define RED  "\x1B[31m"   //https://stackoverflow.com/questions/3585846/color-text-in-terminal-applications-in-unix
#define GREEN  "\x1B[32m"
#define YELLOW  "\x1B[33m"
#define BLUE  "\x1B[34m"
#define MAGENTA  "\x1B[35m"
#define CYAN  "\x1B[36m"
#define RESET  "\x1b[0m"

/* codul de eroare returnat de anumite apeluri */
extern int errno;

typedef struct thData{
	int idThread;
	int cl;
}thData;

static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
void raspunde(void *);
char gameboard[RANDURI][COLOANE];
int numar_jucatori = 0;
int castigator = 0;
int is_final(char gameboard[RANDURI][COLOANE]);
int verifica_orizontala(char gameboard[RANDURI][COLOANE]);
int verifica_verticala(char gameboard[RANDURI][COLOANE]);
int verifica_diagonala_principala(char gameboard[RANDURI][COLOANE]);
int verifica_diagonala_secundara(char gameboard[RANDURI][COLOANE]);
void print_gameboard(char gameboard[RANDURI][COLOANE]);
void initializeaza_matrice(char gameboard[RANDURI][COLOANE]);

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

int main ()
{
  /* initializam matricea */
  initializeaza_matrice(gameboard);
  print_gameboard(gameboard);
  
  struct sockaddr_in server;	// structura folosita de server
  struct sockaddr_in from;	
  int sd;		//descriptorul de socket 
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

    /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
    if ( (client = accept (sd, (struct sockaddr *) &from, &length)) < 0)
	  {
	    perror ("[server]Eroare la accept().\n");
	    continue;
	  }
    
	// int idThread; //id-ul threadului
	// int cl; //descriptorul intors de accept

	td=(struct thData*)malloc(sizeof(struct thData));	
	td->idThread=i++;
	td->cl=client;

  numar_jucatori++;

	pthread_create(&th[i], NULL, &treat, td);	      
				
	} 
}


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
  int i = 0;
  int mutare;
  char turn[10] = "1";
  char color[100];
  char player = 'A';
  int joc_terminat = 0;
	struct thData tdL; 
	tdL= *((struct thData*)arg);

	if (read (tdL.cl, &color,sizeof(color)) <= 0)
	{
	  printf("[Thread %d]\n",tdL.idThread);
	  perror ("Eroare la read() de la client.\n");
	}
  
	printf ("[Thread %d]Mesajul a fost receptionat...%s\n",tdL.idThread, color);
		      
  if(numar_jucatori == 2)
  {	
    while (is_final(gameboard) == 0)
    {
      if(strcmp(turn, "1") == 0)
      { 

        if (tdL.idThread == 1)
        { 
          pthread_mutex_lock(&mutex1);
          strcpy(turn, "WAIT");

          if(write(tdL.cl, &turn, sizeof(turn)) <= 0)
          {
            printf("[Thread %d] ",tdL.idThread);
            perror ("[Thread]Eroare la write() catre client.\n");
          }
          pthread_mutex_unlock(&mutex2);
        }
        
        if(tdL.idThread == 0)
        { 
          pthread_mutex_lock(&mutex2);
          strcpy(turn, "1");

          player = 'A';

          if(write(tdL.cl, &turn, sizeof(turn)) <= 0)
          {
            printf("[Thread %d] ",tdL.idThread);
            perror ("[Thread]Eroare la write() catre client.\n");
          }
          
          if(write(tdL.cl, &gameboard, sizeof(gameboard)) <= 0)
          {
            printf("[Thread %d] ",tdL.idThread);
            perror ("[Thread]Eroare la write() catre client.\n");
          }

          if(read(tdL.cl, &mutare, sizeof(mutare)) <= 0)
          {
            printf("[Thread %d] ",tdL.idThread);
            perror ("[Thread]Eroare la read() catre client.\n");
          }

          int a = 0;
          for (int i = RANDURI - 1; i >= 0 && a == 0; i--)      // mutare
          {
            if (gameboard[i][mutare] == ' ')
            {
              gameboard[i][mutare] = player;
              a = 1;
            }
          }

          print_gameboard(gameboard);

          pthread_mutex_unlock(&mutex1);
        }
        strcpy(turn, "2");

      }
      else if (strcmp(turn, "2") == 0)
      { 

        if (tdL.idThread == 0)
        { 
          pthread_mutex_lock(&mutex2);
          strcpy(turn, "WAIT");

          if(write(tdL.cl, &turn, sizeof(turn)) <= 0)
          {
            printf("[Thread %d] ",tdL.idThread);
            perror ("[Thread]Eroare la write() catre client.\n");
          }

          pthread_mutex_unlock(&mutex1);
        }

        if(tdL.idThread == 1)
        { 
          pthread_mutex_lock(&mutex1);
          strcpy(turn, "2");

          player = 'B';

          if(write(tdL.cl, &turn, sizeof(turn)) <= 0)
          {
            printf("[Thread %d] ",tdL.idThread);
            perror ("[Thread]Eroare la write() catre client.\n");
          }
          
          if(write(tdL.cl, &gameboard, sizeof(gameboard)) <= 0)
          {
            printf("[Thread %d] ",tdL.idThread);
            perror ("[Thread]Eroare la write() catre client.\n");
          }

          if(read(tdL.cl, &mutare, sizeof(mutare)) <= 0)
          {
            printf("[Thread %d] ",tdL.idThread);
            perror ("[Thread]Eroare la read() catre client.\n");
          }

          int a = 0;
          for (int i = RANDURI - 1; i >= 0 && a == 0; i--)       // mutare
          {
            if (gameboard[i][mutare] == ' ')
            {
              gameboard[i][mutare] = player;
              a = 1;
            }
          }

          print_gameboard(gameboard);

          pthread_mutex_unlock(&mutex2);
        }
        strcpy(turn, "1");
         
      }  
    }

    if (is_final(gameboard) == 1)
    {
      strcpy(turn, "TERMINAT");

      if(write(tdL.cl, &turn, sizeof(turn)) <= 0)
        {
          printf("[Thread %d] ",tdL.idThread);
          perror ("[Thread]Eroare la write() catre client.\n");
        }

      if(write(tdL.cl, &castigator, sizeof(castigator)) <= 0)
        {
          printf("[Thread %d] ",tdL.idThread);
          perror ("[Thread]Eroare la write() catre client.\n");
        }

    }
    
  }
		      
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
  if (verifica_verticala(gameboard) || verifica_orizontala(gameboard) || verifica_diagonala_principala(gameboard) || verifica_diagonala_secundara(gameboard))
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
      {
        if(gameboard[i][j] == 'A')
          castigator = 1;
        else
          castigator = 2;        
        
        return 1;
      }
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
        if(gameboard[i][j] == 'A')
          castigator = 1;
        else
          castigator = 2;        
        
        return 1;
      }
    }
  }
  return 0;
}

int verifica_diagonala_principala(char gameboard[RANDURI][COLOANE])
{
  for (int i = 0; i < RANDURI - 3; i++)
  {
    for (int j = 0; j < COLOANE - 3; j++)
    {
      if (gameboard[i][j] == gameboard[i+1][j+1] && gameboard[i+1][j+1] == gameboard[i+2][j+2] && gameboard[i+2][j+2] == gameboard[i+3][j+3] && gameboard[i][j] != ' ')
      {
        if(gameboard[i][j] == 'A')
          castigator = 1;
        else
          castigator = 2;        
        
        return 1;
      }
    }
  }
  return 0;
}

int verifica_diagonala_secundara(char gameboard[RANDURI][COLOANE])
{
  for (int i = 0; i < RANDURI - 3; i++)
  {
    for (int j = COLOANE; j >= 3; j--)
    {
      if (gameboard[i][j] == gameboard[i+1][j-1] && gameboard[i+1][j-1] == gameboard[i+2][j-2] && gameboard[i+2][j-2] == gameboard[i+3][j-3] && gameboard[i][j] != ' ')
      {
        if(gameboard[i][j] == 'A')
          castigator = 1;
        else
          castigator = 2;        
        
        return 1;
      }
    }
  }
  return 0;
}

void initializeaza_matrice(char gameboard[RANDURI][COLOANE])
{
  for (int i = 0; i < RANDURI; i++)
  {
    for (int j = 0; j < COLOANE; j++)
    {
      gameboard[i][j] = ' ';
    }
  }
}