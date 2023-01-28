#include "Ressources.h"
#include "GrilleSDL.h"

// Macros pour les sprites
#define S_V_AISSEAU                 500001
#define S_M_ISSILE                  500002
#define S_B_OMBE                    500003
#define S_B_OUCLIER1                500004
#define S_B_OUCLIER2                500005
#define S_A_LIEN                    500006
#define S_E_XPLOSION                500007
#define S_V_AISSEAU_AMIRAL          500008
#define S_G_AME_OVER                500009

#define S_CHIFFRE_0                300030
#define S_CHIFFRE_1                300031
#define S_CHIFFRE_2                300032
#define S_CHIFFRE_3                300033
#define S_CHIFFRE_4                300034
#define S_CHIFFRE_5                300035
#define S_CHIFFRE_6                300036
#define S_CHIFFRE_7                300037
#define S_CHIFFRE_8                300038
#define S_CHIFFRE_9                300039

void ChargementImages()
{
  // Definition de l'image de fond
  DessineImageFond("./images/fond5_920x720.bmp");

  AjouteSpriteAFondTransparent(S_V_AISSEAU,"./images/Vaisseau2_40.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_M_ISSILE,"./images/Missile_40.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_B_OMBE,"./images/Bombe_40.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_B_OUCLIER1,"./images/Bouclier1_40.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_B_OUCLIER2,"./images/Bouclier2_40.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_A_LIEN,"./images/Alien2_40.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_E_XPLOSION,"./images/Explosion_40.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_V_AISSEAU_AMIRAL,"./images/VaisseauAmiral_40.bmp",255,255,255);
  AjouteSprite(S_G_AME_OVER,"./images/Gameover_40.bmp");

  // Sprites Chiffres
  AjouteSpriteAFondTransparent(S_CHIFFRE_0,"./images/chiffres/Zero.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_CHIFFRE_1,"./images/chiffres/Un.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_CHIFFRE_2,"./images/chiffres/Deux.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_CHIFFRE_3,"./images/chiffres/Trois.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_CHIFFRE_4,"./images/chiffres/Quatre.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_CHIFFRE_5,"./images/chiffres/Cinq.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_CHIFFRE_6,"./images/chiffres/Six.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_CHIFFRE_7,"./images/chiffres/Sept.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_CHIFFRE_8,"./images/chiffres/Huit.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_CHIFFRE_9,"./images/chiffres/Neuf.bmp",255,255,255);  
}

// Fonctions Ressources
int OuvertureFenetreGraphique()
{
  if (OuvrirGrilleSDL(18,23,40,"!!! SPACE INVADERS !!!","./images/fond5_920x720.bmp") < 0)
    return -1;
 
  ChargementImages();
  return 0;
}

int FermetureFenetreGraphique()
{
  FermerGrilleSDL();
  return 0;
}

void DessineVaisseau(int l,int c)
{
  DessineSprite(l,c,S_V_AISSEAU);
}

void DessineBouclier(int l,int c,int type)
{
  switch(type)
  {
    case 1 : DessineSprite(l,c,S_B_OUCLIER1); break;
    case 2 : DessineSprite(l,c,S_B_OUCLIER2); break;
    default: DessineSprite(l,c,S_B_OUCLIER1); break;    
  }
}
void DessineMissile(int l,int c)
{
  DessineSprite(l,c,S_M_ISSILE);
}

void DessineBombe(int l,int c)
{
  DessineSprite(l,c,S_B_OMBE);
}

void DessineAlien(int l,int c)
{
  DessineSprite(l,c,S_A_LIEN);
}

void DessineExplosion(int l,int c)
{
  DessineSprite(l,c,S_E_XPLOSION);
}

void DessineVaisseauAmiral(int l,int c)
{
  DessineSprite(l,c,S_V_AISSEAU_AMIRAL);
}

void DessineGameOver(int l,int c)
{
  DessineSprite(l,c,S_G_AME_OVER);
}

void DessineChiffre(int l,int c,int val)
{
  switch(val)
  {
    case 0 : DessineSprite(l,c,S_CHIFFRE_0); break;
    case 1 : DessineSprite(l,c,S_CHIFFRE_1); break;
    case 2 : DessineSprite(l,c,S_CHIFFRE_2); break;
    case 3 : DessineSprite(l,c,S_CHIFFRE_3); break;
    case 4 : DessineSprite(l,c,S_CHIFFRE_4); break;
    case 5 : DessineSprite(l,c,S_CHIFFRE_5); break;
    case 6 : DessineSprite(l,c,S_CHIFFRE_6); break;
    case 7 : DessineSprite(l,c,S_CHIFFRE_7); break;
    case 8 : DessineSprite(l,c,S_CHIFFRE_8); break;
    case 9 : DessineSprite(l,c,S_CHIFFRE_9); break;
    default : DessineSprite(l,c,S_CHIFFRE_0); break;    
  }
}

