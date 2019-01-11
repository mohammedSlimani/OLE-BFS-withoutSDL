#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define max_point 20

typedef enum bool bool;
typedef struct point point;
typedef struct queueNode queueNode;
typedef enum smth smth;
typedef struct type_coord type_coord;
typedef struct Element Element;
typedef struct File File;

typedef enum bool{ //au lieu de travailler avec 0 et 1 on va travailler avec TRUE et FALSE
	false,
	true,
}bool;

typedef struct point
{
	/*Represente les coordonneés d'un endroit dans la MAP*/
	int x;
	int y;
}point;

typedef struct queueNode
{	/*cetter structure represente les informations qui vont etre enfiler lors de la recherche du plus cours chemin */

    point pt;  // Les coordonnées d'un point
    int dist;  // La distance de ce point de la source 
}queueNode;

typedef enum smth {
	/*cetter structure represente ce qui existe dans une coordonnée d'un coordonne */
	road,
	robot,
	obstacle,
	fire
}smth;

typedef struct type_coord 
{	/*chaque endroit dans notre MAP va porter les informations suivantes*/
	smth something;
	point coord;
	int special; //sera soit le degre de danger du feu soit le 
				//reservoire que contient un robot 
	//il sera -1 pour montrer que c'est un chemin 
}type_coord;

typedef struct Element
{	/*se sont les inforamations que chaque noued du file va contenir*/
    queueNode n;
    struct Element *suivant;
}Element;

typedef struct File
{
    Element *premier; 
}File;


bool isValid(int row, int col);
File *initialiser();
void enfiler(File *file, queueNode s);
bool empty(File *F);
queueNode front(File *F);
void defiler(File *file);
void afficher();
void Load_MAP();
point* GetPath(queueNode T[max_point][max_point], point dest);
void BFS(type_coord MAP[max_point][max_point],queueNode T[max_point][max_point], point src, point dest);
void move_robot(/*point from,point to,*/int shortest/*Path*/ ,point* path);
void make_road(point A);
void ReturnToBase(int distance, point *path);
void update();
point drone();

type_coord MAP[max_point][max_point];


int main(int argc, char const *argv[])
{
	
	Load_MAP();
	afficher();
	int distance = 0;
	point station = {0,0}; //c'est la station où les robots vont etre rechargés
	point dest = drone();
	queueNode T[max_point][max_point];
	while(dest.x != -1 ){
		BFS(MAP,T,station,dest);
		distance = T[dest.x][dest.y].dist;
		point* path = GetPath(T, dest);
		move_robot(distance,path);
		sleep(2);
		make_road(dest);
		ReturnToBase(distance,path);
		dest=drone();
	}return 0;
}

/*Fonction de management des files utilisuer dans notre BFS-------------------*/
bool isValid(int row, int col)
{
    // return true if row number and column number
    // is in range
    return (row >= 0) && (row < max_point) && (col >= 0) && (col < max_point);
}

File *initialiser()
{
    File *file = malloc(sizeof(*file));
    file->premier = NULL;
    return file;
}

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

bool empty(File *F)
{
	if(F==NULL)
	  return true;
	else 
	  return false;
}

queueNode front(File *F)
{
	queueNode r=F->premier->n;
    return r;	
}

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
     
    // Mark the source cell as visited
    visited[src.x][src.y] = true;
    enfiler(q, s);  // Enqueue source cell
    
    // Do a BFS starting from source cell
    while (!empty(q))
    {
        queueNode curr = front(q);
        point pt = curr.pt;
 
        // If we have reached the destination cell,
        // we are done
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

void make_road(point A)
{
	MAP[A.x][A.y].something = road;
	MAP[A.x][A.y].special = -1;
}

void move_robot(int distance,point* path)
{
	int i;
	for (i = 0; i < distance -1 ; ++i)
	{
		MAP[path[i+1].x][path[i+1].y].something =robot;
		MAP[path[i+1].x][path[i+1].y].special = MAP[path[i].x][path[i].y].special;
		make_road(path[i]);
		sleep(1);
		update();
	}
}

void ReturnToBase(int distance, point *path){
	int i;
	point reversedpath[distance];
	for (i = 0; i < distance; ++i)
	{
		reversedpath[i] = path[distance - i -1];
	}
	move_robot(distance,reversedpath);
}



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

point drone(){
	int i ,j ;
	point firelocation = {-1,-1} ;
	for(i=0; i<20;i++){
		for (j = 0; j < 20; j++)
		{
			if (MAP[i][j].something == fire){
				firelocation.x = i;
				firelocation.y = j;
				printf("Fire foud in location %d   %d \n",i,j );
				sleep(1);
				return firelocation;
			}
		}	
	} printf("No fire Found.\n");
	firelocation.x= -1;
	firelocation.y= -1;
	return firelocation;
}


/*Telechargement des informations de la map par tous les robots */
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


void update(){
	system("@cls||clear"); 
	afficher();
}


