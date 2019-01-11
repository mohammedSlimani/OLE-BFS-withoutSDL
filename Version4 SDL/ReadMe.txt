You need to have these libraries installed so you can import these stuff:
	SDL.h
	SDL_image.h

et pour compiler le fichier VersionSDL.c dans linux on utilise la commande :
	gcc VersionSDL.c -lSDL -lSDL_image -o run 
puis:
	./run

REMARQUE: 
	Le program ne peut pas quité que lorsque il a fini son travail et ettendre tout les feux.

	Vous pouvez changer la MAP comme vous voullez et notre program va continuer à trouver le chemin compatible et boujer pour ettiendre le feu et puis revenir au station.
