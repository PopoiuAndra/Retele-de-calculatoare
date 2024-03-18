/* cliTCPIt.c - Exemplu de client TCP
   Trimite un numar la server; primeste de la server numarul incrementat.
         
   Autor: Lenuta Alboaie  <adria@info.uaic.ro> (c)
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
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>
#include <SFML/Graphics.hpp>

/* codul de eroare returnat de anumite apeluri */
extern int errno;

#define SIZE 2048

/* portul de conectare la server*/
int port;
int sd;			// descriptorul de socket

int tip_client = 0; /*Client-Client*/
int logat = 0;
char numeUtilizator[100];
char pozitie_actuala[100];
char destinatie[100];
int quit = 0;

char nume_statie[10][100] = {"", "st1", "st2", "st3", "st4", "st5", "st6", "st7", "st8", "st9"}; 

int X_statii[10] = { 0, 200, 600, 100, 400, 300, 500, 400, 600, 700};
int Y_statii[10] = { 0, 120,  60, 240, 180, 300, 360, 480, 420, 540};

int rute[4][7]={ /* ruta[i][0] = nr de statii din ruta */
  {},
  {4, 3, 5, 6, 8},
  {6, 2, 4, 5, 6, 8, 9},
  {5, 1, 3, 5, 6, 7}
};

char numar[100];
void itoa(int nr)
{
  int n = 0, aux;

  while(nr != 0)
  {
    numar[n++] = nr % 10 + '0';
    nr/=10;
  }
  numar[n] = '\0';

  for(int i = 0; i < n/2; ++ i)
  {
    aux = numar[i];
    numar[i] = numar[n - i - 1];
    numar[n - i - 1] = aux;
  }
}

void Reset()
{
  logat = tip_client = 0;
  strcpy(numeUtilizator, "");
  strcpy(pozitie_actuala, "");
  strcpy(destinatie, "");
}

void GraficaHarta()
{
  char msg[SIZE];
  int i;
  sf::RenderWindow window(sf::VideoMode(800, 600), "Window Harta");

  sf::RectangleShape background(sf::Vector2f(800, 800)); /* patrat de 800 x 800 */
  background.setFillColor(sf::Color::Green);

  /* scrisul din cercuri */
   sf::Font font;
  if (!font.loadFromFile("/usr/share/fonts/type1/urw-base35/NimbusRoman-Regular.t1"))  /* Pathul  fontului */
  { 
    perror("[client] Eroare la path-ul fontului");
    return;
  }

  sf::Text text;
  text.setFont(font); /* selecteaza fontul pentru text*/
  text.setFillColor(sf::Color::White); /* culoarea scrisului */
  text.setCharacterSize(20); /* marimea fiecarui caracter */


  /* liniile dintre statii*/
  sf::VertexArray line1(sf::Lines, 2); /* st1 - st3 */
  line1[0].position = sf::Vector2f(200, 120); // Primul vârf
  line1[1].position = sf::Vector2f(100, 240); // Al doilea vârf

  sf::VertexArray line2(sf::Lines, 2); /* st3 - st5 */
  line2[0].position = sf::Vector2f(100, 240); // Primul vârf
  line2[1].position = sf::Vector2f(300, 300); // Al doilea vârf

  sf::VertexArray line3(sf::Lines, 2); /* st5 - st4 */
  line3[0].position = sf::Vector2f(300, 300); // Primul vârf
  line3[1].position = sf::Vector2f(400, 180); // Al doilea vârf

  sf::VertexArray line4(sf::Lines, 2); /* st4 - st2 */
  line4[0].position = sf::Vector2f(400, 180); // Primul vârf
  line4[1].position = sf::Vector2f(600,  60); // Al doilea vârf

  sf::VertexArray line5(sf::Lines, 2); /* st5 - st6  */
  line5[0].position = sf::Vector2f(300, 300); // Primul vârf
  line5[1].position = sf::Vector2f(500, 360); // Al doilea vârf

  sf::VertexArray line6(sf::Lines, 2); /* st6 - st7 */
  line6[0].position = sf::Vector2f(500, 360); // Primul vârf
  line6[1].position = sf::Vector2f(400, 480); // Al doilea vârf

  sf::VertexArray line7(sf::Lines, 2); /* st6 - st8 */
  line7[0].position = sf::Vector2f(500, 360); // Primul vârf
  line7[1].position = sf::Vector2f(600, 420); // Al doilea vârf

  sf::VertexArray line8(sf::Lines, 2); /* st8 - st9 */
  line8[0].position = sf::Vector2f(600, 420); // Primul vârf
  line8[1].position = sf::Vector2f(700, 540); // Al doilea vârf
  

  sf::CircleShape circle(20); // Argumentul reprezintă raza cercului
  circle.setFillColor(sf::Color::Blue); // Setăm culoarea cercului

  while (window.isOpen())
  {
    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed)
        window.close();
    }

    strcpy(msg, "01harta");

    /* trimiterea mesajului la server */
    if (write (sd,&msg,sizeof(msg)) <= 0)
    {
      perror ("[client]Eroare la write() spre server.\n");
      return ;
    }

    /* citirea raspunsului dat de server 
      (apel blocant pina cind serverul raspunde) */
    if (read (sd, &msg,sizeof(msg)) < 0)
    {
      perror ("[client]Eroare la read() de la server.\n");
      return ;
    }

    strcpy(msg, (msg+6));

// SA NU UITI SA SCHIMBI NUMELE STATIILOR DIN ST1.. IN UNELE MAI OK 
    
    window.clear(); /* curata ecranu*/

    window.draw(background);

    // desenam liniile dintre statii 
    window.draw(line1);
    window.draw(line2);
    window.draw(line3);
    window.draw(line4);
    window.draw(line5);
    window.draw(line6);
    window.draw(line7);
    window.draw(line8);

    // desenam statiile ca niste cercuri negre
    circle.setFillColor(sf::Color::Blue);
    for(i = 1; i <= 9; ++ i)
    {
      circle.setPosition(X_statii[i]-20, Y_statii[i]-20); // Setăm poziția cercului în fereastră
      window.draw(circle);

      text.setString(nume_statie[i]);
      text.setPosition(X_statii[i] - 10, Y_statii[i]- 10); /* setez pozitia */
      window.draw(text); // scriu textul 

    }

    // desenam statia actuala/destinatie cu un partat albastru
    for(i = 1; i <= 9; ++ i)
      if(strcmp(pozitie_actuala, nume_statie[i]) == 0)
      {
        circle.setFillColor(sf::Color::Black);
        circle.setPosition(X_statii[i]-20, Y_statii[i]-20); // Setăm poziția cercului în fereastră
        window.draw(circle);

        text.setString(nume_statie[i]);
        text.setPosition(X_statii[i] - 10, Y_statii[i]- 10); /* setez pozitia */
        window.draw(text); // scriu textul 
      }
      else if(strcmp(destinatie, nume_statie[i]) == 0)
      {
        circle.setFillColor(sf::Color::Black);
        circle.setPosition(X_statii[i]-20, Y_statii[i]-20); // Setăm poziția cercului în fereastră
        window.draw(circle);

        text.setString(nume_statie[i]);
        text.setPosition(X_statii[i] - 10, Y_statii[i]- 10); /* setez pozitia */
        window.draw(text); // scriu textul 
      }


    // desenam autobuzele ca pct rosii si nr lor inauntru

    int X_autobuz[10]; // ca sa verificam daca exista un autobuz in acelasi loc
    int Y_autobuz[10];

    circle.setFillColor(sf::Color::Red);
    i = 1;
    while(i <= 6 && msg != NULL)
    {
      /* am primit msj de forma:*/
      /* 1:2-2:3:4 */
      /* 1 - nr rutei; 2 - orientarea; 3 - nr statiei ; 4 - cu cate minute am trecut de statie */
      char aux[10];
      aux[0] = msg[0];
      aux[1] = '\0';
      //printf("aux: %s\n", aux);

      int ruta = atoi(aux);
      //printf("ruta: %d\n", ruta);
      
      aux[0] = msg[6];
      aux[1] ='\0';
      //printf("aux: %s\n", aux);
      int nr = atoi(aux);
      //printf("nr: %d\n", nr);
      int st_act, next_st;
      //printf("max ruta:%d\n", rute[ruta][0]);
      
      int dir;
      aux[0] = msg[2]; // directia rutei

      if(atoi(aux) == rute[ruta][1]) dir = 1;
      else dir = 2;

      for(int j = 1; j <= rute[ruta][0]; ++ j)
        if(j == nr) 
        {
          st_act = rute[ruta][j];
          if(dir == 1) next_st = rute[ruta][j + 1];
          else next_st = rute[ruta][j - 1];
          break;
        }

      //printf("stat: %d Next: %d\n", st_act, next_st);

      

      int x, y;
        x = X_statii[st_act] + (X_statii[next_st] - X_statii[st_act]) / 3 * (atoi(msg + 8)); 
        y = Y_statii[st_act] + (Y_statii[next_st] - Y_statii[st_act]) / 3 * (atoi(msg + 8)); 
        //printf("X: %d Y: %d\n", x, y);

        X_autobuz[i] = x;
        Y_autobuz[i] = y; 

      circle.setPosition(x-20, y-20); // Setăm poziția cercului în fereastră
      window.draw(circle);

      int ct = 2;
      aux[0] = 'A';
      aux[1] = i + '0';
      aux[2] = '\0';

      for(int j = 1; j <= 6; ++ j)
        if(j != i && X_autobuz[i] == X_autobuz[j] && Y_autobuz[i] == Y_autobuz[j])
        { /* avem 2 autobuze suprapuse */
          aux[ct++] = '/'; aux[ct++] = j + '0';
          aux[ct] = '\0';
        }



      text.setString(aux);
      text.setPosition(x - 10, y - 10); /* setez pozitia */
      window.draw(text); // scriu textul 
      strcpy(aux, "");

      strcpy(msg, (msg+10));
      //printf("New msg: %s\n", msg);
      i++;
    }



    window.display(); /* updateaza Window */
    sleep(1);
  }
}

int Client() // daca vrea sa se deconecteze iesa din whileul infinit si o sa intre iar in bucla din main
{
  char msg[SIZE];
  char buf[SIZE];
  char *line = NULL;  /* forces getline to allocate with malloc */
  size_t len = 0;     /* ignored when line = NULL */

  while(1)
  {   
      if(logat == 1) printf("\n[client] Comenzi permise: pozitie_actuala:nume_statie -- destinatie:nume_statie -- determina_ruta-- logout -- harta -- informatii -- quit\n");
      else return 0;

      //printf("tip_client : %d logat : %d numeUtilizator: %s pozitie_actuala: %s destinatie: %s\n", tip_client, logat, numeUtilizator, pozitie_actuala, destinatie);

        /* citirea mesajului */
      printf ("[client] Introduceti comanda: ");
      if( (getline(&line, &len, stdin)) == -1)   
      {     printf("Eroare la citire in client\n");
      }
      fflush (stdout);
      strcpy(buf, line);
      //read (0, buf, sizeof(buf));

      /* pregatirea mesajului */
        msg[0] = tip_client + '0';
        msg[1] = logat + '0';
        msg[2] = '\0';
        strcat(msg, buf);
        msg[strlen(msg) - 1] = '\0';

        if(strcmp(msg, "01determina_ruta") == 0)
        {
          strcat(msg,":");
          strcat(msg, pozitie_actuala);
          strcat(msg, ":");
          strcat(msg, destinatie);
        }

    // printf("[client] Am trimis %s\n",msg);

      /* trimiterea mesajului la server */
      if (write (sd,&msg,sizeof(msg)) <= 0)
        {
          perror ("[client]Eroare la write() spre server.\n");
          return errno;
        }


      /* citirea raspunsului dat de server 
        (apel blocant pina cind serverul raspunde) */
      if (read (sd, &msg,sizeof(msg)) < 0)
      {
        perror ("[client]Eroare la read() de la server.\n");
        return errno;
      }

      if(strcmp( msg, "quit") == 0) {printf("La revedere!\n"); quit = 1; return 0;}
      if(strcmp(msg, "Sunteti delogat!") == 0) Reset();
      else if(strncmp(msg, "Statie actuala:", 15) == 0) 
        strcpy( pozitie_actuala, (strchr(msg, ':') + 1) );
      else if(strncmp(msg, "Destinatie:", 11) == 0) 
        strcpy( destinatie, (strchr(msg, ':') + 1) );
      else if(strncmp(msg, "harta", 5) == 0)
        GraficaHarta();
      else if(strncmp(msg, "Optiunea ", 9) == 0)
      {

        printf ("[client] %s\n", msg);

        printf("[client] Comenzi permise: alege_optiunea:nr -- exit\n");
        
        printf ("[client] Introduceti comanda: ");
        if( (getline(&line, &len, stdin)) == -1)   
        {     printf("Eroare la citire in client\n");
        }
        fflush (stdout);
        strcpy(buf, line);

        strcpy(msg, "");
        strcat(msg, buf);
        msg[strlen(msg) - 1] = '\0';

        if (write (sd,&msg,sizeof(msg)) <= 0)
        {
          perror ("[client]Eroare la write() spre server.\n");
          return errno;
        }

        if (read (sd, &msg,sizeof(msg)) < 0)
        {
          perror ("[client]Eroare la read() de la server.\n");
          return errno;
        }

        if(strcmp(msg, "Comanda nu a fost gasita!") != 0 && strcmp(msg, "Am iesit din optiuni!") != 0 && strncmp(msg, "Nu mai", 6) != 0)
        {
          printf("[client] %s\n", msg);
          printf ("[client] Introduceti comanda: ");
          if( (getline(&line, &len, stdin)) == -1)   
          {     printf("Eroare la citire in client\n");
          }
          fflush (stdout);
          strcpy(buf, line);

          strcpy(msg, buf);
          msg[strlen(msg) - 1] = '\0';

          if (write (sd,&msg,sizeof(msg)) <= 0)
          {
            perror ("[client]Eroare la write() spre server.\n");
            return errno;
          }

          if (read (sd, &msg,sizeof(msg)) < 0)
          {
            perror ("[client]Eroare la read() de la server.\n");
            return errno;
          }
        }
        printf ("[client] %s\n", msg);
      }
      /* afisam mesajul primit */
      else printf ("[client] %s\n", msg);
  }
  return 0;
}

void Grafica()
{
  char msg[SIZE];
  sf::RenderWindow window(sf::VideoMode(800, 800), "Window");

  sf::Font font;
  if (!font.loadFromFile("/usr/share/fonts/type1/urw-base35/NimbusRoman-Regular.t1"))  /* Pathul  fontului */
  { 
    perror("[client] Eroare la path-ul fontului");
    return;
  }

  sf::Text text;
  text.setFont(font); /* selecteaza fontul pentru text*/
  text.setFillColor(sf::Color::White); /* culoarea scrisului */
  text.setCharacterSize(20); /* marimea fiecarui caracter */

  int x = 10, y = 10;
  text.setPosition(x, y); /* setez pozitia */

  while (window.isOpen())
  {
    x = 10; y = 10;

    /* updateaza fisierul soferului */ 
    strcpy(msg, "11mesaje");
    strcat(msg, (numeUtilizator + 5));

    /* trimiterea mesajului la server */
    if (write (sd,&msg,sizeof(msg)) <= 0)
    {
      perror ("[client]Eroare la write() spre server.\n");
      return ;
    }

    /* citirea raspunsului dat de server 
      (apel blocant pina cind serverul raspunde) */
    if (read (sd, &msg,sizeof(msg)) < 0)
    {
      perror ("[client]Eroare la read() de la server.\n");
      return ;
    }


    sf::Event event; /* Proceseaza ce evenimente mai sunt */
    while (window.pollEvent(event))
    {
      if (event.type == sf::Event::Closed)
      window.close();
    }

    window.clear(); /* curata ecranu*/

    strcpy(msg, strstr(msg, "\n"));
    text.setString(msg);    
    window.draw(text); // scriu textul 

    window.display(); /* updateaza Window */
    sleep(1);
  }
}

int Sofer() // daca vrea sa se deconecteze iesa din whileul infinit si o sa intre iar in bucla din main
{
  char msg[SIZE];
  char buf[SIZE];
  char *line = NULL;  /* forces getline to allocate with malloc */
  size_t len = 0;     /* ignored when line = NULL */
  int nr;

  //printf("Nume utilizator: %s\n", numeUtilizator);
  nr = atoi((numeUtilizator + 5));
  //printf("Numar: %d\n", nr);

  while(1)
  {   
      if(logat == 1) printf("\n[client] Comenzi permise: mesaje -- logout -- quit\n");
      else return 0;

        /* citirea mesajului */
      printf ("[client] Introduceti comanda: ");
      if( (getline(&line, &len, stdin)) == -1)   
      {     printf("Eroare la citire in client\n");
      }
      fflush (stdout);
      strcpy(buf, line);

      /* pregatirea mesajului */
        msg[0] = tip_client + '0';
        msg[1] = logat + '0';
        msg[2] = '\0';
        strcat(msg, buf);
        msg[strlen(msg) - 1] = '\0';

        itoa(nr);
        strcat(msg, numar); /* e soferul nr */

    // printf("[client] Am trimis %s\n",msg);

      /* trimiterea mesajului la server */
      if (write (sd,&msg,sizeof(msg)) <= 0)
        {
          perror ("[client]Eroare la write() spre server.\n");
          return errno;
        }


      /* citirea raspunsului dat de server 
        (apel blocant pina cind serverul raspunde) */
      if (read (sd, &msg,sizeof(msg)) < 0)
      {
        perror ("[client]Eroare la read() de la server.\n");
        return errno;
      }


      if(strncmp(msg, "Sunteti delogat!", 16) == 0) Reset(); 
      else if(strncmp(msg, "Update", 6) == 0)
        Grafica();
      else if(strncmp(msg , "quit", 4) == 0) {quit = 1; printf("La revedere!\n"); return 0 ;}

      /* afisam mesajul primit */
      else printf ("[client] %s\n", msg);
  }
  return 0;
}

int main (int argc, char *argv[])
{
  struct sockaddr_in server;	// structura folosita pentru conectare 
  		// mesajul trimis
  int nr=0;
  char buf[SIZE];
  char msg[SIZE];

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

  char *line = NULL;  /* forces getline to allocate with malloc */
  size_t len = 0;     /* ignored when line = NULL */

  while(1)
{
  strcpy(msg, "");
  strcpy(buf, "");

  /*PARTEA DE LOGARE*/
    if(logat == 0) printf("\n[client] Comenzi permise: login:name -- quit\n");
    else if(logat == 1)
            if(tip_client == 0) { Client(); if(quit == 1) break; printf("\n[client] Comenzi permise: login:name -- quit\n");}
            else { Sofer(); if(quit == 1) break; printf("\n[client] Comenzi permise: login:name -- quit\n"); }

  /* citirea mesajului */
  printf ("[client] Introduceti comanda: ");
  if( (getline(&line, &len, stdin)) == -1)   
  {     printf("Eroare la citire in client\n");
  }
  fflush (stdout);
  strcpy(buf, line);
  //read (0, buf, sizeof(buf));

  /* pregatirea mesajului */
    msg[0] = tip_client + '0';
    msg[1] = logat + '0';
    msg[2] = '\0';
    strcat(msg, buf);
    msg[strlen(msg) - 1] = '\0';

 // printf("[client] Am trimis %s\n",msg);

  /* trimiterea mesajului la server */
  if (write (sd,&msg,sizeof(msg)) <= 0)
    {
      perror ("[client]Eroare la write() spre server.\n");
      return errno;
    }
  

  /* citirea raspunsului dat de server 
     (apel blocant pina cind serverul raspunde) */
  if (read (sd, &msg,sizeof(msg)) < 0)
  {
    perror ("[client]Eroare la read() de la server.\n");
    return errno;
  }

  if(strncmp(msg, "quit", 4) == 0) 
  {
    printf("La revedere!\n");
    break;
  }
  if(strcmp(msg, "Parola:") == 0) // a gasit numele si asteapta parola
  {
    /*Clientul scrie parola*/
    printf("[client] Introduceti parola : ");
    if( (getline(&line, &len, stdin)) == -1)  printf("Eroare la citire in client\n");
    fflush (stdout);
    strcpy(buf, line);

    strcpy(msg, buf);
    msg[strlen(msg) - 1] = '\0';
    
    /*Trimitem parola*/
    if (write (sd,&msg,sizeof(msg)) <= 0) 
    {
      perror ("[client] Eroare la write() spre server.\n");
      return errno;
    }

    /*Primim raspunsul final*/
    if (read (sd, &msg,sizeof(msg)) < 0)
    {
      perror ("[client] Eroare la read() de la server.\n");
      return errno;
    }

    if(strncmp(msg, "10", 2) == 0)
    {
      logat = 1;
      tip_client = 0;
      strcpy(numeUtilizator, (msg+2));
      printf ("[client] Calatorul a fost logat!\n");
    }
    else if(strncmp(msg, "00", 2) == 0)
    {
      logat = 0;
      tip_client = 0;
      printf ("[client] Parola data este gresita!\n");
    }
    else if(strncmp(msg, "11", 2) == 0)
    {
      logat = 1;
      tip_client = 1;
      strcpy(numeUtilizator, (msg+2));
      printf ("[client] Soferul a fost logat!\n");
    }
    else {
      logat = 0;
      tip_client = 0;
      printf ("[client] Userul nu a fost gasit!\n");
    }

  }
  else 
    /* afisam mesajul primit */
    printf ("[client] %s!\n", msg);
}

  /* inchidem conexiunea, am terminat */
  close (sd);
}