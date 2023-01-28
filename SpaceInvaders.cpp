#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include "GrilleSDL.h"
#include "Ressources.h"

// Dimensions de la grille de jeu
#define NB_LIGNE   18
#define LIGNE_VAISSEAU 17
#define NB_COLONNE 23 

// Direction de mouvement
#define GAUCHE       10
#define DROITE       11
#define BAS          12

// Intervenants du jeu (type)
#define VIDE        0
#define VAISSEAU    1
#define MISSILE     2
#define ALIEN       3
#define BOMBE       4
#define BOUCLIER1   5
#define BOUCLIER2   6
#define AMIRAL      7

typedef struct
{
  int type;
  pthread_t tid;
} S_CASE;

typedef struct 
{
  int L;
  int C;
} S_POSITION;

S_CASE tab[NB_LIGNE][NB_COLONNE];
int colonne; // variable position vaisseau
bool fireOn = true;
bool ok = false;
int nbAliens = 24; // param flotteAliens
int lh = 2;
int cg = 8;
int lb = 8;
int cd = 18;
int delay = 1000;
int flotte = -1;
bool MAJScore = false;
int score = 0;
int nbVies = 3;
int lvl = 1;
bool alarmStop = false;
void HandlerSigusr1(int sig);
void HandlerSigusr2(int sig);
void HandlerSighup(int sig);
void HandlerSigint(int sig);
void HandlerSigquit(int sig);
void HandlerSigalarme(int sig);
void HandlerSigchld(int sig);
struct sigaction gauche,droite,espace,missile,mortVaisseau,bombe,amiral,amiralSIGCHLD;
pthread_t Vaiss,Missile,TimeOutMissile,Event,FlotteAliens,Invader,Score,Bombe,Amiral; // identifiant different thread
pthread_mutex_t mutexGrille,mutexColonne,mutexFlotteAliens,mutexScore,mutexVie; // mutex de la grille
pthread_cond_t condScore,condVies,condFlotteAliens;
void *fctVaisseau(void *);
void *fctMissile(S_POSITION* mi);
void *fctTimeOut(void*);
void *fctEvent(void*);
void *fctInvader(void*);
void *fctFlotteAliens(void*);
void *fctScore(void*);
void *fctBombe(S_POSITION* bo);
void *fctAmiral(void*);
void modifyFlotte();
void afficheFlotte();
void supprimeAlien();
void moinsUneVie(void*);
void Attente(int milli);
void setTab(int l,int c,int type=VIDE,pthread_t tid=0);

///////////////////////////////////////////////////////////////////////////////////////////////////
void Attente(int milli)
{
  struct timespec del;
  del.tv_sec = milli/1000;
  del.tv_nsec = (milli%1000)*1000000;
  nanosleep(&del,NULL);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void setTab(int l,int c,int type,pthread_t tid)
{
  tab[l][c].type = type;
  tab[l][c].tid = tid;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc,char* argv[])
{
  
 
  srand((unsigned)time(NULL));

  // Ouverture de la fenetre graphique
  printf("(MAIN %ld) Ouverture de la fenetre graphique\n",pthread_self()); fflush(stdout);
  if (OuvertureFenetreGraphique() < 0)
  {
    printf("Erreur de OuvrirGrilleSDL\n");
    fflush(stdout);
    exit(1);
  }

  // Initialisation des mutex et variables de condition
  // TO DO
  if(pthread_mutex_init(&mutexGrille,NULL) != 0)
  {
    perror("Erreur creation mutex grille\n");
    exit(1);
  }
  if(pthread_mutex_init(&mutexColonne,NULL) != 0)
  {
    perror("Erreur creation mutex colonne\n");
    exit(1);
  }
  if(pthread_mutex_init(&mutexFlotteAliens,NULL) != 0)
  {
    perror("Erreur creation mutex flotteAliens\n");
    exit(1);
  }
  if( pthread_mutex_init(&mutexScore,NULL) != 0)
  {
    perror("Erreur creation mutex flotteAliens\n");
    exit(1);
  }
  if(pthread_mutex_init(&mutexScore,NULL) != 0)
  {
    perror("Erreur creation mutex score\n");
    exit(1);
  }
  pthread_cond_init(&condScore,NULL);
  pthread_cond_init(&condVies,NULL);
  pthread_cond_init(&condFlotteAliens,NULL);
  // Armement des signaux
  // TO DO
  droite.sa_handler = HandlerSigusr1;
  sigemptyset(&droite.sa_mask);
  droite.sa_flags = 0;
  sigaction(SIGUSR1,&droite,NULL);

  gauche.sa_handler = HandlerSigusr2;
  sigemptyset(&gauche.sa_mask);
  gauche.sa_flags = 0;
  sigaction(SIGUSR2,&gauche,NULL);

  espace.sa_handler = HandlerSighup;
  sigemptyset(&espace.sa_mask);
  espace.sa_flags = 0;
  sigaction(SIGHUP,&espace,NULL); 

  missile.sa_handler = HandlerSigint;
  sigemptyset(&missile.sa_mask);
  missile.sa_flags = 0;
  sigaction(SIGINT,&missile,NULL); 

  mortVaisseau.sa_handler = HandlerSigquit;
  sigemptyset(&mortVaisseau.sa_mask);
  mortVaisseau.sa_flags = 0;
  sigaction(SIGQUIT,&mortVaisseau,NULL); 

  bombe.sa_handler = HandlerSigint;
  sigemptyset(&bombe.sa_mask);
  bombe.sa_flags = 0;
  sigaction(SIGINT,&bombe,NULL);



  // Initialisation de tab
  for (int l=0 ; l<NB_LIGNE ; l++)
    for (int c=0 ; c<NB_COLONNE ; c++)
      setTab(l,c);

  // Initialisation des boucliers
  setTab(NB_LIGNE-2,11,BOUCLIER1,0);  DessineBouclier(NB_LIGNE-2,11,1);
  setTab(NB_LIGNE-2,12,BOUCLIER1,0);  DessineBouclier(NB_LIGNE-2,12,1);
  setTab(NB_LIGNE-2,13,BOUCLIER1,0);  DessineBouclier(NB_LIGNE-2,13,1);
  setTab(NB_LIGNE-2,17,BOUCLIER1,0);  DessineBouclier(NB_LIGNE-2,17,1);
  setTab(NB_LIGNE-2,18,BOUCLIER1,0);  DessineBouclier(NB_LIGNE-2,18,1);
  setTab(NB_LIGNE-2,19,BOUCLIER1,0);  DessineBouclier(NB_LIGNE-2,19,1);
  //initialisation des vie
  setTab(4,3,VAISSEAU,0);
  setTab(4,4,VAISSEAU,0);
  DessineVaisseau(16,4);
  DessineVaisseau(16,3);
  // Creation des threads
  // TO DO
  if(pthread_create(&Event,NULL,fctEvent,NULL) != 0)
  {
    perror("Error create thread event \n");
    exit(1);
  }
  if(pthread_create(&Vaiss,NULL,fctVaisseau,NULL) != 0)
  {
    perror("Error create thread vaisseau \n");
    exit(1);
  }
  if(pthread_create(&Invader,NULL,fctInvader,NULL) != 0)
  {
    perror("Error create thread invader \n");
    exit(1);
  }
  if(pthread_create(&Score,NULL,fctScore,NULL) != 0)
  {
    perror("Error create thread invader \n");
    exit(1);
  }
  if(pthread_create(&Amiral,NULL,fctAmiral,NULL) != 0)
  {
    perror("Error create thread invader \n");
    exit(1);
  }

  pthread_mutex_lock(&mutexVie);
  while(!ok && nbVies > 0)
  {
    pthread_cond_wait(&condVies,&mutexVie);
    if(pthread_create(&Vaiss,NULL,fctVaisseau,NULL) != 0)
    {
      perror("Error create thread vaisseau \n");
      exit(1);
    }
  }
  pthread_mutex_unlock(&mutexVie);
  if(nbVies == 0)
  {
    DessineGameOver(6,11);
    sleep(5);
  }

  exit(0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void HandlerSigchld(int sig){
  printf("Reception sigchld\n");
  alarm(0);
  alarmStop = true;
}
void HandlerSigalarme(int sig)
{
  printf("Reception sigalrm\n");
  alarmStop = true;
}
void *fctAmiral(void*){
  sigset_t mask;
  amiral.sa_handler = HandlerSigalarme;
  sigaction(SIGALRM,&amiral,NULL);

  amiralSIGCHLD.sa_handler = HandlerSigchld;
  sigaction(SIGCHLD,&amiralSIGCHLD,NULL);

  sigemptyset(&mask);
  sigaddset(&mask,SIGUSR1);
  sigaddset(&mask,SIGUSR2);
  sigaddset(&mask,SIGHUP);
  sigaddset(&mask,SIGQUIT);
  sigprocmask(SIG_SETMASK,&mask,NULL);
  int spawn;
  int nbalarm;
  int depl;
  while(1)
  {
    
    if(alarmStop)
    {
      pthread_mutex_lock(&mutexGrille);
      setTab(0,spawn,VIDE,0);
      setTab(0,spawn+1,VIDE,0);
      EffaceCarre(0,spawn);
      EffaceCarre(0,spawn +1);
      pthread_mutex_unlock(&mutexGrille);
    }

    alarmStop = false;
    pthread_mutex_lock(&mutexFlotteAliens);
    while((nbAliens%6) != 0 || (nbAliens == 0))
    {
      pthread_cond_wait(&condFlotteAliens,&mutexFlotteAliens);
    }
    pthread_mutex_unlock(&mutexFlotteAliens);
    spawn = rand()%14 +8;
    depl = rand()%2+1;
    printf("depl : %d\n",depl);
    nbalarm = rand()%8+4;
    printf("nb alarm : %d",nbalarm);
    
    if(tab[0][spawn].type == VIDE && tab[0][spawn+1].type == VIDE)
    {
      pthread_mutex_lock(&mutexGrille);
      setTab(0,spawn,AMIRAL,pthread_self());
      setTab(0,spawn+1,AMIRAL,pthread_self());
      DessineVaisseauAmiral(0,spawn);
      pthread_mutex_unlock(&mutexGrille);
      alarm(nbalarm);
      if(depl == 1) // deplacement a droite
      {
        while(!alarmStop)
        {
          while(!alarmStop && (spawn + 1) < 23)
          {
            Attente(200);
            pthread_mutex_lock(&mutexGrille);
            switch(tab[0][spawn + 2].type)
            {
              case 0:{
                setTab(0,spawn,VIDE,0);
                setTab(0,spawn+2,AMIRAL,pthread_self());
                EffaceCarre(0,spawn);
                DessineVaisseauAmiral(0,spawn+1);
                spawn++;
                pthread_mutex_unlock(&mutexGrille);
              }
              break;
              case 2:{
                Attente(50);
                pthread_mutex_unlock(&mutexGrille);
              }
              break;
            }
          }
          while(!alarmStop && (spawn - 1) > 7)
          {
            Attente(200);
            pthread_mutex_lock(&mutexGrille);
            switch(tab[0][spawn - 1].type)
            {
              case 0:{
                setTab(0,spawn+1,VIDE,0);
                setTab(0,spawn-1,AMIRAL,pthread_self()); 
                EffaceCarre(0,spawn+1);               
                DessineVaisseauAmiral(0,spawn-1);
                spawn--;
                pthread_mutex_unlock(&mutexGrille);
              }
              break;
              case 2:{
                Attente(50);
                pthread_mutex_unlock(&mutexGrille);
              }
              break;
            }
          }          
        }
      }
      else// deplacement a gauche
      {
        while(!alarmStop)
        {
          while(!alarmStop && (spawn - 1) > 7)
          {
            Attente(200);
            pthread_mutex_lock(&mutexGrille);
            switch(tab[0][spawn - 1].type)
            {
              case 0:{
                setTab(0,spawn+1,VIDE,0);
                setTab(0,spawn-1,AMIRAL,pthread_self());
                EffaceCarre(0,spawn+1);
                DessineVaisseauAmiral(0,spawn-1);
                spawn--;
                pthread_mutex_unlock(&mutexGrille);
              }
              break;
              case 2:{
                Attente(50);
                pthread_mutex_unlock(&mutexGrille);
              }
              break;
            }
          }
          while(!alarmStop && (spawn + 1) < 23)
          {
            Attente(200);
            pthread_mutex_lock(&mutexGrille);
            switch(tab[0][spawn + 2].type)
            {
              case 0:{
                setTab(0,spawn,VIDE,0);
                setTab(0,spawn+2,AMIRAL,pthread_self());
                EffaceCarre(0,spawn);
                DessineVaisseauAmiral(0,spawn+1);
                spawn++;
                pthread_mutex_unlock(&mutexGrille);
              }
              break;
              case 2:{
                Attente(50);
                pthread_mutex_unlock(&mutexGrille);
              }
              break;
            }
          }          
        }
      }
    }

  }

  pthread_exit(NULL);
}
void moinsUneVie(void*)
{
  printf("Moins une vie\n");
  pthread_mutex_lock(&mutexVie);
  nbVies--;
  if(nbVies == 2)
    EffaceCarre(16,4);
  if(nbVies == 1)
    EffaceCarre(16,3);
  pthread_mutex_unlock(&mutexVie);
  pthread_cond_signal(&condVies);
}
void *fctBombe(S_POSITION* bo){
  sigset_t mask;
  bombe.sa_handler = HandlerSigint;
  sigaction(SIGINT,&bombe,NULL);

  sigemptyset(&mask);
  sigaddset(&mask,SIGUSR1);
  sigaddset(&mask,SIGALRM);
  sigaddset(&mask,SIGCHLD);
  sigaddset(&mask,SIGUSR2);
  sigaddset(&mask,SIGHUP);
  sigaddset(&mask,SIGQUIT);
  sigprocmask(SIG_SETMASK,&mask,NULL);

  if(pthread_mutex_lock(&mutexGrille) != 0)
  {
    perror("Erreur lock mutex grille \n");
    exit(1);
  }
  switch(tab[bo->L][bo->C].type)
  {
    case 0: {
      // case vide
      //printf("Creation bombe case vide\n");
      setTab(bo->L, bo->C, BOMBE, pthread_self());
      DessineBombe(bo->L, bo->C);
      if(pthread_mutex_unlock(&mutexGrille) != 0)
      {
        perror("Erreur unlock mutex grille \n");
        exit(1);
      }
    }
    break;
    case 2:{
      //printf("Creation bombe case missile\n");
      pthread_kill(tab[bo->L][bo->C].tid,SIGINT);
      setTab(bo->L,bo->C,VIDE,0);
      EffaceCarre(bo->L,bo->C);
      if(pthread_mutex_unlock(&mutexGrille) != 0)
      {
        perror("Erreur unlock mutex grille \n");
        exit(1);
      }
      free(bo);
      pthread_exit(NULL);
    }
    break;
    case 5:{
      // bouclier 1
      //printf("Creation bombe case bouclier 1\n");
      setTab(bo->L,bo->C,VIDE,0);
      EffaceCarre(bo->L,bo->C);
      setTab(bo->L, bo->C , BOUCLIER2 , 0);
      DessineBouclier(bo->L, bo->C,2);
      if(pthread_mutex_unlock(&mutexGrille) != 0)
      {
        perror("Erreur unlock mutex grille \n");
        exit(1);
      }
      free(bo);
      pthread_exit(NULL);         
    }
    break;
    case 6:
    {
      //bouclier 2
      //printf("Creation bombe case bouclier 2\n");
      setTab(bo->L,bo->C,VIDE,0);
      EffaceCarre(bo->L,bo->C);
      if(pthread_mutex_unlock(&mutexGrille) != 0)
      {
        perror("Erreur unlock mutex grille \n");
        exit(1);
      }
      free(bo);
      pthread_exit(NULL); 
    }
  }
  Attente(160); 
  while(1)
  {
    if( pthread_mutex_lock(&mutexGrille) != 0)
    {
      perror("Erreur lock mutex grille \n");
      exit(1);
    }
    if(bo->L < NB_LIGNE-1)
    {
      switch(tab[bo->L + 1][bo->C].type)
      {
        case 0: {
          // case vide
          //printf("bombe avance case vide\n");
          setTab(bo->L,bo->C,VIDE,0);
          EffaceCarre(bo->L,bo->C);
          setTab(bo->L +1 , bo->C, BOMBE, pthread_self());
          DessineBombe(bo->L +1, bo->C);
          bo->L++;
          if(pthread_mutex_unlock(&mutexGrille) != 0)
          {
            perror("Erreur unlock mutex grille \n");
            exit(1);
          }
        }
        break;
        case 1:{
          printf("bombe avance case vaisseau\n");
          setTab(bo->L,bo->C,VIDE,0);
          EffaceCarre(bo->L,bo->C);
          kill(0,SIGQUIT);  
          if(pthread_mutex_unlock(&mutexGrille) != 0)
          {
            perror("Erreur unlock mutex grille \n");
            exit(1);
          }
          free(bo);
          pthread_exit(NULL);       
        }
        break;
        case 2:{
          printf("bombe avance case missile\n");
          pthread_kill(tab[bo->L+1][bo->C].tid,SIGINT);
          setTab(bo->L+1,bo->C,VIDE,0);
          EffaceCarre(bo->L+1,bo->C);
          setTab(bo->L,bo->C,VIDE,0);
          EffaceCarre(bo->L,bo->C);
          if(pthread_mutex_unlock(&mutexGrille) != 0)
          {
            perror("Erreur unlock mutex grille \n");
            exit(1);
          }
          free(bo);
          pthread_exit(NULL);
        }
        break;
        case 3:{
          // fait rien c'est un alien on attend un tour de plus
          if(pthread_mutex_unlock(&mutexGrille) != 0)
          {
            perror("Erreur unlock mutex grille \n");
            exit(1);
          }
        }
        break;
        case 5:{
          // bouclier 1
          setTab(bo->L,bo->C,VIDE,0);
          EffaceCarre(bo->L,bo->C);
          setTab(bo->L +1, bo->C , BOUCLIER2 , 0);
          EffaceCarre(bo->L +1, bo->C);
          DessineBouclier(bo->L +1, bo->C,2);
          if(pthread_mutex_unlock(&mutexGrille) != 0)
          {
            perror("Erreur unlock mutex grille \n");
            exit(1);
          }
          free(bo);
          pthread_exit(NULL);          
        }
        break;
        case 6:
        {
          //bouclier 2
          setTab(bo->L,bo->C,VIDE,0);
          EffaceCarre(bo->L,bo->C);
          setTab(bo->L +1, bo->C , VIDE , 0);
          EffaceCarre(bo->L +1, bo->C);
          if(pthread_mutex_unlock(&mutexGrille) != 0)
          {
            perror("Erreur unlock mutex grille \n");
            exit(1);
          }
          free(bo);
          pthread_exit(NULL);   
        }
      }
    }
    else
    {
      // detruit bombe fin de tab
      setTab(bo->L,bo->C,VIDE,0);
      EffaceCarre(bo->L,bo->C);      
      if(pthread_mutex_unlock(&mutexGrille) != 0)
      {
        perror("Erreur unlock mutex grille \n");
        exit(1);
      }
      free(bo);
      pthread_exit(NULL);  
    }
  Attente(160);
  } 
  pthread_exit(NULL);
}
void *fctScore(void*){
  DessineChiffre(10,2,0);
  DessineChiffre(10,3,0);
  DessineChiffre(10,4,0);
  DessineChiffre(10,5,0);
  pthread_mutex_lock(&mutexScore);
  int print,digit,i;
  while(!MAJScore)
  {
    i = 5;
    pthread_cond_wait(&condScore,&mutexScore);
    // augmenter 1 le score
    print = score;
    while(print != 0)
    {
      digit = print % 10;
      print = print / 10;
      DessineChiffre(10,i,digit);
      i--;
    }
    MAJScore = false;
  }
  pthread_mutex_unlock(&mutexScore);
  pthread_exit(NULL);
}
void *fctInvader(void*)
{
  bool fin = false;
  int i,print,digit;
  DessineChiffre(13,3,0);
  DessineChiffre(13,4,1);
  printf("Creation premiere flotte\n");
  if(pthread_create(&FlotteAliens,NULL,fctFlotteAliens,NULL) != 0)
  {
    perror("Error create thread flotteAliens \n");
    exit(1);
  }
  if(pthread_join(FlotteAliens,NULL) != 0)
  {
    perror("Erreur pthread join main\n");
    exit(1);
  }   
  while(!fin)
  {
    if(ok)
    { 
      pthread_exit(NULL);
    } 
    printf("cas vaut : %d",flotte);
    if(flotte == 1)
    {
      fin = true;
      if( pthread_mutex_lock(&mutexGrille) != 0)
      {
        perror("Erreur lock mutex grille \n");
        exit(1);
      }
      supprimeAlien();
      kill(0,SIGQUIT); 
      if( pthread_mutex_unlock(&mutexGrille) != 0)
      {
        perror("Erreur lock mutex grille \n");
        exit(1);
      }
      if(pthread_create(&FlotteAliens,NULL,fctFlotteAliens,NULL) != 0)
      {
        perror("Error create thread flotteAliens \n");
        exit(1);
      }
      if(pthread_join(FlotteAliens,NULL) != 0)
      {
        perror("Erreur pthread join main\n");
        exit(1);
      }  
    }
    if(flotte == 2)
    {

      lvl++;
      printf("On relance la flote lvl : %d\n",lvl);
      print = lvl;
      i = 4;
      while(print != 0)
      {
        digit = print % 10;
        print = print / 10;
        DessineChiffre(13,i,digit);
        i--;
      }
      flotte == -1;
      delay = (delay/100)*70;
      printf("On relance la flote plus rapide\n");
      if(pthread_create(&FlotteAliens,NULL,fctFlotteAliens,NULL) != 0)
      {
        perror("Error create thread flotteAliens \n");
        exit(1);
      }
      if(pthread_join(FlotteAliens,NULL) != 0)
      {
        perror("Erreur pthread join main\n");
        exit(1);
      }       
    }
    // on recrée les bouclier
    if( pthread_mutex_lock(&mutexGrille) != 0)
    {
      perror("Erreur lock mutex grille \n");
      exit(1);
    }
    setTab(NB_LIGNE-2,11,BOUCLIER1,0);  DessineBouclier(NB_LIGNE-2,11,1);
    setTab(NB_LIGNE-2,12,BOUCLIER1,0);  DessineBouclier(NB_LIGNE-2,12,1);
    setTab(NB_LIGNE-2,13,BOUCLIER1,0);  DessineBouclier(NB_LIGNE-2,13,1);
    setTab(NB_LIGNE-2,17,BOUCLIER1,0);  DessineBouclier(NB_LIGNE-2,17,1);
    setTab(NB_LIGNE-2,18,BOUCLIER1,0);  DessineBouclier(NB_LIGNE-2,18,1);
    setTab(NB_LIGNE-2,19,BOUCLIER1,0);  DessineBouclier(NB_LIGNE-2,19,1);
    if( pthread_mutex_unlock(&mutexGrille) != 0)
    {
      perror("Erreur lock mutex grille \n");
      exit(1);
    }
  }
  pthread_exit(NULL);
}
void *fctFlotteAliens(void*)
{
  int i,j,k;
  int randAlienLigne,randAlienColonne;
  bool tireBombe = true;
  bool trouveAlien = false;
  printf("Test 14\n");
  if( pthread_mutex_lock(&mutexGrille) != 0)
  {
    perror("Erreur lock mutex grille \n");
    exit(1);
  }
  // Creation des aliens
  for(i = 2;i < 9 ; i=i+2)
  {
    for(j = 8 ; j < 19; j = j+ 2)
    {
      setTab(i, j, ALIEN, pthread_self());
      DessineAlien(i,j);
    }
  }
  if( pthread_mutex_unlock(&mutexGrille) != 0)
  {
    perror("Erreur lock mutex grille \n");
    exit(1);
  }
  if( pthread_mutex_lock(&mutexFlotteAliens) != 0)
  {
    perror("Erreur lock mutex flotte \n");
    exit(1);
  }
  lh = 2;
  cg = 8;
  lb = 8;
  cd = 18;
  nbAliens = 24;
  if( pthread_mutex_unlock(&mutexFlotteAliens) != 0)
  {
    perror("Erreur unlock mutex flotte \n");
    exit(1);
  }


  while(nbAliens > 0 && lb < 15)
  {
    // 4 pas sur la droite
    printf("nb alien : %d\n",nbAliens);
    for(k = cd; k < 22; k++)
    {
      Attente(delay);
      if(nbAliens == 0)
      { 
        printf("fin flotte plus d'aliens\n");
        flotte = 2;
        pthread_exit(NULL);
      }
      pthread_cond_signal(&condFlotteAliens);
      if(ok)
      { 
        pthread_exit(NULL);
      }
      for(i = lh;i < lb + 1 ; i=i+2)
      {
        for(j = cg ; j < cd + 1; j = j+ 2)
        {
          if( pthread_mutex_lock(&mutexGrille) != 0)
          {
            perror("Erreur lock mutex grille \n");
            exit(1);
          }
          if(tab[i][j].type == ALIEN)
          {
            if(tab[i][j+1].type == VIDE)
            {
              setTab(i,j,VIDE,0);
              EffaceCarre(i,j);
              setTab(i, j+1, ALIEN, pthread_self());
              DessineAlien(i,j+1);            
            }
            else
            {
              if(tab[i][j+1].type == MISSILE)
              {
                pthread_kill(tab[i][j+1].tid,SIGINT);
                setTab(i,j+1,VIDE,0);
                EffaceCarre(i,j+1);
                setTab(i,j,VIDE,0);
                EffaceCarre(i,j);
                if( pthread_mutex_lock(&mutexFlotteAliens) != 0)
                {
                  perror("Erreur lock mutex grille \n");
                  exit(1);
                }
                nbAliens--;
                if( pthread_mutex_unlock(&mutexFlotteAliens) != 0)
                {
                  perror("Erreur lock mutex grille \n");
                  exit(1);
                }
                if(pthread_mutex_lock(&mutexScore) != 0)
                {
                  perror("Erreur lock mutex score \n");
                  exit(1);
                }
                score++;
                MAJScore = true;
                if(pthread_mutex_unlock(&mutexScore) != 0)
                {
                  perror("Erreur unlock mutex score \n");
                  exit(1);
                }
                pthread_cond_signal(&condScore);                
              }
              if(tab[i][j+1].type == BOMBE)
              {
                pthread_kill(tab[i][j+1].tid,SIGINT);
                setTab(i,j+1,VIDE,0);
                EffaceCarre(i,j+1); 
                setTab(i,j,VIDE,0);
                EffaceCarre(i,j);
                setTab(i, j+1, ALIEN, pthread_self());
                DessineAlien(i,j+1);
                S_POSITION * bo = (S_POSITION *)malloc(sizeof(S_POSITION));
                bo->L = i+1;
                bo->C = j+1;
                if(pthread_create(&Bombe,NULL,(void*(*)(void*))fctBombe,bo) != 0)
                {
                  perror("Error create thread vaisseau \n");
                  exit(1);
                }              

              }

            }            
          }
          if( pthread_mutex_unlock(&mutexGrille) != 0)
          {
            perror("Erreur lock mutex grille \n");
            exit(1);
          } 
        }
      }
      if( pthread_mutex_lock(&mutexFlotteAliens) != 0)
      {
        perror("Erreur lock mutex grille \n");
        exit(1);
      }  
      cg++;
      cd++;
      if(tireBombe == true)
      {
        if( pthread_mutex_lock(&mutexGrille) != 0)
        {
          perror("Erreur lock mutex grille \n");
          exit(1);
        } 
        randAlienLigne = rand()%lb + lh; // nombre entre lb et lh
        randAlienColonne = rand()%cd + cg;
        while(!trouveAlien)
        {
          if(tab[randAlienLigne][randAlienColonne].type == ALIEN)
          {
            S_POSITION * bo = (S_POSITION *)malloc(sizeof(S_POSITION));
            bo->L = randAlienLigne+1;
            bo->C = randAlienColonne;
            if(pthread_create(&Bombe,NULL,(void*(*)(void*))fctBombe,bo) != 0)
            {
              perror("Error create thread vaisseau \n");
              exit(1);
            }           
            tireBombe = false;
            trouveAlien = true;            
          }
          else
          {
            randAlienLigne = rand()%lb + lh; // nombre entre lb et lh
            randAlienColonne = rand()%cd + cg;            
          }
        }

        if( pthread_mutex_unlock(&mutexGrille) != 0)
        {
          perror("Erreur unlock mutex grille \n");
          exit(1);
        } 
      }
      else
      {
        tireBombe = true;
      }
      trouveAlien = false;
      if( pthread_mutex_lock(&mutexGrille) != 0)
      {
        perror("Erreur lock mutex grille \n");
        exit(1);
      } 
      modifyFlotte();
      if( pthread_mutex_unlock(&mutexGrille) != 0)
      {
        perror("Erreur lock mutex grille \n");
        exit(1);
      } 
      if( pthread_mutex_unlock(&mutexFlotteAliens) != 0)
      {
        perror("Erreur lock mutex grille \n");
        exit(1);
      }  

    }
    // 4 pas sur la gauche
    printf("nb alien : %d\n",nbAliens);
    for(k = cg; k > 8; k--)
    {
      if(nbAliens == 0)
      { 
        printf("fin flotte plus d'aliens\n");
        flotte = 2;
        pthread_exit(NULL);
      }
      pthread_cond_signal(&condFlotteAliens);
      if(ok)
      { 
        pthread_exit(NULL);
      }
      Attente(delay);
      for(i = lh;i < lb + 1 ; i=i+2)
      {
        for(j = cd ; j > cg - 1; j = j- 2)
        {
          if( pthread_mutex_lock(&mutexGrille) != 0)
          {
            perror("Erreur lock mutex grille \n");
            exit(1);
          }
          if(tab[i][j].type == ALIEN)
          {
            if(tab[i][j-1].type == VIDE)
            {
              setTab(i,j,VIDE,0);
              EffaceCarre(i,j);
              setTab(i, j-1, ALIEN, pthread_self());
              DessineAlien(i,j-1);            
            }
            else
            {
              if(tab[i][j-1].type == MISSILE)
              {
                pthread_kill(tab[i][j-1].tid,SIGINT);
                setTab(i,j-1,VIDE,0);
                EffaceCarre(i,j-1);
                setTab(i,j,VIDE,0);
                EffaceCarre(i,j);
                if( pthread_mutex_lock(&mutexFlotteAliens) != 0)
                {
                  perror("Erreur lock mutex grille \n");
                  exit(1);
                }
                nbAliens--;
                if( pthread_mutex_unlock(&mutexFlotteAliens) != 0)
                {
                  perror("Erreur lock mutex grille \n");
                  exit(1);
                }
                if(pthread_mutex_lock(&mutexScore) != 0)
                {
                  perror("Erreur lock mutex score \n");
                  exit(1);
                }
                score++;
                MAJScore = true;
                if(pthread_mutex_unlock(&mutexScore) != 0)
                {
                  perror("Erreur unlock mutex score \n");
                  exit(1);
                }
                pthread_cond_signal(&condScore);                
              }
              if(tab[i][j-1].type == BOMBE)
              {
                pthread_kill(tab[i][j-1].tid,SIGINT);
                setTab(i,j-1,VIDE,0);
                EffaceCarre(i,j-1); 
                setTab(i,j,VIDE,0);
                EffaceCarre(i,j);
                setTab(i, j-1, ALIEN, pthread_self());
                DessineAlien(i,j-1);
                S_POSITION * bo = (S_POSITION *)malloc(sizeof(S_POSITION));
                bo->L = i+1;
                bo->C = j-1;
                if(pthread_create(&Bombe,NULL,(void*(*)(void*))fctBombe,bo) != 0)
                {
                  perror("Error create thread vaisseau \n");
                  exit(1);
                }                 
              }

            }
          }
          if( pthread_mutex_unlock(&mutexGrille) != 0)
          {
            perror("Erreur lock mutex grille \n");
            exit(1);
          } 
        }
      }
      if( pthread_mutex_lock(&mutexFlotteAliens) != 0)
      {
        perror("Erreur lock mutex grille \n");
        exit(1);
      }  
      cg--;
      cd--;
      if(tireBombe == true)
      {
        if( pthread_mutex_lock(&mutexGrille) != 0)
        {
          perror("Erreur lock mutex grille \n");
          exit(1);
        } 
        randAlienLigne = rand()%lb + lh; // nombre entre lb et lh
        randAlienColonne = rand()%cd + cg;
        while(!trouveAlien)
        {
          if(tab[randAlienLigne][randAlienColonne].type == ALIEN)
          {
            S_POSITION * bo = (S_POSITION *)malloc(sizeof(S_POSITION));
            bo->L = randAlienLigne+1;
            bo->C = randAlienColonne;
            if(pthread_create(&Bombe,NULL,(void*(*)(void*))fctBombe,bo) != 0)
            {
              perror("Error create thread vaisseau \n");
              exit(1);
            }           
            tireBombe = false;
            trouveAlien = true;            
          }
          else
          {
            randAlienLigne = rand()%lb + lh; // nombre entre lb et lh
            randAlienColonne = rand()%cd + cg;            
          }
        }

        if( pthread_mutex_unlock(&mutexGrille) != 0)
        {
          perror("Erreur unlock mutex grille \n");
          exit(1);
        } 
      }
      else
      {
        tireBombe = true;
      }
      trouveAlien = false;
      if( pthread_mutex_lock(&mutexGrille) != 0)
      {
        perror("Erreur lock mutex grille \n");
        exit(1);
      } 
      modifyFlotte();
      if( pthread_mutex_unlock(&mutexGrille) != 0)
      {
        perror("Erreur lock mutex grille \n");
        exit(1);
      } 
      if( pthread_mutex_unlock(&mutexFlotteAliens) != 0)
      {
        perror("Erreur lock mutex grille \n");
        exit(1);
      }   
    }
    // on descend de 1
    printf("nb alien : %d\n",nbAliens);
    Attente(delay);
    if(nbAliens == 0)
    { 
      printf("fin flotte plus d'aliens\n");
      flotte = 2;
      pthread_exit(NULL);
    }
    pthread_cond_signal(&condFlotteAliens);
    if(ok)
    { 
      pthread_exit(NULL);
    }
    for(i = lh ;i < lb + 1 ; i=i+2)
    {
      for(j = cg ; j < cd + 1; j = j+ 2)
      {
        if( pthread_mutex_lock(&mutexGrille) != 0)
        {
          perror("Erreur lock mutex grille \n");
          exit(1);
        }
        if(tab[i][j].type == ALIEN)
        {
          if(tab[i + 1][j].type == VIDE)
          {
            setTab(i,j,VIDE,0);
            EffaceCarre(i,j);
            setTab(i + 1, j, ALIEN, pthread_self());
            DessineAlien(i + 1,j);            
          }
          else
          {
            if(tab[i + 1][j].type == MISSILE)
            {
              pthread_kill(tab[i+1][j].tid,SIGINT);
              setTab(i+1,j,VIDE,0);
              EffaceCarre(i+1,j);
              setTab(i,j,VIDE,0);
              EffaceCarre(i,j);
              if( pthread_mutex_lock(&mutexFlotteAliens) != 0)
              {
                perror("Erreur lock mutex grille \n");
                exit(1);
              }
              nbAliens--;
              if( pthread_mutex_unlock(&mutexFlotteAliens) != 0)
              {
                perror("Erreur lock mutex grille \n");
                exit(1);
              }
              if(pthread_mutex_lock(&mutexScore) != 0)
              {
                perror("Erreur lock mutex score \n");
                exit(1);
              }
              score++;
              MAJScore = true;
              if(pthread_mutex_unlock(&mutexScore) != 0)
              {
                perror("Erreur unlock mutex score \n");
                exit(1);
              }
              pthread_cond_signal(&condScore);             
            }
            if(tab[i + 1][j].type == BOMBE)
            {
              pthread_kill(tab[i+1][j].tid,SIGINT);
              setTab(i+1,j,VIDE,0);
              EffaceCarre(i+1,j); 
              setTab(i,j,VIDE,0);
              EffaceCarre(i,j);
              setTab(i+1, j, ALIEN, pthread_self());
              DessineAlien(i+1,j);
              S_POSITION * bo = (S_POSITION *)malloc(sizeof(S_POSITION));
              bo->L = i+2;
              bo->C = j;
              if(pthread_create(&Bombe,NULL,(void*(*)(void*))fctBombe,bo) != 0)
              {
                perror("Error create thread vaisseau \n");
                exit(1);
              }               
            }

          }          
        }
        if( pthread_mutex_unlock(&mutexGrille) != 0)
        {
          perror("Erreur lock mutex grille \n");
          exit(1);
        }
      }
    }

    if( pthread_mutex_lock(&mutexFlotteAliens) != 0)
    {
      perror("Erreur lock mutex Flotte \n");
      exit(1);
    }  
    lh++;
    lb++;
      if(tireBombe == true)
      {
        if( pthread_mutex_lock(&mutexGrille) != 0)
        {
          perror("Erreur lock mutex grille \n");
          exit(1);
        } 
        randAlienLigne = rand()%lb + lh; // nombre entre lb et lh
        randAlienColonne = rand()%cd + cg;
        while(!trouveAlien)
        {
          if(tab[randAlienLigne][randAlienColonne].type == ALIEN)
          {
            S_POSITION * bo = (S_POSITION *)malloc(sizeof(S_POSITION));
            bo->L = randAlienLigne+1;
            bo->C = randAlienColonne;
            if(pthread_create(&Bombe,NULL,(void*(*)(void*))fctBombe,bo) != 0)
            {
              perror("Error create thread vaisseau \n");
              exit(1);
            }           
            tireBombe = false;
            trouveAlien = true;            
          }
          else
          {
            randAlienLigne = rand()%lb + lh; // nombre entre lb et lh
            randAlienColonne = rand()%cd + cg;            
          }
        }

        if( pthread_mutex_unlock(&mutexGrille) != 0)
        {
          perror("Erreur unlock mutex grille \n");
          exit(1);
        } 
      }
      else
      {
        tireBombe = true;
      }
    trouveAlien = false;
    if( pthread_mutex_lock(&mutexGrille) != 0)
    {
      perror("Erreur lock mutex grille \n");
      exit(1);
    } 
    modifyFlotte();
    if( pthread_mutex_unlock(&mutexGrille) != 0)
    {
      perror("Erreur lock mutex grille \n");
      exit(1);
    } 
    if( pthread_mutex_unlock(&mutexFlotteAliens) != 0)
    {
      perror("Erreur unlock mutex Flotte \n");
      exit(1);
    } 
  }
  printf("On sort du thread flotte\n");
  if(nbAliens == 0)
  {
    flotte = 2;
    pthread_exit(NULL);
  }
  else
  {
    flotte = 1;
    pthread_exit(NULL);
  }
}
void *fctEvent(void*)
{
  EVENT_GRILLE_SDL event;
  while(!ok)
  {
    event = ReadEvent();
    if (event.type == CROIX) 
    {
      ok = true;
      pthread_cond_signal(&condVies);
    }
    if (event.type == CLAVIER)
    {
      if (event.touche == KEY_RIGHT){ 
        //printf("Flèche droite enfoncée\n");
        kill(0,SIGUSR1);
      }
      if (event.touche == KEY_LEFT)
      {
        //printf("Flèche droite enfoncée\n");
        kill(0,SIGUSR2);
      } 
      if(event.touche == KEY_SPACE)
      {
        //printf("Espace enfoncée\n");
        kill(0,SIGHUP);
      }
      //printf("Touche enfoncee : %c\n",event.touche);
    }
  }

  // Fermeture de la fenetre
  printf("(ThreadEvent %ld) Fermeture de la fenetre graphique...",pthread_self()); fflush(stdout);
  FermetureFenetreGraphique();
  printf("OK\n");
  pthread_exit(NULL);
}
void *fctVaisseau(void *)
{
  sigset_t mask;

  droite.sa_handler = HandlerSigusr1;
  sigaction(SIGUSR1,&droite,NULL);

  gauche.sa_handler = HandlerSigusr2;
  sigaction(SIGUSR2,&gauche,NULL);

  espace.sa_handler = HandlerSighup;
  sigaction(SIGHUP,&espace,NULL); 

  mortVaisseau.sa_handler = HandlerSigquit;
  sigaction(SIGQUIT,&mortVaisseau,NULL);

  sigemptyset(&mask);
  sigaddset(&mask,SIGINT);
  sigaddset(&mask,SIGALRM);
  sigaddset(&mask,SIGCHLD);
  sigprocmask(SIG_SETMASK,&mask,NULL);

  pthread_cleanup_push(moinsUneVie,NULL);
  colonne = 15;
  if( pthread_mutex_lock(&mutexGrille) != 0)
  {
    perror("Erreur lock mutex grille \n");
    exit(1);
  }
  if(tab[LIGNE_VAISSEAU][colonne].type == VIDE)
  {  
    setTab(LIGNE_VAISSEAU, colonne, VAISSEAU, pthread_self());
    DessineVaisseau(LIGNE_VAISSEAU,colonne);
  }
  else
  {
    perror("Erreur emplacement vaisseau occupé\n");
    exit(1);
  }
  if(pthread_mutex_unlock(&mutexGrille) != 0)
  {
    perror("Erreur unlock mutex grille \n");
    exit(1);
  }
  while(1)
  {
    pause();
  }
  pthread_cleanup_pop(1);
  pthread_exit(NULL);
}
void HandlerSigusr1(int sig)
{
  if( pthread_mutex_lock(&mutexGrille) != 0)
  {
    perror("Erreur lock mutex grille \n");
    exit(1);
  }

  if((colonne + 1) > 22)
  {
    printf("Impossible d'avancer sur la droite\n");
  }
  else
  {
    setTab(LIGNE_VAISSEAU, colonne, VIDE, 0);
    EffaceCarre(LIGNE_VAISSEAU,colonne);

    if(tab[LIGNE_VAISSEAU][colonne+1].type == VIDE)
    {
      colonne++;
      setTab(LIGNE_VAISSEAU, colonne, VAISSEAU, pthread_self());
      DessineVaisseau(LIGNE_VAISSEAU,colonne);
      //printf("Vaisseau deplacé sur la droite\n");     
    }
    else
      printf("Erreur colision vaisseau droite\n");

  }

  if(pthread_mutex_unlock(&mutexGrille) != 0)
  {
    perror("Erreur unlock mutex grille \n");
    exit(1);
  }
}
void HandlerSigusr2(int sig)
{
  if( pthread_mutex_lock(&mutexGrille) != 0)
  {
    perror("Erreur lock mutex grille \n");
    exit(1);
  }

  if((colonne - 1) < 8)
  {
    printf("Impossible d'avancer sur la gauche\n");
  }
  else
  {
    setTab(LIGNE_VAISSEAU, colonne, VIDE, 0);
    EffaceCarre(LIGNE_VAISSEAU,colonne);
    if(tab[LIGNE_VAISSEAU][colonne-1].type == VIDE)
    {
      colonne--;
      setTab(LIGNE_VAISSEAU, colonne, VAISSEAU, pthread_self());
      DessineVaisseau(NB_LIGNE-1,colonne);     
    }
    else
      printf("Erreur colision vaisseau gauche\n");
  }

  if(pthread_mutex_unlock(&mutexGrille) != 0)
  {
    perror("Erreur unlock mutex grille \n");
    exit(1);
  }
}
void HandlerSighup(int sig)
{
  if(fireOn)
  {
    fireOn = false;
    S_POSITION * mi = (S_POSITION *)malloc(sizeof(S_POSITION));
    mi->L = NB_LIGNE -2;
    mi->C = colonne;
    if(pthread_create(&Missile,NULL,(void*(*)(void*))fctMissile,mi) != 0)
    {
      perror("Error create thread vaisseau \n");
      exit(1);
    }  
    if(pthread_create(&TimeOutMissile,NULL,fctTimeOut,NULL) != 0)
    {
      perror("Error create thread vaisseau \n");
      exit(1);
    }  
  }
}
void HandlerSigquit(int sig)
{
  printf("Siquit recu\n");

  if( pthread_mutex_lock(&mutexColonne) != 0)
  {
    perror("Erreur lock mutex grille \n");
    exit(1);
  }  
  setTab(LIGNE_VAISSEAU,colonne,VIDE,0);
  EffaceCarre(LIGNE_VAISSEAU,colonne);
  if( pthread_mutex_unlock(&mutexColonne) != 0)
  {
    perror("Erreur lock mutex grille \n");
    exit(1);
  }
  pthread_cancel(Vaiss);
}
void *fctMissile(S_POSITION* mi)
{
  sigset_t mask;
  missile.sa_handler = HandlerSigint;
  sigaction(SIGINT,&missile,NULL);

  sigemptyset(&mask);
  sigaddset(&mask,SIGUSR1);
  sigaddset(&mask,SIGALRM);
  sigaddset(&mask,SIGCHLD);
  sigaddset(&mask,SIGUSR2);
  sigaddset(&mask,SIGHUP);
  sigaddset(&mask,SIGQUIT);
  sigprocmask(SIG_SETMASK,&mask,NULL);

  if( pthread_mutex_lock(&mutexGrille) != 0)
  {
    perror("Erreur lock mutex grille \n");
    exit(1);
  }

  switch(tab[mi->L][mi->C].type)
  {
    case 0: {
      // case vide
      setTab(mi->L, mi->C, MISSILE, pthread_self());
      DessineMissile(mi->L, mi->C);
      if(pthread_mutex_unlock(&mutexGrille) != 0)
      {
        perror("Erreur unlock mutex grille \n");
        exit(1);
      }
    }
    break;
    case 3:{
      setTab(mi->L,mi->C,VIDE,0);
      EffaceCarre(mi->L,mi->C);
      if(pthread_mutex_lock(&mutexFlotteAliens) != 0)
      {
        perror("Erreur lock mutex flotte \n");
        exit(1);
      }
      nbAliens--;
      if(pthread_mutex_unlock(&mutexFlotteAliens) != 0)
      {
        perror("Erreur unlock mutex flotte \n");
        exit(1);
      }
      if(pthread_mutex_unlock(&mutexGrille) != 0)
      {
        perror("Erreur unlock mutex grille \n");
        exit(1);
      }
      if(pthread_mutex_lock(&mutexScore) != 0)
      {
        perror("Erreur lock mutex score \n");
        exit(1);
      }
      score++;
      MAJScore = true;
      if(pthread_mutex_unlock(&mutexScore) != 0)
      {
        perror("Erreur unlock mutex score \n");
        exit(1);
      }
      pthread_cond_signal(&condScore);
      free(mi);
      pthread_exit(NULL); 
    }
    break;
    case 4:{
      pthread_kill(tab[mi->L][mi->C].tid,SIGINT);
      setTab(mi->L,mi->C,VIDE,0);
      EffaceCarre(mi->L,mi->C);
      if(pthread_mutex_unlock(&mutexGrille) != 0)
      {
        perror("Erreur unlock mutex grille \n");
        exit(1);
      }
      free(mi);
      pthread_exit(NULL);      
    }
    break;
    case 5:{
      // bouclier 1
      setTab(mi->L,mi->C,VIDE,0);
      EffaceCarre(mi->L,mi->C);
      setTab(mi->L, mi->C , BOUCLIER2 , 0);
      DessineBouclier(mi->L, mi->C,2);
      if(pthread_mutex_unlock(&mutexGrille) != 0)
      {
        perror("Erreur unlock mutex grille \n");
        exit(1);
      }
      free(mi);
      pthread_exit(NULL);         
    }
    break;
    case 6:
    {
      //bouclier 2
      setTab(mi->L,mi->C,VIDE,0);
      EffaceCarre(mi->L,mi->C);
      if(pthread_mutex_unlock(&mutexGrille) != 0)
      {
        perror("Erreur unlock mutex grille \n");
        exit(1);
      }
      free(mi);
      pthread_exit(NULL); 
    }
  }

  Attente(80);
  
  while(1)
  {
    if( pthread_mutex_lock(&mutexGrille) != 0)
    {
      perror("Erreur lock mutex grille \n");
      exit(1);
    }
    if(mi->L > 0)
    {
      switch(tab[mi->L -1][mi->C].type)
      {
        case 0: {
          // case vide
          setTab(mi->L,mi->C,VIDE,0);
          EffaceCarre(mi->L,mi->C);
          setTab(mi->L -1 , mi->C, MISSILE, pthread_self());
          DessineMissile(mi->L -1, mi->C);
          mi->L--;
          if(pthread_mutex_unlock(&mutexGrille) != 0)
          {
            perror("Erreur unlock mutex grille \n");
            exit(1);
          }
        }
        break;
        case 3:{
          setTab(mi->L,mi->C,VIDE,0);
          EffaceCarre(mi->L,mi->C);
          setTab(mi->L -1, mi->C , VIDE , 0);
          EffaceCarre(mi->L -1, mi->C);
          if(pthread_mutex_lock(&mutexFlotteAliens) != 0)
          {
            perror("Erreur unlock mutex grille \n");
            exit(1);
          }
          nbAliens--;
          if(pthread_mutex_unlock(&mutexFlotteAliens) != 0)
          {
            perror("Erreur unlock mutex grille \n");
            exit(1);
          }
          if(pthread_mutex_unlock(&mutexGrille) != 0)
          {
            perror("Erreur unlock mutex grille \n");
            exit(1);
          }
          if(pthread_mutex_lock(&mutexScore) != 0)
          {
            perror("Erreur lock mutex score \n");
            exit(1);
          }
          score++;
          MAJScore = true;
          if(pthread_mutex_unlock(&mutexScore) != 0)
          {
            perror("Erreur unlock mutex score \n");
            exit(1);
          }
          pthread_cond_signal(&condScore);
          free(mi);
          pthread_exit(NULL);
        }
        break;
        case 4:{
          pthread_kill(tab[mi->L-1][mi->C].tid,SIGINT);
          setTab(mi->L-1,mi->C,VIDE,0);
          EffaceCarre(mi->L-1,mi->C);
          setTab(mi->L,mi->C,VIDE,0);
          EffaceCarre(mi->L,mi->C);
          if(pthread_mutex_unlock(&mutexGrille) != 0)
          {
            perror("Erreur unlock mutex grille \n");
            exit(1);
          }
          free(mi);
          pthread_exit(NULL);      
        }
        break;
        case 5:{
          // bouclier 1
          setTab(mi->L,mi->C,VIDE,0);
          EffaceCarre(mi->L,mi->C);
          setTab(mi->L -1, mi->C , BOUCLIER2 , 0);
          EffaceCarre(mi->L -1, mi->C);
          DessineBouclier(mi->L -1, mi->C,2);
          if(pthread_mutex_unlock(&mutexGrille) != 0)
          {
            perror("Erreur unlock mutex grille \n");
            exit(1);
          }
          free(mi);
          pthread_exit(NULL);          
        }
        break;
        case 6:
        {
          //bouclier 2
          setTab(mi->L,mi->C,VIDE,0);
          EffaceCarre(mi->L,mi->C);
          setTab(mi->L -1, mi->C , VIDE , 0);
          EffaceCarre(mi->L -1, mi->C);
          if(pthread_mutex_unlock(&mutexGrille) != 0)
          {
            perror("Erreur unlock mutex grille \n");
            exit(1);
          }
          free(mi);
          pthread_exit(NULL);   
        }
        case 7:{
          setTab(mi->L,mi->C,VIDE,0);
          EffaceCarre(mi->L,mi->C);
          pthread_kill(tab[mi->L -1][mi->C].tid,SIGCHLD);
          if(pthread_mutex_lock(&mutexScore) != 0)
          {
            perror("Erreur lock mutex score \n");
            exit(1);
          }
          score = score + 10;
          MAJScore = true;
          if(pthread_mutex_unlock(&mutexScore) != 0)
          {
            perror("Erreur unlock mutex score \n");
            exit(1);
          }
          if(pthread_mutex_unlock(&mutexGrille) != 0)
          {
            perror("Erreur unlock mutex grille \n");
            exit(1);
          }
        }
        break;
      }
    }
    else
    {
      // detruit missile fin de tab
      setTab(mi->L,mi->C,VIDE,0);
      EffaceCarre(mi->L,mi->C);      
      if(pthread_mutex_unlock(&mutexGrille) != 0)
      {
        perror("Erreur unlock mutex grille \n");
        exit(1);
      }
      free(mi);
      pthread_exit(NULL);  
    }
  Attente(80);
  }
  pthread_exit(NULL);
}
void HandlerSigint(int sig)
{
  pthread_exit(NULL);
}

void *fctTimeOut(void*){
  Attente(600);
  fireOn = true;
  pthread_exit(NULL);
}
void modifyFlotte()
{
  // appeler cette fonction en ayant pris le mutexFlotteAlien !!!
  int i = cg;
  int cpt = 0;
  while(cpt == 0 && i < cd + 1)
  {
    if(tab[lh][i].type == ALIEN)
      cpt++;
    i++;    
  }
  if(cpt == 0)
    lh  = lh + 2;
  cpt = 0;
  i = cg;
  while(cpt == 0 && i < cd + 1)
  {
    if(tab[lb][i].type == ALIEN)
      cpt++; 
    i++;   
  }
  if(cpt == 0)
    lb  = lb - 2;
  cpt = 0;
  i = lh;
  while(cpt == 0 && i < lb +1)
  {
    if(tab[i][cd].type == ALIEN)
      cpt++;
    i++;
  }
  if(cpt == 0)
    cd  = cd - 2;
  cpt = 0;
  i = lh;
  while(cpt == 0 && i < lb +1)
  {
    if(tab[i][cg].type == ALIEN)
      cpt++;
    i++;
  }
  if(cpt == 0)
    cg  = cg + 2;
}
void afficheFlotte()
{
  // fonction test
  printf("Sup gauche : %d\n",cg);
  printf("Sup droit : %d\n",cd);
  printf("ligne haut : %d\n",lh);
  printf("ligne bas : %d\n",lb);
}
void supprimeAlien(){
  // fonction a lancer avec mutexGrille pris
  int i,j;
  if(pthread_mutex_lock(&mutexFlotteAliens) != 0)
  {
    perror("Erreur unlock mutex grille \n");
    exit(1);
  }
  for(i = lh;i < lb + 1; i=i+2)
  {
    for(j = cg ; j < cd + 1 ; j = j+ 2)
    {
      if(tab[i][j].type == ALIEN)
      {
        setTab(i,j,VIDE,0);
        EffaceCarre(i,j);
      }
    }
  }  
  if(pthread_mutex_unlock(&mutexFlotteAliens) != 0)
  {
    perror("Erreur unlock mutex grille \n");
    exit(1);
  }   
}
