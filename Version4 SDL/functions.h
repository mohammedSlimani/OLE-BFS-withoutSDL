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
void move_robot(int shortest,point* path,SDL_Surface *screen,point pFire);
void make_road(point A);
void update();
point drone();
void LoadImagesToThe(SDL_Surface *screen,point pFire);

type_coord MAP[max_point][max_point];
