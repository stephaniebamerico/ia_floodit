#include "flood_it.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int DEBUG = 0;

// DEBUG =====================================================
void print_map_colors(Vertice **vertices, int lin, int col) {
    printf("\nMAP %d %d:\n", lin, col);
    for (int i = 0; i < lin; ++i) {
        for (int j = 0; j < col; ++j) {
            printf("%d ", vertices[(i*col)+j]->color+1);
        }
        printf("\n");
    } 
}

void print_map_ids(Vertice **vertices, int lin, int col) {
    printf("\nMAP %d %d:\n", lin, col);
    for (int i = 0; i < lin; ++i) {
        for (int j = 0; j < col; ++j) {
            printf("%02d ", vertices[(i*col)+j]->id);
        }
        printf("\n");
    } 
}

void print_map_ids_colors(Vertice **vertices, int lin, int col) {
    printf("\nMAP %d %d:\n", lin, col);
    for (int i = 0; i < lin; ++i) {
        for (int j = 0; j < col; ++j) {
            printf("%02d(%d) ", vertices[(i*col)+j]->id, vertices[(i*col)+j]->color+1);
        }
        printf("\n");
    } 
}

void print_neighbors(List *vertices, int colors) {
    printf("\nArestas:\n");
    while (vertices != NULL) {
        printf("Vertice %d:\n", vertices->vertice->id);

        for (int c = 0; c < colors; c++) {
            printf("Cor %d: ", c+1);
            List *t = vertices->vertice->neighbors[c];

            while (t != NULL) {
                printf("%d ", t->vertice->id);
                t = t->next;
            }
            printf("\n");
        }
        vertices = vertices->next;
    } 
}

void print_list(List *vertices) {
    printf("\n%d(%d)", vertices->vertice->id, vertices->vertice->color+1);
    while (vertices->next != NULL) {
        vertices = vertices->next;
        printf("-> %d(%d)", vertices->vertice->id, vertices->vertice->color+1);
    }
    printf("\n");
}

void print_search_space(Search_Space *search_space) {    
    printf("\n\nPRINTING SEARCH SPACE\n");
    Search_Space *t = search_space;
    for (int i = 1; t != NULL; ++i) {
        printf("Board %d\n", i);
        print_list(t->board->vertices);
        t = t->next;
        printf("\n\n");
    }
    
}
// DEBUG =====================================================

void remove_vertice(Board *board, Vertice *v) {
    if (DEBUG)
        printf("Removendo %d\n", v->id);

    List *i_v = board->vertices;
    while (i_v->next != NULL && i_v->next->vertice->id != v->id)
        i_v = i_v->next;
    
    if (i_v->next != NULL) {
        board->vertices_per_color[i_v->next->vertice->color] -= 1;
        i_v->next = i_v->next->next;
        // TODO: free t e t->v
    }
}

//COMENTAR
void copy_board(Board **new_board, Board **board) {
    (*new_board) = (Board *) malloc(sizeof(Board));

    (*new_board)->colors = (*board)->colors;

    (*new_board)->vertices_per_color = (int *) malloc(sizeof(int)*(*board)->colors);  
    for (int c = 0; c < (*board)->colors; c++)
        (*new_board)->vertices_per_color[c] = (*board)->vertices_per_color[c];
    
    (*new_board)->vertices = NULL;
    List *i_board = (*board)->vertices, *i_new_board = (*new_board)->vertices;
    Vertice *v, *new_v;

    // Copy all vertices on the list
    while (i_board != NULL) {
        v = i_board->vertice;
        new_vertice(&new_v, (*board)->colors, v->id, v->color);

        if ((*new_board)->vertices == NULL) {
            (*new_board)->vertices = (List *) malloc(sizeof(List));
            i_new_board = (*new_board)->vertices;
        }
        else {
            i_new_board->next = (List *) malloc(sizeof(List));
            i_new_board = i_new_board->next;
        }
        i_new_board->vertice = new_v;
        i_new_board->next = NULL;

        v->copy = new_v;
        i_board = i_board->next;
    }

    // Copy neighbors for all vertices
    i_board = (*board)->vertices;
    i_new_board = (*new_board)->vertices;
    List *i_neighbor;
    while (i_new_board != NULL) {
        for (int i_color = 0; i_color < (*board)->colors; ++i_color) {
            i_neighbor = i_board->vertice->neighbors[i_color];

            while (i_neighbor != NULL) {
                add_neighbor(i_new_board->vertice, i_neighbor->vertice->copy);
                add_neighbor(i_neighbor->vertice->copy, i_new_board->vertice);
                i_neighbor = i_neighbor->next;
            }
        }
        i_new_board = i_new_board->next;
        i_board = i_board->next;
    }

    (*new_board)->g_parameter = (*board)->g_parameter;
    (*new_board)->f_parameter = (*board)->f_parameter;

    if((*board)->solution != NULL) {
        (*new_board)->solution = (History *) malloc(sizeof(History));
        (*new_board)->solution->color = (*board)->solution->color;
        (*new_board)->solution->next = NULL;

        History *n_t = (*new_board)->solution;
        for (History *t = (*board)->solution->next; t != NULL; t = t->next) {
            n_t->next = (History *) malloc(sizeof(History));
            n_t = n_t->next;

            n_t->color = t->color;
            n_t->next = NULL;
        }
    }
    else
        (*new_board)->solution = NULL;    
}

void map_preprocessing(Board *board, Vertice **vertices_matrix, int lin, int col) {
    // Adds all edges from all vertices on map
    Vertice *v, *r, *b;

    for (int i = 0; i < (lin - 1); ++i) {
        for (int j = 0; j < (col - 1); ++j) {
            v = vertices_matrix[(i * col) + j];
            b = vertices_matrix[((i + 1) * col) + j];
            r = vertices_matrix[(i * col) + j + 1];

            // Right neighbor
            add_neighbor(v, r);
            add_neighbor(r, v);

            // Bottom neighbor
            add_neighbor(v, b);
            add_neighbor(b, v);
        }
    }

    for (int i = 0; i < lin-1; ++i) {
        // Rightmost vertices
        v = vertices_matrix[(i * col) + col - 1];
        b = vertices_matrix[((i + 1) * col) + col - 1];

        // Bottom neighbor
        add_neighbor(v, b);
        add_neighbor(b, v);
    }

    for (int i = 0; i < col - 1; ++i) {
        // Bottom vertices
        v = vertices_matrix[((lin - 1) * col) + i];
        r = vertices_matrix[((lin - 1) * col) + i + 1];

        // Right neighbor
        add_neighbor(v, r);
        add_neighbor(r, v);
    }

    // Collapses same color neighbors from all vertices on map
    for (int i = 0; i < lin*col; ++i)
        if (vertices_matrix[i] != NULL)
            collapses_map(board, vertices_matrix[i], vertices_matrix[i]->color);
}

void collapses_map(Board *board, Vertice *root, int color) {
    if (DEBUG)
        printf("\n[collapses_map]\n==================\nColapsando %d com cor %d!\n", root->id, color+1);
    
    List *i_root, *i_neighbor, *next;
    Vertice *neighbor;

    i_root = root->neighbors[color];
    // Root absorbs neighbor's neighborhood 
    while (i_root != NULL) {
        neighbor = i_root->vertice;

        if (DEBUG)
            printf("\nColapsando neighbor %d:\n", neighbor->id);

        remove_neighbor(neighbor, root);
    
        for (int i_color = 0; i_color < board->colors; ++i_color) {
            i_neighbor = neighbor->neighbors[i_color];

            if (DEBUG)
                printf(". Copiando lista de cor %d\n", i_color+1);

            while (i_neighbor != NULL) {
                add_neighbor(root, i_neighbor->vertice);
                add_neighbor(i_neighbor->vertice, root);

                next = i_neighbor->next;
                remove_neighbor(i_neighbor->vertice, neighbor);
                remove_neighbor(neighbor, i_neighbor->vertice);

                if (DEBUG)
                    printf(".. %d copiado\n", i_neighbor->vertice->id);
                i_neighbor = next;
            }
        }

        i_root = i_root->next;
        remove_neighbor(root, neighbor);
        remove_vertice(board, neighbor);
    }

    root->color = color;
}

void add_board(Search_Space **search_space, Board *board) {
    Search_Space *t = *search_space;
    if(t != NULL) {
        while(t->next != NULL)
            t = t->next;
        t->next = (Search_Space *) malloc(sizeof(Search_Space));
        t = t->next;  
    }
    else {
        t = (Search_Space *) malloc(sizeof(Search_Space));
        *search_space = t;
    }

    t->board = board;
    t->next = NULL;
}

void remove_board(Search_Space **search_space) {
    *search_space = (*search_space)->next;
    //TODO FREE
}

void update_history(Board **board) {
    History *t = (*board)->solution;
    if(t != NULL) {
        while (t->next != NULL)
            t = t->next;
        
        t->next = (History *) malloc(sizeof(History));
        t = t->next;
        t->color = (*board)->vertices->vertice->color + 1;
        t->next = NULL;
    }
    else {
        (*board)->solution = (History *) malloc(sizeof(History));
        t = (*board)->solution;
    }
    t->color = (*board)->vertices->vertice->color + 1;
    t->next = NULL;
}

int heuristic(Board *board) {
    int h = 0;
    for (int c = 0; c < board->colors; c++) {
        h += board->vertices_per_color[c];
    }
    
    return h;
}

History* a_star(Board *board) {
    // Estado inicial
    board->g_parameter = 0;
    board->f_parameter = heuristic(board);
    board->solution = NULL;

    // Insere t na lista l
    Search_Space *search_space = (Search_Space *) malloc(sizeof(Search_Space));
    search_space->board = board;
    search_space->next = NULL;

    Board *q = search_space->board;
    while (q->vertices->next != NULL) {
        printf("=======================================================\n");
        // Acha o board com o menor f
        q = search_space->board;
        remove_board(&search_space);

        Search_Space *t = search_space;
        while (t != NULL) {
            if (t->board->f_parameter < q->f_parameter) {
                // Add q antigo na lista
                add_board(&search_space, q);

                q = t->board;
                
                // Remove q novo da lista
                remove_board(&t);
            }
            //printf("f: %d\n", t->board->f_parameter);
            t = t->next;
        }

        print_list(q->vertices);
        for (int i = 0; i < board->colors; i++)
            printf("Cor %d: %d\n", i+1, board->vertices_per_color[i]);
        

        // Gera os secessores de q
        for (int new_color = 0; new_color < q->colors; new_color++) {
            Vertice *root = q->vertices->vertice;
            Board *new_board;
            if (root->neighbors[new_color] != NULL) {
                copy_board(&new_board, &q);

                Vertice *new_root = new_board->vertices->vertice;
                new_board->vertices_per_color[new_root->color] -= 1;
                collapses_map(new_board, new_root, new_color);
                new_board->vertices_per_color[new_root->color] += 1;
                update_history(&new_board);

                // Se sucessor e solucao, para
                if(new_board->vertices->next == NULL) {
                    return new_board->solution;
                }
                
                // senao, insere sucessor na lista
                new_board->g_parameter = q->g_parameter + 1;
                new_board->f_parameter = new_board->g_parameter + heuristic(new_board);
                add_board(&search_space, new_board);
            }
        }
        printf("\n\n");
        //print_search_space(search_space);
    }
    return q->solution;
}

int main(int argc, char** argv) {
    // Activates debug mode if parameter was passed
    if (argc > 1)
        if (strcmp(argv[1], "-d") || strcmp(argv[1], "--debug"))
            DEBUG = 1;
    
    // Reads map as both matrix and list, creating a vertice for each cell
    // The matrix will be useful for preprocessing and the list will be used in the long run
    Board *board = (Board *) malloc(sizeof(Board));
    board->g_parameter = 0;
    board->vertices = NULL;
    int lin, col;

    scanf("%d %d %d", &lin, &col, &(board->colors));

    board->vertices_per_color = (int *) malloc(sizeof(int)*board->colors);
    for (int i = 0; i < board->colors; i++) 
        board->vertices_per_color[i] = 0;    

    int n_vertices = lin * col;
    
    Vertice **vertices_matrix = (Vertice **) malloc(n_vertices * sizeof(Vertice *));
    Vertice *v; 
    List *i_v;
    int color;
    for (int i = 0; i < n_vertices; ++i) {
        scanf("%d", &color);
        new_vertice(&v, board->colors, i, color-1);

        vertices_matrix[i] = v;

        if (board->vertices == NULL) {
            board->vertices = (List *) malloc(sizeof(List));
            i_v = board->vertices;
        }
        else {
            i_v->next = (List *) malloc(sizeof(List));
            i_v = i_v->next;
        }
        board->vertices_per_color[v->color] += 1;

        i_v->vertice = v;
        i_v->next = NULL;
    }

    if (DEBUG) {
        print_map_ids_colors(vertices_matrix, lin, col);
        print_list(board->vertices);
    }
    
    // Create all necessary edges and collapse cells with same color
    map_preprocessing(board, vertices_matrix, lin, col);
    // TODO: free matrix
    free(vertices_matrix);
    
    if (DEBUG) {
        print_neighbors(board->vertices, board->colors);
        print_list(board->vertices);
    }

    History *solution = a_star(board);
    int solution_size = 0;
    for (History *i = solution; i != NULL; i = i->next)
        ++solution_size;
    printf("%d\n", solution_size);
    for (History *i = solution; i != NULL; i = i->next)
        printf("%d ", i->color);
    printf("\n");
}