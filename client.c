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
#define GREEN  "\x1B[32m"  //https://www.lihaoyi.com/post/BuildyourownCommandLinewithANSIescapecodes.html
#define ORANGE  "\x1B[33m"
#define BLUE  "\x1B[34m"
#define MAGENTA  "\x1B[35m"
#define CYAN  "\x1B[36m"
#define BACKGROUND "\x1b[47;4;1m"
#define RESET  "\x1b[0m"
char gameboard[RANDURI][COLOANE];

/* codul de eroare returnat de anumite apeluri */
extern int errno;

int port;
char culoare[100]="";

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
      if (strcmp(culoare, "MAGENTA") == 0)
      {      
        printf(BACKGROUND MAGENTA "%c" RESET, gameboard[i][j]);
      }
      else if (strcmp(culoare, "RED") == 0)
      {
        printf(BACKGROUND RED "%c" RESET, gameboard[i][j]);
      }
      else if (strcmp(culoare, "BLUE") == 0)
      {
        printf(BACKGROUND BLUE "%c" RESET, gameboard[i][j]);
      }
      else if (strcmp(culoare, "GREEN") == 0)
      {
        printf(BACKGROUND GREEN "%c" RESET, gameboard[i][j]);
      }
      else if (strcmp(culoare, "CYAN") == 0)
      {
        printf(BACKGROUND CYAN "%c" RESET, gameboard[i][j]);
      }
      else if (strcmp(culoare, "ORANGE") == 0)
      {
        printf(BACKGROUND ORANGE "%c" RESET, gameboard[i][j]);
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
  int mutare;
  char turn[10];

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

  while (1)
  {
  
  

  printf ("[client]Alegeti o culoare cu care sa jucati(");
  printf(RED "RED" RESET "/");
  printf(BLUE "BLUE" RESET "/");
  printf(CYAN "CYAN" RESET "/");
  printf(MAGENTA "MAGENTA" RESET "/");
  printf(ORANGE "ORANGE" RESET "/");
  printf(GREEN "GREEN" RESET "): ");
  fflush (stdout);
  //read (0, culoare, sizeof(culoare));     //citim culoarea selectata de player
  scanf("%s", culoare);

  while (strcmp(culoare, "MAGENTA") != 0 && strcmp(culoare, "RED") != 0 && strcmp(culoare, "BLUE") != 0 && strcmp(culoare, "CYAN") != 0 && strcmp(culoare, "GREEN") != 0 && strcmp(culoare, "ORANGE") != 0)
  {
    printf("%s", culoare);
    printf("Introduceti o culoare valida!\n");
    scanf("%s", culoare);
  }
  

  printf(BLUE "[client] Am citit %s\n" RESET, culoare);


  if (write (sd, &culoare, sizeof(culoare)) <= 0)
    {
      perror ("[client]Eroare la write() spre server.\n");
      return errno;
    }

  while(strcmp(turn,"TERMINAT") != 0)
  {

    if (read (sd, &turn, sizeof(turn)) < 0)
      {
        perror ("[client]Eroare la read() de la server!!!!.\n");
        return errno;
      }

    printf("%s",turn);
    printf("\n");

    if(strcmp(turn,"TERMINAT") == 0)
    {
      printf("Jocul s-a incheiat!\n");
      int castigator;

      if (read (sd, &castigator, sizeof(castigator)) < 0)   // primim un mesaj ce contine castigatorul
      {
        perror ("[client]Eroare la read() de la server!!!!.\n");
        return errno;
      }
      printf("Castiga jucatorul cu numarul %d", castigator);
      printf("!\n");
    }
    else if(strcmp(turn, "WAIT") == 0)
    {
      ; // se asteapta randul jucatorului
    }
    else                        // e randul lui
    {
      if (read (sd, &gameboard, sizeof(gameboard)) < 0)
      {
        perror ("[client]Eroare la read() de la server.\n");
        return errno;
      }

      print_gameboard(gameboard);

      printf("[client] Faceti o mutare!\n");
      scanf("%d", &mutare);
      while (mutare != 0 && mutare != 1 && mutare != 2 && mutare != 3 && mutare != 4 && mutare != 5 && mutare != 6)
      {
        printf("Introduceti un numar intre 0 si 6!\n");
        scanf("%d", &mutare);
      }
      
      if (write (sd, &mutare, sizeof(mutare)) <= 0)
      {
        perror ("[client]Eroare la write() spre server.\n");
        return errno;
      }

      printf("[client]Asteptati, celalalt player muta...\n");

    }
  }
  strcpy(turn, ""); // urmatoarea repriza
  }
  close (sd);
}