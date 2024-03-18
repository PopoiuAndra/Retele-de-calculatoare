/* servTCPConcTh2.c - Exemplu de server TCP concurent care deserveste clientii
   prin crearea unui thread pentru fiecare client.
   Asteapta un numar de la clienti si intoarce clientilor numarul incrementat.
	Intoarce corect identificatorul din program al thread-ului.
  
   
   Autor: Lenuta Alboaie  <adria@info.uaic.ro> (c)
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
#include <time.h>

/* portul folosit */
#define PORT 2909
#define SIZE 2048

/* codul de eroare returnat de anumite apeluri */
extern int errno;

pthread_mutex_t mtx, detRuta, alege_optiunea; 

typedef struct thData{
	int idThread; //id-ul thread-ului tinut in evidenta de acest program
	int cl; //descriptorul intors de accept
}thData;

static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
void raspunde(void *);
void Client(char msg[SIZE], void *);
void Sofer(char msg[SIZE]);
int Logat(char msg[SIZE], void *);


int graf[10][10];
int muchii[8][2] = { {1, 3}, {3, 5}, {5, 4}, {4, 2}, {5, 6}, {6, 7}, {6, 8}, {8, 9} };
char nume[100];
char nume_statie[10][100] = {"", "st1", "st2", "st3", "st4", "st5", "st6", "st7", "st8", "st9"}; 
int rute[4][7]={ /* ruta[i][0] = nr de statii din ruta */
  {},
  {4, 3, 5, 6, 8},
  {6, 2, 4, 5, 6, 8, 9},
  {5, 1, 3, 5, 6, 7}
};
int rute_accesibile_din_statii[10][4]={ /*ruta 1, 2 sau 3 pentru fiecare statie*/
{0, 0,0,0},
{1, 0,0,1},
{2, 0,1,0},
{3, 1,0,1},
{4, 0,1,0},
{5, 1,1,1},
{6, 1,1,1},
{7, 0,0,1},
{8, 1,1,0},
{9, 0,1,0},
};
int autobuze[7][4]={ /* [ruta] [statie de plecare] [statie finala] [directie] */
  {},
  {1, 3, 8, 1},
  {1, 8, 3, -1},
  {2, 2, 9, 1},
  {2, 9, 2, -1},
  {3, 1, 7, 1},
  {3, 7, 1, -1}
};
int grad_ocupare[8];
int grad_ocupare_ajutor[8];

int main ()
{
  struct sockaddr_in server;	// structura folosita de server
  struct sockaddr_in from;	
  int nr;		//mesajul primit de trimis la client 
  int sd;		//descriptorul de socket 
  int pid;
  pthread_t th[100];    //Identificatorii thread-urilor care se vor crea
	int i=0;

   if (pthread_mutex_init(&mtx, NULL) != 0) { 
     printf("\n mutex init has failed\n"); 
      return 1; 
      } 

      if (pthread_mutex_init(&detRuta, NULL) != 0) { 
     printf("\n mutex init has failed\n"); 
      return 1; 
      } 

      if (pthread_mutex_init(&alege_optiunea, NULL) != 0) { 
     printf("\n mutex init has failed\n"); 
      return 1; 
      } 
      
  
  for(i = 0; i <= 9; ++ i)
    graf[muchii[i][0]][muchii[i][1]] = graf[muchii[i][1]][muchii[i][0]] = 1;

  for(i = 1; i <= 6; ++ i) 
    grad_ocupare[i] = grad_ocupare_ajutor[i] = 0;
  i = 0;

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
      unsigned int length = sizeof (from);

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

 pthread_mutex_destroy(&mtx); 
 pthread_mutex_destroy(&detRuta); 
 pthread_mutex_destroy(&alege_optiunea); 
};				

static void *treat(void * arg)
{		
		struct thData tdL; 
		tdL= *((struct thData*)arg);	
        while(1)
        {
		    printf ("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
		    fflush (stdout);		 
		    pthread_detach(pthread_self());	
		    raspunde((struct thData*)arg);
        }
        close ((intptr_t)arg);/* am terminat cu acest client, inchidem conexiunea */
		return(NULL);	
  		
};

char numar[100];
void itoa(int nr)
{
  int n = 0, aux;
  if(nr == 0)
  { numar[0] = '0', numar[1] = '\0'; return;} 

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

void raspunde(void *arg)
{
    int i=0;
    char msg[SIZE];
	struct thData tdL; 
	tdL= *((struct thData*)arg);

	while (read (tdL.cl, &msg,sizeof(msg)) <= 0){}


	printf ("[Thread %d]Mesajul a fost receptionat...%s\n",tdL.idThread, msg);

  if(msg[1] == '0') // nu e logat
  {
      if(strncmp((msg+2), "login:", 6) == 0) 
      {
        int logat = Logat((msg+8), (struct thData*)arg);

        if(logat == 0) {strcpy(msg, "10");strcat(msg, nume);}
        else if (logat == 10) {strcpy(msg, "00");strcat(msg, nume);}
        else if (logat == 1) {strcpy(msg, "11"); strcat(msg, nume);}
        else if (logat == 11) {strcpy(msg, "00"); strcat(msg, nume);}
        else strcpy(msg, "Nume de utilizator incorect");


        printf("\nnume: %s\n", nume);
      }
      else if(strncmp((msg+2), "quit", 4) == 0)
        strcpy(msg, "quit");
      else strcpy(msg, "Comanda nu a fost gasita!");
  }
  else /*e logat si vedem ce tip de client e*/ 
    if(msg[0] == '0') Client(msg, (struct thData*)arg);
    else if(msg[0] == '1') Sofer(msg);

	printf("[Thread %d]Trimitem mesajul inapoi...%s\n",tdL.idThread, msg);
		      
		      
		      /* returnam mesajul clientului */
	 if (write (tdL.cl, &msg, sizeof(msg)) <= 0)
		{
		 printf("[Thread %d] ",tdL.idThread);
		 perror ("[Thread]Eroare la write() catre client.\n");
		}
	else
		printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",tdL.idThread);	

}

int Logat(char msg[SIZE], void * arg)
{
    pthread_mutex_lock(&mtx); 

    struct thData tdL; 
	  tdL= *((struct thData*)arg);
    char parolaPrimita[SIZE];


    FILE *file1 = fopen("loginCalatori.txt", "r");
    FILE *file2 = fopen("loginSoferi.txt", "r");

    char line[100];
    int i;

    if(file1 == NULL)
    {
       perror("EROARE: nu am putut deschide fisierul loginCalatori.txt");
       return 2;
    }

    /* Se cauta oruntre caltori */
    while (fgets(line, 100, file1))
    {

        for(i = 0; line[i] != ':'; ++ i)
          nume[i] = line[i];
        nume[i] = '\0';

        //printf("Am gasit numele: %s\n", nume);
        if(strcmp(nume, msg) == 0)
        {
          char *parola = (strchr(line,':')+1);
          parola[strlen(parola)-1] = '\0';
          printf("Am gasit parola: %s\nVerificam parola!\n", parola);

          strcpy(parolaPrimita, "Parola:");
          if (write (tdL.cl, &parolaPrimita, sizeof(parolaPrimita)) <= 0)
	      	{
		        printf("[Thread %d] ",tdL.idThread);
		        perror ("[Thread]Eroare la write() catre client.\n");
		      }
	        else
		        printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",tdL.idThread);	
          
          while (read (tdL.cl, &parolaPrimita,sizeof(parolaPrimita)) <= 0){}
          

          pthread_mutex_unlock(&mtx); 
          if(strcmp(parolaPrimita, parola) == 0) return 0; // parola e buna
          else return 10; // parola nu e buna
        }
    }

    if(file2 == NULL)
    {
       perror("EROARE: nu am putut deschide fisierul loginCalatori.txt");
       return 2;
    }

    /* Se cauta printre soferi*/
    while (fgets(line, 100, file2))
    {

        for(i = 0; line[i] != ':'; ++ i)
          nume[i] = line[i];
        nume[i] = '\0';

        //printf("Am gasit numele: %s\n", nume);
        if(strcmp(nume, msg) == 0)
        {
          char *parola = (strchr(line,':')+1);
          parola[strlen(parola)-1] = '\0';
          printf("Am gasit parola: %s\nVerificam parola!\n", parola);

          strcpy(parolaPrimita, "Parola:");
          if (write (tdL.cl, &parolaPrimita, sizeof(parolaPrimita)) <= 0)
	      	{
		        printf("[Thread %d] ",tdL.idThread);
		        perror ("[Thread]Eroare la write() catre client.\n");
		      }
	        else
		        printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",tdL.idThread);	
          
          while (read (tdL.cl, &parolaPrimita,sizeof(parolaPrimita)) <= 0){}

          printf("Parola primita: %s\n", parolaPrimita);

          pthread_mutex_unlock(&mtx); 

          if(strcmp(parolaPrimita, parola) == 0) return 1; // parola e buna
          else return 11; // parola nu e buna
        }
    }
  fclose(file1);
  fclose(file2);

  pthread_mutex_unlock(&mtx); 

  return 2;
}

int vizitat[10], tata[10];
int vector_rute[10][10],ruta_posibila[10], m;

void ScrieInSofer(int optiune[4][10],int  nr, int ok) /* ok = 1 -> necesita asistenta*/
{

  pthread_mutex_lock(&mtx); 
  int i;
  for(i = 1; i <= optiune[nr][0]; i += 2)
  {
    printf("Autobuz: %d ", (optiune[nr][i]));
    printf("Timp: %d \n", optiune[nr][i+1]);

    /* scrie in sofer(optiune[nr][i]).txt (ora + optiune[nr][i+1]) - ora la care expira mesajul*/
    char fisier[100];
    strcpy(fisier, "sofer");
    itoa(optiune[nr][i]);
    strcat(fisier, numar);
    strcat(fisier, ".txt");

    printf("Fisier: %s\n", fisier);
    
    /* determinam ora de expirare*/
    int ore = 0, minute = 0, j = 0;
    time_t givemetime = time(NULL);
    char ora_actuala[100];
    strcpy(ora_actuala, ctime(&givemetime));
    strncpy(ora_actuala, (ora_actuala + 11), 5);
    ora_actuala[5] = '\0'; /* am determinat ora actuala*/

    while(ora_actuala[j] != ':')
      ore = ora_actuala[j++] - '0' + ore *10;
    j++;
    while(ora_actuala[j] != '\0')
      minute = ora_actuala[j++] - '0' + minute *10;

    minute += optiune[nr][i+1];
    ore += minute / 60;
    minute = minute % 60;

    char linie[100];
    itoa(ore);
    strcpy(linie, numar);
    strcat(linie, ":");
    itoa(minute);
    strcat(linie, numar);
    strcat(linie, "-");
    if(ok == 1)
    {
      grad_ocupare_ajutor[optiune[nr][i]] ++;
      printf("Autobuz: %d Grad ocupare AJ: %d\n", i, grad_ocupare_ajutor[optiune[nr][i]]);
      strcat(linie, "Avem un calator care are nevoie de asistenta!\n");
    }
    else 
    {
      grad_ocupare[optiune[nr][i]] ++;
      printf("Autobuz: %d Grad ocupare: %d\n", i, grad_ocupare[optiune[nr][i]]);
      strcat(linie, "Avem un calator!\n");
    }

    printf("linie:%s|\n", linie);

    FILE *file = fopen(fisier, "a");
    if(file == NULL)
    {
       perror("EROARE: nu am putut deschide fisierul loginCalatori.txt");
       pthread_mutex_unlock(&mtx); 
       return;
    }
    else
    {
      fprintf(file, linie);
      fclose(file);
    }

  }
  printf("\n");

  pthread_mutex_unlock(&mtx); 
}

void DFS(int x, int t)
{
  vizitat[x] = 1; 
  tata[x] = t; 
  for(int i = 1; i <= 9; ++ i) 
    if( graf[x][i] == 1 && vizitat[i] == 0 )
      DFS(i , x);
}

void Backtracking(int x, int drum[], int n , int ruta_din_care_vin)
{
  if(x == n) {
    m ++;

    // printf("Rute folosite: %d rute:", ruta_posibila[0]);
    vector_rute[m][0] = ruta_posibila[0];
    for(int i = 1; i <= n; ++ i)
    {
      vector_rute[m][i] = ruta_posibila[i];
      // printf("%d ", vector_rute[m][i]);
    }
    // printf("\n");

    return;
  } 
  if( rute_accesibile_din_statii[drum[x+1]][ruta_din_care_vin] == 1 ) /* incerc sa continui cu ruta existenta*/
  {
    ruta_posibila[x + 1] = ruta_din_care_vin;
    Backtracking(x + 1, drum, n, ruta_din_care_vin);
  }
  else 
  for(int i = 1; i <= 3; ++ i) /* incerc si alte rute */
      if( i != ruta_din_care_vin && rute_accesibile_din_statii[drum[x+1]][i] == 1 ) /* daca o ruta e accesibila din statia x o incercam*/
      {
        ruta_posibila[x + 1] = i;
        ruta_posibila[0] ++;
        Backtracking(x + 1, drum, n, i);
        ruta_posibila[0] --;
      }

}

char timp_de_parcurs[SIZE];

int PozitieAutobuz(int x, int st, int nextst, int timp_de_add)
{
  time_t givemetime = time(NULL);
  char ora_actuala[100];
  strcpy(ora_actuala, ctime(&givemetime));
  strncpy(ora_actuala, (ora_actuala + 11), 5);
  ora_actuala[5] = '\0'; /* am determinat ora actuala*/

  //printf("ORA ACTUALA: %s\n", ora_actuala);
  printf("RUTA: %d ", autobuze[x][0]);
  //printf("NR DE STATII DIN RUTA: %d\n", rute[autobuze[x][0]][0]);

  int i = 0, ore = 0, minute = 0;
  while(ora_actuala[i] != ':')
    ore = ora_actuala[i++] - '0' + ore *10;
  i++;
  while(ora_actuala[i] != '\0')
    minute = ora_actuala[i++] - '0' + minute *10;
  printf("ORE: %d MINUTE: %d ", ore, minute);
  minute += timp_de_add;

  int ture = ((ore - 8) * 60 + minute) / ((rute[autobuze[x][0]][0] - 1) * 3 * 2);
  int minute_ramase = ((ore- 8) * 60 + minute) - ture * ((rute[autobuze[x][0]][0] - 1) * 3 * 2);
  printf("TURE: %d MINUTE RAMASE:%d\n", ture, minute_ramase);

  int directie = autobuze[x][3];
  if(minute_ramase < ((rute[autobuze[x][0]][0] - 1) * 3))
  {
    printf("RUTA: %d-%d\n", autobuze[x][1], autobuze[x][2]);
  }
  else 
  {
    printf("RUTA: %d-%d\n", autobuze[x][2], autobuze[x][1]);
    directie = (-1) * directie;
    minute_ramase -= (rute[autobuze[x][0]][0] - 1) * 3;
  }

  if( directie == 1) /*NU E BUN!!!!!!!!!!!!!!!!!1*/
  {
    int ruta_autobuzului_actual = autobuze[x][0];
    int a_cata_statie_e_din_ruta_actuala = minute_ramase / 3 + 1;
    int numele_statiei_de_care_tocmai_a_trecut_atuobuzul = rute[ruta_autobuzului_actual][minute_ramase / 3 + 1];
    printf("E trecut de %d minute de statia: %d[%d]\n", minute_ramase % 3, numele_statiei_de_care_tocmai_a_trecut_atuobuzul, a_cata_statie_e_din_ruta_actuala);
    /* Hai sa ajungem in statia st*/

    int ok = 0;
    int timp_de_ajungere_in_statia_st = 3 - minute_ramase % 3; /* timpul dintre statia abia trecuta si urmatoare*/
    for(i = a_cata_statie_e_din_ruta_actuala + 1; i <= rute[ruta_autobuzului_actual][0]; ++ i) /* trec prin toate statiile*/
        if(rute[ruta_autobuzului_actual][i] == st){
          printf("asta e statia cautata tata %d\n", rute[ruta_autobuzului_actual][i] );
          if(nextst == rute[ruta_autobuzului_actual][i + 1] || i == rute[ruta_autobuzului_actual][0])    
            return timp_de_ajungere_in_statia_st;
        }
        else { 
          printf("Am trecut prin statia: %d\n", rute[ruta_autobuzului_actual][i]); 
          timp_de_ajungere_in_statia_st += 3;
        }
    timp_de_ajungere_in_statia_st -= 3;
    printf("Timp dupa prima parcurgere: %d\nok: %d\n", timp_de_ajungere_in_statia_st, ok);

    if(ok == 0)
      for(i = 1; i < rute[ruta_autobuzului_actual][0]; ++ i)
        if(rute[ruta_autobuzului_actual][i] == st)
        {
          if(nextst == rute[ruta_autobuzului_actual][i + 1])
            ok = 2;
          break;
        }

    if(ok == 0) /* nu am trecut prin statia cautata */
    {
      for(i = rute[ruta_autobuzului_actual][0]; i > 0; -- i) /* trec prin toate statiile*/
        if(rute[ruta_autobuzului_actual][i] == st){
          printf("asta e statia cautata tata %d\n", rute[ruta_autobuzului_actual][i] );
          printf("Timp dupa a doua parcurgere:%d\n", timp_de_ajungere_in_statia_st);
          return timp_de_ajungere_in_statia_st;
          break;
        }
        else { printf("Am trecut prin statia: %d\n", rute[ruta_autobuzului_actual][i]); timp_de_ajungere_in_statia_st += 3;}
    }
    else if(ok == 2)
    {
      timp_de_ajungere_in_statia_st += (rute[ruta_autobuzului_actual][0] - 1) * 3;
      for(i = 1; i <= rute[ruta_autobuzului_actual][0]; ++ i) /* trec prin toate statiile*/
        if(rute[ruta_autobuzului_actual][i] == st){
          printf("asta e statia cautata tata %d\n", rute[ruta_autobuzului_actual][i] );

          return timp_de_ajungere_in_statia_st;
          break;
        }
        else { 
          printf("Am trecut prin statia: %d\n", rute[ruta_autobuzului_actual][i]); 
          timp_de_ajungere_in_statia_st += 3;
        }
    }
    printf("Ca sa ajung in statia vruta[%d] s au facut : %d minute\n", st, timp_de_ajungere_in_statia_st );
  }
  else{
    int ruta_autobuzului_actual = autobuze[x][0];
    int a_cata_statie_e_din_ruta_actuala = rute[autobuze[x][0]][0] - (minute_ramase / 3 );
    int numele_statiei_de_care_tocmai_a_trecut_atuobuzul = rute[ruta_autobuzului_actual][a_cata_statie_e_din_ruta_actuala];
    printf("E trecut de %d minute de statia: %d[%d]\n", minute_ramase % 3, numele_statiei_de_care_tocmai_a_trecut_atuobuzul, a_cata_statie_e_din_ruta_actuala);
    /* Hai sa ajungem in statia st*/

    int ok = 0;
    int timp_de_ajungere_in_statia_st = 3 - minute_ramase % 3; /* timpul dintre statia abia trecuta si urmatoare*/
    for(i = a_cata_statie_e_din_ruta_actuala - 1; i > 0; -- i) /* trec prin toate statiile*/
        if(rute[ruta_autobuzului_actual][i] == st){
          printf("asta e statia cautata tata %d\n", rute[ruta_autobuzului_actual][i] );
          if(nextst == rute[ruta_autobuzului_actual][i - 1]  || i == 1)
           return timp_de_ajungere_in_statia_st;
        }
        else { 
          printf("Am trecut prin statia: %d\n", rute[ruta_autobuzului_actual][i]); 
          timp_de_ajungere_in_statia_st += 3;
        }
    timp_de_ajungere_in_statia_st -= 3;
    if(ok == 0)
      for(i = rute[ruta_autobuzului_actual][0]; i > 1; -- i)
        if(rute[ruta_autobuzului_actual][i] == st)
        {
          if(nextst == rute[ruta_autobuzului_actual][i - 1])
            ok = 2;
          break;
        }
    printf("Timp dupa prima parcurgere: %d\nok: %d\n", timp_de_ajungere_in_statia_st, ok);

    if(ok == 0) /* nu am trecut prin statia cautata */
    {
      for(i = 1; i <= rute[ruta_autobuzului_actual][0]; ++ i) /* trec prin toate statiile*/
        if(rute[ruta_autobuzului_actual][i] == st){
          printf("asta e statia cautata tata %d\n", rute[ruta_autobuzului_actual][i] ); 
          printf("Timp dupa a doua parcurgere:%d\n", timp_de_ajungere_in_statia_st);
          return timp_de_ajungere_in_statia_st;
          break;
        }
        else {  printf("Am trecut prin statia: %d\n", rute[ruta_autobuzului_actual][i]); timp_de_ajungere_in_statia_st += 3;}
    }
    else if(ok == 2)
    {
      timp_de_ajungere_in_statia_st += (rute[ruta_autobuzului_actual][0] - 1) * 3;
      for(i = rute[ruta_autobuzului_actual][0]; i > 0; -- i) /* trec prin toate statiile*/
        if(rute[ruta_autobuzului_actual][i] == st){
          printf("asta e statia cautata tata %d\n", rute[ruta_autobuzului_actual][i] );
          return timp_de_ajungere_in_statia_st;
          break;
        }
        else { 
          printf("Am trecut prin statia: %d\n", rute[ruta_autobuzului_actual][i]); 
          timp_de_ajungere_in_statia_st += 3;
        }
    printf("Ca sa ajung in statia vruta[%d] s au facut : %d minute\n", st, timp_de_ajungere_in_statia_st );
  }
}
return -1;
}

void Client(char msg[SIZE], void *arg)
{
  int i;
	struct thData tdL; 
	tdL= *((struct thData*)arg);
  if( strcmp((msg + 2), "logout") == 0 ) strcpy(msg, "Sunteti delogat!");
  else if( strcmp((msg + 2), "quit") == 0 )  strcpy(msg, "quit");
  else if( strncmp( (msg + 2), "pozitie_actuala:", 16) == 0 )
  {
    char statie[100];
    strcpy(statie, (msg + 18));
    for(i = 1; i < 10; ++ i) /* Cauta statia data in statiile existente*/
    {
      if(strcmp(statie, nume_statie[i]) == 0)
      { strcpy(msg, "Statie actuala:");
        strcat(msg, nume_statie[i]);
        return;
      }
    }
    
    strcpy(msg, "Statia nu exista!\nStatii:\n");
    for(i = 1; i< 10; ++ i)
    {
      strcat(msg, nume_statie[i]);
      strcat(msg, "\n");
    }
  }
  else if( strncmp( (msg + 2), "destinatie:", 11) == 0 )
  {
    char statie[100];
    strcpy(statie, (msg + 13) );
    printf("Statie: %s:\n", statie);
    for(i = 1; i < 10; ++ i) /* Cauta statia data in statiile existente*/
    {
      if(strcmp(statie, nume_statie[i]) == 0)
      { strcpy(msg, "Destinatie:");
        strcat(msg, nume_statie[i]);
        return;
      }
    }

    strcpy(msg, "Statia nu exista!\nStatii:\n");
    for(i = 1; i< 10; ++ i)
    {
      strcat(msg, nume_statie[i]);
      strcat(msg, "\n");
    }

  }
  else if( strncmp( (msg + 2), "determina_ruta", 14) == 0)
  {
    
    char pozitie_actuala[100], destinatie[100];
    int optiuni[4][10]={}; /* [optiunea] [autobuz + timp] */

    strcpy(destinatie, (strchr( (strchr(msg, ':') + 1), ':') + 1) );
    strcpy(pozitie_actuala, (strchr(msg, ':') + 1));
    i = 0;
    while(pozitie_actuala[i] != ':')
      i++;
    pozitie_actuala[i] = '\0';

    if(strcmp(destinatie, "") == 0 || strcmp(pozitie_actuala, "") == 0)
      {
        strcpy(msg, "Nu ati oferit destule date!");
        return;
      }

    int start, finish;
    for(i = 1; i <= 9; ++ i)
      if(strcmp(pozitie_actuala, nume_statie[i]) == 0)
      {
        start = i;
        break;
      }
      
    for(i = 1; i <= 9; ++ i)
      if(strcmp(destinatie, nume_statie[i]) == 0)
      {
        finish = i;
        break;
      }

    for(i = 1; i <= 9; ++ i)
      vizitat[i] = tata[i] = 0;
    DFS(start, 0);

    int nod = finish,  aux;
    int n = 1;
    int drum[10];
    drum[1] = finish;
    while(tata[nod] != 0) /* aflu ruta*/
    {
      n++;
      drum[n] = tata[nod];
      nod = tata[nod];
    }
    for(i = 1; i <= n/2; ++ i) /* rearanjez ruta de la start la finish*/
    {
      aux = drum[i];
      drum[i] = drum[n - i + 1];
      drum[n - i + 1] = aux;
    }

    printf("DRUM: ");
    for(i = 1; i <= n; ++ i)
      printf("%d ", drum[i]);
    printf("\n");

    m = 0;
    for(i = 1; i <= 3; ++ i)
      if(rute_accesibile_din_statii[start][i] == 1)
      {
        ruta_posibila[1] = i;
        ruta_posibila[0] = 1; /* numarul de rute folosite */

        Backtracking(1, drum, n, i);

      }

    int minim = 3; /* maxim poate sa ia toate rutele intr o calatorie */
    for(i = 1; i <= m; ++ i) /* verific daca ruta e ok(daca e minimala si o trimit) */
      if(vector_rute[i][0] < minim) minim = vector_rute[i][0];

    
    strcpy(msg, "");

    int ruta_actuala, j, k, optiune = 0;
    for(i = 1; i <= m; ++ i) /* pentru fiecare ruta gasita*/
    { 
      if(vector_rute[i][0] == minim) /* daca este minima */
      { 
        int timp_total = 0; 
        ++optiune;
        itoa(optiune);
        strcat(timp_de_parcurs, "Optiunea ");
        strcat(timp_de_parcurs, numar);
        strcat(timp_de_parcurs, ": ");

        /* cautam cele mai apropiate autobuze*/
        for(j = 1; j <= n; ++ j) /* cautam pentru fiecare drumulet intre statii daca*/
          if(j == 1 || vector_rute[i][j] != vector_rute[i][j - 1]){ /* trebuie schimbat autobuzul */
            ruta_actuala = vector_rute[i][j]; /* ruta noua */
            if(j == 1)
            {
              printf("Statia in care se schimba:%d[%d]\n",  drum[j], ruta_actuala);
              itoa(drum[j]); /* pastrat in numar[] */
            }           
            else{ printf("Statia in care se schimba:%d[%d]\n",  drum[j -1], ruta_actuala);
                itoa(drum[j-1]); /* pastrat in numar[] */}

                strcat(timp_de_parcurs,"din statia ");
                strcat(timp_de_parcurs, numar);

            int autobuz_ales, timp_pana_in_statie = 100;
            for(k = 1; k <= 6; ++ k) /* pentru toate autobuzele */
              if(ruta_actuala == autobuze[k][0])
              { /* se cauta autobuzele de pe ruta aia*/
                if(j == 1)
                  aux = PozitieAutobuz(k, drum[j], drum[j + 1], timp_total);
                else 
                  aux = PozitieAutobuz(k, drum[j - 1], drum[j], timp_total);

                if(aux < timp_pana_in_statie) timp_pana_in_statie = aux, autobuz_ales = k;
              }

              printf("Autobuz ales de luat din statie: %d, timp: %d\n", autobuz_ales, timp_pana_in_statie); 
                timp_total += timp_pana_in_statie;
                strcat(timp_de_parcurs, " cu autobuzul ");
                itoa(autobuz_ales);
                strcat(timp_de_parcurs, numar);
                strcat(timp_de_parcurs, " in urm minute ");
                itoa(timp_pana_in_statie);
                strcat(timp_de_parcurs, numar);
                strcat(timp_de_parcurs, "\n");
                strcat(msg, timp_de_parcurs);
                strcpy(timp_de_parcurs, "");

              optiuni[optiune][++ optiuni[optiune][0]] = autobuz_ales;
              optiuni[optiune][++ optiuni[optiune][0]] = timp_total;

              if(j == 1) timp_total -= 3;
              timp_total += 3;
          }
          else timp_total+=3;
          
        /* VA URMA */
      strcat(timp_de_parcurs, "Timp total: ");
      itoa(timp_total);
      strcat(timp_de_parcurs, numar);
      strcat(timp_de_parcurs, "minute\n");
      strcat(msg, timp_de_parcurs);
      strcpy(timp_de_parcurs, "");
      printf("Timp total: %d\n", timp_total);
      }
    }


    if(optiune == 0) 
    {
      strcpy(msg, "Nu exista traseu");
      return;
    }

    char msg2[SIZE];
    strcpy(msg2, msg);
    printf("[Thread %d]Trimitem mesajul inapoi...%s\n",tdL.idThread, msg2);
		      
          // returnam mesajul clientului 
    if (write (tdL.cl, &msg2, sizeof(msg2)) <= 0)
    {
      printf("[Thread %d] ",tdL.idThread);
      perror ("[Thread]Eroare la write() catre client.\n");
    }
    else
      printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",tdL.idThread);	
    
    while (read (tdL.cl, &msg2,sizeof(msg2)) <= 0){}
    printf("[Thread %d] Mesajul primit...%s\n",tdL.idThread, msg2);

    pthread_mutex_lock(&alege_optiunea); 
    if(strcmp(msg2, "exit") == 0)
      {
        strcpy(msg2, "Am iesit din optiuni!");
      }
    else if(strncmp(msg2, "alege_optiunea:", 15) == 0)
    {

      int nr = atoi((strchr(msg2, ':') + 1));
      if(nr > optiune) strcpy(msg2, "Optiunea selectata nu exista!");
      else
      { 
      //printf("NR: %d\n", nr);
      strcpy(msg2, "Ai nevoie de asistenta?(Y/N)");

      if (write (tdL.cl, &msg2, sizeof(msg2)) <= 0)
      {
        printf("[Thread %d] ",tdL.idThread);
        perror ("[Thread]Eroare la write() catre client.\n");
      }
      else
        printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",tdL.idThread);	
      
      while (read (tdL.cl, &msg2,sizeof(msg2)) <= 0){}

      if(strcmp(msg2, "Y") == 0)
      {
        for(i = 1; i <= optiuni[nr][0];  i += 2)
          if(grad_ocupare_ajutor[optiuni[nr][i]] >= 5) 
           { strcpy(msg, "Nu mai sunt locuri pentru persoane care au nevoie de asistenta!");
             return;
           }
          else
          {
            printf("Autobuz: %d Grad ocupare AJ: %d\n", i, grad_ocupare_ajutor[optiuni[nr][i]]);
          }

        strcpy(msg2, "Locul a fost rezervat cu asistenta");
        ScrieInSofer(optiuni, nr, 1);
      }
      else if(strcmp(msg2, "N") == 0)
      {
        for(i = 1; i <= optiuni[nr][0];  i += 2)
          if(grad_ocupare[optiuni[nr][i]]+grad_ocupare_ajutor[optiuni[nr][i]] >= 30) 
           { strcpy(msg, "Nu mai sunt locuri!");
             return;
           }
          else
          {
            printf("Autobuz: %d Grad ocupare: %d\n", i, grad_ocupare[optiuni[nr][i]]);
          }

        //printf("Grad ocupare AJ: %d\n", grad_ocupare_ajutor[optiune[nr][i]]);
        strcpy(msg2, "Locul a fost rezervat fara asistenta");
        ScrieInSofer(optiuni, nr, 0);
      }
      else 
      {
        strcpy(msg2, "Comanda nu a fost gasita!");
      }
      }
    
    }
    else strcpy(msg2, "Comanda nu a fost gasita!");
      pthread_mutex_unlock(&alege_optiunea); 
    strcpy(msg, msg2);

  }
  else if( strcmp( (msg + 2), "harta") == 0) 
  {
    int x;
    strcpy(msg, "harta\n"); 
    /* trimitem msj de forma:*/
    /* 1:2-2:3:4 */
    /* 1 - nr rutei; 2 - orientarea; 3 - nr statiei ; 4 - cu cate minute am trecut de statie */
    
    for(x = 1; x <= 6; ++ x)
    {
      time_t givemetime = time(NULL);
      char ora_actuala[100];
      strcpy(ora_actuala, ctime(&givemetime));
      strncpy(ora_actuala, (ora_actuala + 11), 5);
      ora_actuala[5] = '\0'; /* am determinat ora actuala*/

      printf("RUTA: %d ", autobuze[x][0]);
      itoa(autobuze[x][0]);
      strcat(msg, numar);
      strcat(msg, ":");

      int i = 0, ore = 0, minute = 0;
      while(ora_actuala[i] != ':')
        ore = ora_actuala[i++] - '0' + ore *10;
      i++;
      while(ora_actuala[i] != '\0')
        minute = ora_actuala[i++] - '0' + minute *10;
      printf("ORE: %d MINUTE: %d ", ore, minute);

      int ture = ((ore - 8) * 60 + minute) / ((rute[autobuze[x][0]][0] - 1) * 3 * 2);
      int minute_ramase = ((ore- 8) * 60 + minute) - ture * ((rute[autobuze[x][0]][0] - 1) * 3 * 2);
      printf("TURE: %d MINUTE RAMASE:%d\n", ture, minute_ramase);

      int directie = autobuze[x][3];
      if(minute_ramase < ((rute[autobuze[x][0]][0] - 1) * 3))
      {
        printf("RUTA: %d-%d\n", autobuze[x][1], autobuze[x][2]);
        itoa(autobuze[x][1]);
        strcat(msg, numar);
        strcat(msg, "-");
        itoa(autobuze[x][2]);
        strcat(msg, numar);
        strcat(msg, ":");
      }
      else 
      {
        printf("RUTA: %d-%d\n", autobuze[x][2], autobuze[x][1]);
        itoa(autobuze[x][2]);
        strcat(msg, numar);
        strcat(msg, "-");
        itoa(autobuze[x][1]);
        strcat(msg, numar);
        strcat(msg, ":");

        directie = (-1) * directie;
        minute_ramase -= (rute[autobuze[x][0]][0] - 1) * 3;
      }

      if( directie == 1) /*NU E BUN!!!!!!!!!!!!!!!!!1*/
      {
        int ruta_autobuzului_actual = autobuze[x][0];
        int a_cata_statie_e_din_ruta_actuala = minute_ramase / 3 + 1;
        int numele_statiei_de_care_tocmai_a_trecut_atuobuzul = rute[ruta_autobuzului_actual][minute_ramase / 3 + 1];
        printf("E trecut de %d minute de statia: %d[%d]\n", minute_ramase % 3, numele_statiei_de_care_tocmai_a_trecut_atuobuzul, a_cata_statie_e_din_ruta_actuala);

        itoa(a_cata_statie_e_din_ruta_actuala);
        strcat(msg, numar);
        strcat(msg, ":");
        itoa((minute_ramase % 3));
        strcat(msg, numar);

      }
      else
      {
        int ruta_autobuzului_actual = autobuze[x][0];
        int a_cata_statie_e_din_ruta_actuala = rute[autobuze[x][0]][0] - (minute_ramase / 3 );
        int numele_statiei_de_care_tocmai_a_trecut_atuobuzul = rute[ruta_autobuzului_actual][a_cata_statie_e_din_ruta_actuala];
        printf("E trecut de %d minute de statia: %d[%d]\n", minute_ramase % 3, numele_statiei_de_care_tocmai_a_trecut_atuobuzul, a_cata_statie_e_din_ruta_actuala);

        itoa(a_cata_statie_e_din_ruta_actuala);
        strcat(msg, numar);
        strcat(msg, ":");
        itoa((minute_ramase % 3));
        strcat(msg, numar);
      }
      strcat(msg, "\n");
    }
  }
  else if( strcmp( (msg + 2), "informatii") == 0)
  {
    strcpy(msg, "\n");
    int x;
    for(x = 1; x <= 6; ++ x)
    {
      time_t givemetime = time(NULL);
      char ora_actuala[100];
      strcpy(ora_actuala, ctime(&givemetime));
      strncpy(ora_actuala, (ora_actuala + 11), 5);
      ora_actuala[5] = '\0'; /* am determinat ora actuala*/

      printf("RUTA: %d ", autobuze[x][0]);
      itoa(x);
      strcat(msg, "Autobuzul ");
      strcat(msg, numar);
      strcat(msg, "; ");

      int i = 0, ore = 0, minute = 0;
      while(ora_actuala[i] != ':')
        ore = ora_actuala[i++] - '0' + ore *10;
      i++;
      while(ora_actuala[i] != '\0')
        minute = ora_actuala[i++] - '0' + minute *10;
      printf("ORE: %d MINUTE: %d ", ore, minute);

      int ture = ((ore - 8) * 60 + minute) / ((rute[autobuze[x][0]][0] - 1) * 3 * 2);
      int minute_ramase = ((ore- 8) * 60 + minute) - ture * ((rute[autobuze[x][0]][0] - 1) * 3 * 2);
      printf("TURE: %d MINUTE RAMASE:%d\n", ture, minute_ramase);

      int directie = autobuze[x][3];
      if(minute_ramase < ((rute[autobuze[x][0]][0] - 1) * 3))
      {
        printf("RUTA: %d-%d\n", autobuze[x][1], autobuze[x][2]);
        strcat(msg, " Orientare ");
        itoa(autobuze[x][1]);
        strcat(msg, numar);
        strcat(msg, "-");
        itoa(autobuze[x][2]);
        strcat(msg, numar);
        strcat(msg, "; ");
      }
      else 
      {
        printf("RUTA: %d-%d\n", autobuze[x][2], autobuze[x][1]);
        strcat(msg, " Orientare ");
        itoa(autobuze[x][2]);
        strcat(msg, numar);
        strcat(msg, "-");
        itoa(autobuze[x][1]);
        strcat(msg, numar);
        strcat(msg, "; ");

        directie = (-1) * directie;
        minute_ramase -= (rute[autobuze[x][0]][0] - 1) * 3;
      }

      if( directie == 1) /*NU E BUN!!!!!!!!!!!!!!!!!1*/
      {
        int ruta_autobuzului_actual = autobuze[x][0];
        int a_cata_statie_e_din_ruta_actuala = minute_ramase / 3 + 1;
        int numele_statiei_de_care_tocmai_a_trecut_atuobuzul = rute[ruta_autobuzului_actual][minute_ramase / 3 + 1];
        printf("E trecut de %d minute de statia: %d[%d]\n", minute_ramase % 3, numele_statiei_de_care_tocmai_a_trecut_atuobuzul, a_cata_statie_e_din_ruta_actuala);

        strcat(msg, " Autobuzul a trecut de statia ");
        itoa(numele_statiei_de_care_tocmai_a_trecut_atuobuzul);
        strcat(msg, numar);
        strcat(msg, " in urma cu ");
        itoa((minute_ramase % 3));
        strcat(msg, numar);
        strcat(msg, "minute;");

      }
      else
      {
        int ruta_autobuzului_actual = autobuze[x][0];
        int a_cata_statie_e_din_ruta_actuala = rute[autobuze[x][0]][0] - (minute_ramase / 3 );
        int numele_statiei_de_care_tocmai_a_trecut_atuobuzul = rute[ruta_autobuzului_actual][a_cata_statie_e_din_ruta_actuala];
        printf("E trecut de %d minute de statia: %d[%d]\n", minute_ramase % 3, numele_statiei_de_care_tocmai_a_trecut_atuobuzul, a_cata_statie_e_din_ruta_actuala);

        strcat(msg, " Autobuzul a trecut de statia ");
        itoa(numele_statiei_de_care_tocmai_a_trecut_atuobuzul);
        strcat(msg, numar);
        strcat(msg, " in urma cu ");
        itoa((minute_ramase % 3));
        strcat(msg, numar);
        strcat(msg, "minute;");
      }
      strcat(msg, " Grad de ocupare: ");
      itoa(grad_ocupare[x]+grad_ocupare_ajutor[x]);
      strcat(msg, numar);
      strcat(msg, "; Grad de ocupare pentru persoane care solicita asistenta: ");
      itoa(grad_ocupare_ajutor[x]);
      strcat(msg, numar);
      strcat(msg, "\n");
    }
  }
  else strcpy(msg, "Comada nu a fost gasita!");
}

void Sofer(char msg[SIZE])
{ 
  if(strncmp((msg+2), "logout", 6) == 0) strcpy(msg, "Sunteti delogat!");
  else if(strncmp((msg+2), "mesaje", 6) == 0)
  {

    char nr[5] = {};
    int nr_sofer;
    strcpy(nr, (msg+8));
    nr_sofer = atoi(nr);

    printf("Capacitate before: %d %d\n", grad_ocupare[nr_sofer], grad_ocupare_ajutor[nr_sofer]);

    time_t givemetime = time(NULL);
    char ora_actuala[100];
    strcpy(ora_actuala, ctime(&givemetime));
    strncpy(ora_actuala, (ora_actuala + 11), 5);
    ora_actuala[5] = '\0'; /* am determinat ora actuala*/

    int i = 0, j, ore = 0, minute = 0, m = 0;
    while(ora_actuala[i] != ':')
      ore = ora_actuala[i++] - '0' + ore *10;
    i++;
    while(ora_actuala[i] != '\0')
      minute = ora_actuala[i++] - '0' + minute *10;


    pthread_mutex_lock(&mtx); 
    char fisier[100];
    strcpy(fisier, "sofer");
    strcat(fisier, nr);
    strcat(fisier, ".txt");

    strcpy(msg, "Update\n");

    char linii_bune[100][100], line[100];
    FILE *file = fopen(fisier, "r+");

    if(file == NULL)
    {
       perror("EROARE: nu am putut deschide fisierul sofernr.txt");
       return;
    }

    printf("Linii din fisierul: %s\n", fisier);
    while (fgets(line, 100, file))
    {
      strcpy(nr, "");
      printf("line: %s", line);

      for(j = 0; line[j] != ':'; ++ j)
        nr[j] = line[j];
      nr[j] = '\0';

      printf("Ora: %s ", nr);

      if(atoi(nr) >= ore) 
      {
        if(atoi(nr) == ore)
        {
          j ++;
          int aux = 0;
          while(line[j] != '-')
            nr[aux++] = line[j++];
          nr[aux] = '\0';

          printf("Minute: %s\n", nr);
        
          if(atoi(nr) > minute) 
            strcpy(linii_bune[++ m], line);
          else if(strstr(line, "Avem un calator care are nevoie de asistenta!") != NULL) /* e cu asistenta */
              grad_ocupare_ajutor[nr_sofer] --;
          else grad_ocupare[nr_sofer] --; 

        }
        else {printf("Ramane linia: %s", line);
          strcpy(linii_bune[++ m], line);
        }
      }
      else if(strstr(line, "Avem un calator care are nevoie de asistenta!") != NULL) /* e cu asistenta */
              grad_ocupare_ajutor[nr_sofer] --;
            else grad_ocupare[nr_sofer] --; 
    }

    if (ftruncate(fileno(file), 0) != 0) /* am golit fisierul*/
    {
      perror("EROARE: La trunchiere");
      return;
    }

    fflush(file);

    fseek(file, 0, SEEK_SET);

    for(i = 1; i <= m; ++ i)
    {   printf("%s", linii_bune[i]);
        strcat(msg, linii_bune[i]);

        if (fputs(linii_bune[i], file) == EOF) 
        {
            perror("EROAR: La scriere");
        }
    }

    fflush(file);
    fclose(file);

    pthread_mutex_unlock(&mtx); 
    
    printf("Capacitate after: %d %d\n", grad_ocupare[nr_sofer], grad_ocupare_ajutor[nr_sofer]);
  }
  else if( strncmp((msg + 2), "quit", 4) == 0) strcpy(msg, "quit");
  else strcpy(msg, "Comada nu a fost gasita!");
}