#ifndef RESSOURCES_H
#define RESSOURCES_H

int  OuvertureFenetreGraphique();
int  FermetureFenetreGraphique();

void DessineVaisseau(int l,int c);
void DessineMissile(int l,int c);
void DessineBombe(int l,int c);
void DessineBouclier(int l,int c,int type);
void DessineAlien(int l,int c);
void DessineExplosion(int l,int c);
void DessineVaisseauAmiral(int l,int c);
void DessineGameOver(int l,int c);
void DessineChiffre(int l,int c,int val);


#endif
