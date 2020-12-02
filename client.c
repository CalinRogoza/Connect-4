/* cliTCPIt.c - Exemplu de client TCP
   Trimite un numar la server; primeste de la server numarul incrementat.
         
   Autor: Lenuta Alboaie  <adria@infoiasi.ro> (c)2009
*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>

#define RANDURI 6
#define COLOANE 7
#define RED  "\x1B[31m"   //https://stackoverflow.com/questions/3585846/color-text-in-terminal-applications-in-unix
#define GREEN  "\x1B[32m"
#define YELLOW  "\x1B[33m"
#define BLUE  "\x1B[34m"
#define MAGENTA  "\x1B[35m"
#define CYAN  "\x1B[36m"
#define RESET  "\x1b[0m"
char gameboard[RANDURI][COLOANE];

/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server*/
int port;


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
      if (gameboard[i][j] == 'A')
      {      
        printf(RED "%c" RESET, gameboard[i][j]);
      }
      else
      {
        printf(BLUE "%c" RESET, gameboard[i][j]);
      }
    }
    printf("%c",'|');
    printf("\n");
  }
  for (int i = 0; i <= COLOANE + 1; i++)
  {
    printf("%c",'^');
    fflush(stdout);
  }
  printf("\n");
  fflush(stdout);
}


int main (int argc, char *argv[])
{
  int sd;			// descriptorul de socket
  struct sockaddr_in server;	// structura folosita pentru conectare 
  		// mesajul trimis
  int nr=0;
  char buf[100];

  /* exista toate argumentele in linia de comanda? */
  if (argc != 3)
    {
      printf ("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
      return -1;
    }

  /* stabilim portul */
  port = atoi (argv[2]);

  /* cream socketul */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("Eroare la socket().\n");
      return errno;
    }

  /* umplem structura folosita pentru realizarea conexiunii cu serverul */
  /* familia socket-ului */
  server.sin_family = AF_INET;
  /* adresa IP a serverului */
  server.sin_addr.s_addr = inet_addr(argv[1]);
  /* portul de conectare */
  server.sin_port = htons (port);
  
  /* ne conectam la server */
  if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
    {
      perror ("[client]Eroare la connect().\n");
      return errno;
    }

  /* citirea mesajului */
  printf ("[client]Alegeti o culoare cu care sa jucati(");
  printf(RED "RED" RESET "/");
  printf(BLUE "BLUE" RESET "/");
  printf(CYAN "CYAN" RESET "/");
  printf(MAGENTA "MAGENTA" RESET "/");
  printf(YELLOW "YELLOW" RESET "/");
  printf(GREEN "GREEN" RESET "): ");
  fflush (stdout);
  read (0, buf, sizeof(buf));
  //scanf("%d",&nr);
  
  printf(BLUE "[client] Am citit %s\n" RESET, buf);

  /* trimiterea mesajului la server */
  if (write (sd,&buf,sizeof(buf)) <= 0)
    {
      perror ("[client]Eroare la write() spre server.\n");
      return errno;
    }

  /* citirea raspunsului dat de server 
     (apel blocant pina cind serverul raspunde) */
  if (read (sd, &gameboard,sizeof(gameboard)) < 0)
    {
      perror ("[client]Eroare la read() de la server.\n");
      return errno;
    }
  
  /* afisam mesajul primit */
  printf ("[client]Mesajul primit este: \n");
  print_gameboard(gameboard);  

  /* inchidem conexiunea, am terminat */
  close (sd);
}