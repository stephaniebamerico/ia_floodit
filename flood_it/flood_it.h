#ifndef __FLOOD_IT__
#define __FLOOD_IT__
#endif

#include "graph.h"

typedef struct History History;
typedef struct Search_Space Search_Space;
typedef struct Board Board;

struct History {
    int color;
    History *next;
};

struct Search_Space {
    Board *board;
    Search_Space *next;
};

struct Board {
    int colors;
    int *vertices_per_color;
    List *vertices;

    int g_parameter, f_parameter;
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
void remove_board(Search_Space **search_space, Search_Space *remove);

void update_history(Board **board);

int heuristic(Board *board);
History* a_star(Board *board);
