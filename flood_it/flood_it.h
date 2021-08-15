#ifndef __FLOOD_IT__
#define __FLOOD_IT__
#endif

#include "graph.h"

//#define MEMORY_THRESHOLD 6442450944
#define MEMORY_THRESHOLD (100000)

typedef struct History History;
typedef struct Search_Space Search_Space;
typedef struct Board Board;

struct History {
    int color;
    History *next;
};

struct Search_Space {
    Board *board;
    Search_Space *prev, *next;
};

struct Board {
    int colors;
    int *vertices_per_color;
    List *vertices;

    double g_parameter, f_parameter;
    History *solution;
};

// Removes vertice v from board
void remove_vertice(Board *board, Vertice *v);

// Creates a copy of board
void copy_board(Board **new_board, Board **board);

// Adds all edges to the initial map
void map_preprocessing(Board *board, Vertice **vertices_matrix, int lin, int col);

// Collapses same color neighbors
void collapses_map(Board *board, Vertice *root, int color, Vertice **vertices_matrix);

void add_board(Search_Space **search_space, Board *board);
Board* remove_best_board(Search_Space **search_space);

void update_history(Board **board);

double heuristic(Board *board);
History* a_star(Board *board);
