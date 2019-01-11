#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <unistd.h>
#include <SDL/SDL_image.h>
#include "functions.h"


int main(int argc, char const *argv[])
{	
	
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Surface *screen = NULL;
	SDL_Event event;
	Load_MAP();
	screen = SDL_SetVideoMode(700,700,32, SDL_HWSURFACE | SDL_DOUBLEBUF);
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 1024);
	Mix_AllocateChannels(32);
	Mix_Volume(1, MIX_MAX_VOLUME/2);
	Mix_Chunk *fire_down = Mix_LoadWAV("fire_down.wav");
	Mix_Chunk *fill_water = Mix_LoadWAV("water.wav");
	Mix_VolumeChunk(fire_down, MIX_MAX_VOLUME); 
  	Mix_VolumeChunk(fill_water, MIX_MAX_VOLUME);

	point pFire = drone();
	point pStation = {0,0};
	point pRobot = {0,1};
	queueNode T[max_point][max_point];
	int distance = 0;
	int OK = 1;
	
	while(OK){
		
		SDL_WaitEvent(&event);
		switch(event.type){
			case SDL_QUIT:
				OK = 0;
				break;
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym){
					case SDLK_ESCAPE:
						OK =0;
						break;
				}
				break;
		}

		while(pFire.x != -1){  //On a trouvé le feu

			if (MAP[pRobot.x][pRobot.y].special - MAP[pFire.x][pFire.y].special >= 0 )
			//Dans ce can on peut eteindre le feu
			{	printf("Le reservoire du robot est : %d\n",MAP[pRobot.x][pRobot.y].special);
				printf("Le danger du feu est : %d\n",MAP[pFire.x][pFire.y].special );
				printf("There is Enough water!\n");
				printf("Going to the fire position ...\n\n");
				BFS(MAP,T,pRobot,pFire);
				distance = T[pFire.x][pFire.y].dist;
				point* path = GetPath(T,pFire);
				move_robot(distance,path,screen,pFire);
				pRobot.x = path[distance -1].x;
				pRobot.y = path[distance -1].y;
				MAP[pRobot.x][pRobot.y].special -= MAP[pFire.x][pFire.y].special;
				printf("Putting down fire ...... Please Wait\n");
				Mix_PlayChannel(1, fire_down, 0);
				sleep(3);
				make_road(pFire);
				pFire = drone();
			}else{
			//On ne peut peas eteindre le feu alors il faut revenir au station
				printf("Le reservoire du robot est : %d\n",MAP[pRobot.x][pRobot.y].special);
				printf("Le danger du feu est : %d\n",MAP[pFire.x][pFire.y].special );
				printf("Not enough Water!!\n");
				printf("Returning to Base ....\n\n");

				BFS(MAP,T,pRobot,pStation);
				distance = T[pStation.x][pStation.y].dist;
				point* path1 = GetPath(T,pStation);
				move_robot(distance,path1,screen,pFire);
				pRobot.x = path1[distance -1].x;//0
				pRobot.y = path1[distance -1].y;//1
				Mix_PlayChannel(1, fill_water, 0);
				printf("Refilling the Tank..... Please Wait\n");
				sleep(4);
				MAP[pRobot.x][pRobot.y].special = 3; //Refill the tank
			}

		}
	}SDL_Quit();
	return EXIT_SUCCESS;
}


/*-----------------------------------------------------------------------------------------*/
void LoadImagesToThe(SDL_Surface *screen,point pFire){
	int i;
	SDL_Rect posFire = {pFire.y*35,pFire.x*35};
	SDL_Surface *obstacle_image = IMG_Load("obstacle.bmp");
	SDL_Surface *fire_image = IMG_Load("FIRE.bmp");
	SDL_Surface *robot_image = IMG_Load("ROBOT_FIGHT_FIRE.bmp");
	SDL_Surface *staion_image = IMG_Load("Station.bmp");
	SDL_Surface *num0 = IMG_Load("n0.bmp");
	SDL_Surface *num1 = IMG_Load("n1.bmp");
	SDL_Surface *num2 = IMG_Load("n2.bmp");
	SDL_Surface *num3 = IMG_Load("n3.bmp");
	SDL_Surface *box = IMG_Load("box.bmp");
	SDL_BlitSurface(box,NULL,screen,&posFire);
	SDL_Rect position = {0,0};
	for (i = 0; i < max_point; ++i)
	{
		for (int j = 0; j < max_point; ++j)
		{	
			//le choix de 35 c'est car la lagrgueur et le hauteur des images est 35 pixels
			position.x = 35*j;
			position.y = 35*i;
			if (i == 0 && j ==0) 								SDL_BlitSurface(staion_image,NULL,screen,&position);
			else if (MAP[i][j].something == obstacle)			SDL_BlitSurface(obstacle_image,NULL,screen,&position);
			else if(MAP[i][j].something == fire){
				SDL_BlitSurface(fire_image,NULL,screen,&position);
				switch(MAP[i][j].special){
					case 0:
						SDL_BlitSurface(num0,NULL,screen,&position);
						break;
					case 1:
						SDL_BlitSurface(num1,NULL,screen,&position);
						break;
					case 2:
						SDL_BlitSurface(num2,NULL,screen,&position);
						break;
					case 3:
						SDL_BlitSurface(num3,NULL,screen,&position);
						break;

				}
			}				
			else if(MAP[i][j].something == robot){
				SDL_BlitSurface(robot_image,NULL,screen,&position);
				switch(MAP[i][j].special){
					case 0:
						SDL_BlitSurface(num0,NULL,screen,&position);
						break;
					case 1:
						SDL_BlitSurface(num1,NULL,screen,&position);
						break;
					case 2:
						SDL_BlitSurface(num2,NULL,screen,&position);
						break;
					case 3:
						SDL_BlitSurface(num3,NULL,screen,&position);
						break;

				}
			}				
		}	
	}
}



/*-----------------------------------------------------------------------------------------*/
bool isValid(int row, int col)
{
    // return true if row number and column number
    // is in range
    return (row >= 0) && (row < max_point) && (col >= 0) && (col < max_point);
}


/*-----------------------------------------------------------------------------------------*/
File *initialiser()
{
    File *file = malloc(sizeof(*file));
    file->premier = NULL;
    return file;
}


/*-----------------------------------------------------------------------------------------*/
void enfiler(File *file, queueNode s)
{
    Element *nouveau = malloc(sizeof(*nouveau));
    if (file == NULL || nouveau == NULL)
    {
        exit(EXIT_FAILURE);
    }

    nouveau->n = s;
    nouveau->suivant = NULL;

    if (file->premier != NULL) /* La file n'est pas vide */
    {
        /* On se positionne à la fin de la file */
        Element *elementActuel = file->premier;
        while (elementActuel->suivant != NULL)
        {
            elementActuel = elementActuel->suivant;
        }
        elementActuel->suivant = nouveau;
    }
    else /* La file est vide, notre élément est le premier */
    {
        file->premier = nouveau;
    }
}


/*-----------------------------------------------------------------------------------------*/
bool empty(File *F)
{
	if(F==NULL)
	  return true;
	else 
	  return false;
}


/*-----------------------------------------------------------------------------------------*/
queueNode front(File *F)
{
	queueNode r=F->premier->n;
    return r;	
}


/*-----------------------------------------------------------------------------------------*/
void defiler(File *file)
{
    if (file == NULL)
    {
        exit(EXIT_FAILURE);
    }

    /* On vérifie s'il y a quelque chose à défiler */
    if (file->premier != NULL)
    {
        Element *elementDefile = file->premier;
        file->premier = elementDefile->suivant;
        free(elementDefile);
    }
}
 

/*-----------------------------------------------------------------------------------------*/
point* GetPath(queueNode T[max_point][max_point], point dest)
{
	int i=0,N =0;
	N = T[dest.x][dest.y].dist+1;
	point *P=(point*)malloc(N*sizeof(point));
	point pot = dest;
	
	for( i=0; i<N; i++){
		P[N-1-i] = pot;
		pot = T[pot.x][pot.y].pt;
	}
	return P;
}


/*-----------------------------------------------------------------------------------------*/
void BFS(type_coord MAP[max_point][max_point],queueNode T[max_point][max_point], point src, point dest)
{
    int i,j,row,col;
    int rowNum[] = {-1, 0, 0, 1};
    int colNum[] = {0, -1, 1, 0};
     // Create a queue for BFS
    File *q = initialiser();
    bool visited[max_point][max_point];
    // distance of source cell is 0
    queueNode s = {src, 0};
    for (i = 0; i < max_point; i++)
	{
		for (j = 0; j < max_point; j++)
		{
			visited[i][j] = false;
		}
	}
     
    // on marque le source comme visité
    visited[src.x][src.y] = true;
    enfiler(q, s);  // Enqueue la source 
    
    // Do a BFS starting from source 
    while (!empty(q))
    {
        queueNode curr = front(q);
        point pt = curr.pt;
 
        // si on est arrivé à notre distination
        // alors on a terminé
        if (pt.x == dest.x && pt.y == dest.y)
        
            return ;
 
        // Otherwise dequeue the front cell in the queue
        // and enqueue its adjacent cells
        defiler(q);
 
        for (i = 0; i < 4; i++)
        {
            row = pt.x + rowNum[i];
            col = pt.y + colNum[i];
			             
            // if adjacent cell is valid, has path and
            // not visited yet, enqueue it.
            if (isValid(row, col) && (MAP[row][col].something != obstacle ) && !visited[row][col])
            {
            	T[row][col].pt = pt;
            	T[row][col].dist = curr.dist + 1;
                // mark cell as visited and enqueue it
                visited[row][col] = true;
                queueNode Adjcell = { {row, col}, curr.dist + 1 };
                enfiler(q,Adjcell);
            }
        }
    }
}

/*Fonction de management des mouvements des robots */

/*-----------------------------------------------------------------------------------------*/
void make_road(point A)
{
	MAP[A.x][A.y].something = road;
	MAP[A.x][A.y].special = -1;
}


/*-----------------------------------------------------------------------------------------*/
void move_robot(int distance,point* path,SDL_Surface* screen,point pFire)
{
	int i;
	for (i = 0; i < distance -1 ; ++i)
	{
		MAP[path[i+1].x][path[i+1].y].something =robot;
		MAP[path[i+1].x][path[i+1].y].special = MAP[path[i].x][path[i].y].special;
		make_road(path[i]);
		SDL_FillRect(screen,NULL,SDL_MapRGB(screen->format,0,0,0));
		LoadImagesToThe(screen,pFire);
		SDL_Flip(screen);
		sleep(1);
	}
}


/*-----------------------------------------------------------------------------------------*/
/*void ReturnToBase(int distance, point *path,SDL_Surface *screen){
	int i;
	point reversedpath[distance];
	for (i = 0; i < distance; ++i)
	{
		reversedpath[i] = path[distance - i -1];
	}
	move_robot(distance,reversedpath,screen,pFire);
}*/


/*-----------------------------------------------------------------------------------------*/
void afficher()
{
	int i,j;
	for (i = 0; i < max_point; ++i)
	{
		for (j = 0; j < max_point; ++j)
		{
			printf("%d ",MAP[i][j].something);
		} 
		printf("\n");
	}
}


/*-----------------------------------------------------------------------------------------*/
point drone(){
	int i ,j ;
	point firelocation = {-1,-1} ;
	for(i=0; i<max_point;i++){
		for (j = 0; j < max_point; j++)
		{
			if (MAP[i][j].something == fire){
				firelocation.x = i;
				firelocation.y = j;
				printf("Fire foud in location %d   %d \n",i,j );
				sleep(0.5);
				return firelocation;
			}
		}	
	} printf("No fire Found.\n");
	firelocation.x= -1;
	firelocation.y= -1;
	return firelocation;
}

/*-------------------------------------------------------------------------------------------*/

void Load_MAP()
{
  int i,j; 
	//Nous avons fait cet map d'une maniere manuelle 
	//#hard_coded
	for (i = 0; i < max_point; ++i)
	{
		for (j = 0; j < max_point; ++j)
		{
			MAP[i][j].something = road;
			MAP[i][j].special = -1;
		}
	}
	MAP[1][0].something 		= obstacle;
	MAP[1][1].something 		= obstacle;
	MAP[2][0].something 		= obstacle;
	MAP[3][0].something 		= obstacle;
	MAP[3][1].something 		= obstacle;
	MAP[4][0].something 		= obstacle;
	MAP[5][0].something 		= obstacle;
	MAP[5][1].something 		= obstacle;
	MAP[1][3].something 		= obstacle;
	MAP[2][3].something 		= obstacle;
	MAP[3][3].something 		= obstacle;
	MAP[4][3].something 		= obstacle;
	MAP[5][3].something 		= obstacle;
	MAP[2][4].something 		= obstacle;
	MAP[3][5].something 		= obstacle;
	MAP[1][6].something 		= obstacle;
	MAP[2][6].something 		= obstacle;
	MAP[3][6].something 		= obstacle;
	MAP[4][6].something 		= obstacle;
	MAP[5][6].something 		= obstacle;
	MAP[1][8].something 		= obstacle;
	MAP[1][9].something 		= obstacle;
	MAP[2][8].something 		= obstacle;
	MAP[3][8].something 		= obstacle;
	MAP[3][9].something 		= obstacle;
	MAP[4][9].something 		= obstacle;
	MAP[5][9].something 		= obstacle;
	MAP[5][8].something 		= obstacle;
	MAP[1][11].something 		= obstacle;
	MAP[2][11].something 		= obstacle;
	MAP[3][11].something 		= obstacle;
	MAP[4][11].something 		= obstacle;
	MAP[5][11].something 		= obstacle;
	MAP[1][13].something 		= obstacle;
	MAP[2][13].something 		= obstacle;
	MAP[3][13].something 		= obstacle;
	MAP[4][13].something 		= obstacle;
	MAP[5][13].something 		= obstacle;
	MAP[1][15].something 		= obstacle;
	MAP[2][15].something 		= obstacle;
	MAP[3][15].something 		= obstacle;
	MAP[4][15].something 		= obstacle;
	MAP[5][15].something 		= obstacle;
	MAP[5][17].something 		= obstacle;
	MAP[1][14].something 		= obstacle;
	MAP[3][14].something 		= obstacle;
	MAP[1][17].something 		= obstacle;
	MAP[2][17].something 		= obstacle;
	MAP[3][17].something 		= obstacle;
	MAP[1][18].something 		= obstacle;
	MAP[3][18].something 		= obstacle;
	MAP[4][18].something 		= obstacle;
	MAP[5][18].something 		= obstacle;
	MAP[8][0].something 		= obstacle;
	MAP[9][0].something 		= obstacle;
	MAP[10][0].something 		= obstacle;
	MAP[11][0].something 		= obstacle;
	MAP[12][0].something 		= obstacle;
	MAP[8][1].something 		= obstacle;
	MAP[9][2].something 		= obstacle;
	MAP[10][1].something 		= obstacle;
	MAP[11][2].something 		= obstacle;
	MAP[12][1].something 		= obstacle;
	MAP[8][4].something 		= obstacle;
	MAP[9][5].something 		= obstacle;
	MAP[10][6].something 		= obstacle;
	MAP[11][6].something 		= obstacle;
	MAP[12][6].something 		= obstacle;
	MAP[9][7].something 		= obstacle;
	MAP[8][8].something 		= obstacle;
	MAP[8][10].something 		= obstacle;
	MAP[9][10].something 		= obstacle;
	MAP[10][10].something 		= obstacle;
	MAP[11][10].something 		= obstacle;
	MAP[12][10].something 		= obstacle;
	MAP[8][11].something 		= obstacle;
	MAP[8][12].something 		= obstacle;
	MAP[10][11].something 		= obstacle;
	MAP[10][12].something 		= obstacle;
	MAP[12][11].something 		= obstacle;
	MAP[12][12].something 		= obstacle;
	MAP[15][0].something 		= obstacle;
	MAP[16][0].something 		= obstacle;
	MAP[17][0].something 		= obstacle;
	MAP[18][0].something 		= obstacle;
	MAP[19][0].something 		= obstacle;
	MAP[15][1].something 		= obstacle;
	MAP[15][2].something 		= obstacle;
	MAP[17][1].something 		= obstacle;	
	MAP[17][2].something 		= obstacle;
	MAP[15][5].something 		= obstacle;
	MAP[16][5].something 		= obstacle;
	MAP[17][5].something 		= obstacle;
	MAP[18][5].something 		= obstacle;
	MAP[19][5].something 		= obstacle;
	MAP[15][4].something 		= obstacle;
	MAP[15][6].something 		= obstacle;
	MAP[19][4].something 		= obstacle;
	MAP[19][6].something 		= obstacle;
	MAP[15][8].something 		= obstacle;
	MAP[16][8].something 		= obstacle;
	MAP[17][8].something 		= obstacle;
	MAP[18][8].something 		= obstacle;
	MAP[19][8].something 		= obstacle;
	MAP[15][9].something 		= obstacle;
	MAP[15][10].something 		= obstacle;
	MAP[16][10].something 		= obstacle;
	MAP[17][9].something 		= obstacle;
	MAP[18][10].something 		= obstacle;
	MAP[19][10].something 		= obstacle;
	MAP[15][12].something 		= obstacle;
	MAP[16][12].something 		= obstacle;
	MAP[17][12].something 		= obstacle;
	MAP[18][12].something 		= obstacle;
	MAP[19][12].something 		= obstacle;
	MAP[15][13].something 		= obstacle;
	MAP[15][14].something 		= obstacle;
	MAP[17][13].something 		= obstacle;
	MAP[17][14].something 		= obstacle;
	MAP[19][13].something 		= obstacle;
	MAP[19][14].something 		= obstacle;
	MAP[15][16].something 		= obstacle;
	MAP[16][16].something 		= obstacle;
	MAP[17][16].something 		= obstacle;
	MAP[19][16].something 		= obstacle;
	MAP[2][5].something 		= fire;
	MAP[2][5].special 			= 1;
	MAP[5][5].something 		= fire;
	MAP[5][5].special 			= 1;
	MAP[5][14].something 		= fire;
	MAP[5][14].special 			= 2;
	MAP[10][5].something 		= fire;
	MAP[10][5].special 			= 1;
	MAP[12][13].something 		= fire;
	MAP[12][13].special 		= 3;
	MAP[16][1].something 		= fire;
	MAP[16][1].special 			= 1;
	MAP[17][6].something 		= fire;
	MAP[17][6].special	 		= 1;
	MAP[17][10].something 		= fire;
	MAP[17][10].special 		= 1;
	MAP[0][1].something 		= robot;
	MAP[0][1].special 			= 3; //Le robot a un reservoire plien de 3/3
}

