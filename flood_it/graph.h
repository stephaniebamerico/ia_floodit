#ifndef __GRAPH__
#define __GRAPH__
#endif

typedef struct Vertice Vertice;
typedef struct List List;

extern int DEBUG;
extern long int memory_used;

struct Vertice {
    int id;
    int color;
    int area;
    List **neighbors;

    // From pt-br "gambiarra":
    // This helps when copying the board
    Vertice *copy;

    int visited; // for bfs
    int distance; // for bfs
};

struct List {
    Vertice *vertice;
    List *next;
};

// Allocates and initializes a vertice
void new_vertice(Vertice **v, int n_colors, int id, int color, int area);

// Adds dest to srcs neighborhood
void add_neighbor(Vertice *src, Vertice *dest);

// Removes dest from srcs neighborhood
void remove_neighbor(Vertice *src, Vertice *dest);
