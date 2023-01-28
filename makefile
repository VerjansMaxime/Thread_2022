.SILENT:

GRILLESDL=GrilleSDL
RESSOURCES=Ressources

CC = g++ -DSUN -I$(GRILLESDL) -I$(RESSOURCES)
OBJS = $(GRILLESDL)/GrilleSDL.o $(RESSOURCES)/Ressources.o
PROGRAMS = SpaceInvaders

ALL: $(PROGRAMS)

SpaceInvaders:	SpaceInvaders.cpp $(OBJS)
	echo Creation de SpaceInvaders...
	$(CC) SpaceInvaders.cpp -o SpaceInvaders $(OBJS) -lrt -lpthread -lSDL

$(GRILLESDL)/GrilleSDL.o:	$(GRILLESDL)/GrilleSDL.cpp $(GRILLESDL)/GrilleSDL.h
		echo Creation de GrilleSDL.o ...
		$(CC) -c $(GRILLESDL)/GrilleSDL.cpp
		mv GrilleSDL.o $(GRILLESDL)

$(RESSOURCES)/Ressources.o:	$(RESSOURCES)/Ressources.cpp $(RESSOURCES)/Ressources.h
		echo Creation de Ressources.o ...
		$(CC) -c $(RESSOURCES)/Ressources.cpp
		mv Ressources.o $(RESSOURCES)

clean:
	@rm -f $(OBJS) core

clobber:	clean
	@rm -f tags $(PROGRAMS)
