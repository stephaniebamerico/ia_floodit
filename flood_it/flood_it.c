#include "flood_it.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO fix prune when full memory
// TODO [enhance] use better types e.g. short instead of int
// TODO [enhance] comment everything
// TODO [enhance] translate debug messages to english
// TODO [enhance] use preprocessor for debug sections

int DEBUG = 0;
long int memory_used;

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
    if (vertices == NULL)
        return;

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

void remove_board(Board *board) {
    while (board->vertices != NULL) {
        List *l = board->vertices;
        Vertice *v = board->vertices->vertice;

        for (int c = 0; c < board->colors; c++) {
            while (v->neighbors[c] != NULL) {
                List *i_v = v->neighbors[c];

                v->neighbors[c] = v->neighbors[c]->next;

                i_v->next = NULL;
                i_v->vertice = NULL;
                free(i_v);
                memory_used -= sizeof(List);
            }
        }

        board->vertices = l->next;
        l->next = NULL;
        l->vertice = NULL;
        v->copy = NULL;

        free(v->neighbors);
        free(v);
        free(l);
        memory_used -= board->colors * sizeof(List *);
        memory_used -= sizeof(Vertice);
        memory_used -= sizeof(List);
    }

    while (board->solution != NULL) {
        History *cur_solution = board->solution;
        board->solution = board->solution->next;

        cur_solution->next = NULL;
        free(cur_solution);
        memory_used -= sizeof(History);
    }

    free(board->vertices_per_color);
    memory_used -= board->colors * sizeof(int);
    free(board);
    memory_used -= sizeof(Board);
}

void prune(Search_Space **search_space) {
    while (memory_used >= MEMORY_THRESHOLD && *search_space != NULL) {
        printf("Memory used %ld\n", memory_used);
        int nboards = 0;
        Search_Space *c = (*search_space);
        do {
            ++nboards;
            c = c->next;
        } while (c != (*search_space));
        printf("Boards %d\n", nboards);

        Search_Space *cur = (*search_space)->prev; // go to max board at the end of the queue

        if (cur->next == cur) // queue is one element only
            (*search_space) = NULL;

        cur->prev->next = cur->next;
        cur->next->prev = cur->prev;

        cur->prev = NULL;
        cur->next = NULL;

        remove_board(cur->board);

        cur->board = NULL;
        free(cur);
        memory_used -= sizeof(Search_Space);
    }
}

void remove_vertice(Board *board, Vertice *v) {
    if (DEBUG)
        printf("Removendo %d\n", v->id);
    // we never remove the root (first on the list) so don't need to check for
    // it and update the head

    List *i_v = board->vertices;
    while (i_v->next->vertice->id != v->id)
        i_v = i_v->next;

    board->vertices_per_color[i_v->next->vertice->color] -= 1;

    List *remove = i_v->next;
    i_v->next = i_v->next->next;

    free(remove->vertice->neighbors);
    free(remove->vertice);
    free(remove);
    memory_used -= board->colors * sizeof(List *);
    memory_used -= sizeof(Vertice);
    memory_used -= sizeof(List);
}

void copy_board(Board **new_board, Board **board) {
    (*new_board) = (Board *) malloc(sizeof(Board));
    memory_used += sizeof(Board);
    (*new_board)->colors = (*board)->colors;
    (*new_board)->vertices_per_color = (int *) malloc(sizeof(int)*(*board)->colors);
    memory_used += sizeof(int) * (*board)->colors;

    for (int c = 0; c < (*board)->colors; c++)
        (*new_board)->vertices_per_color[c] = (*board)->vertices_per_color[c];

    (*new_board)->vertices = NULL;
    List *i_board = (*board)->vertices, *i_new_board = (*new_board)->vertices;
    Vertice *v, *new_v;

    // Copy all vertices on the list
    while (i_board != NULL) {
        v = i_board->vertice;
        new_vertice(&new_v, (*board)->colors, v->id, v->color, v->area);

        if ((*new_board)->vertices == NULL) {
            (*new_board)->vertices = (List *) malloc(sizeof(List));
            memory_used += sizeof(List);

            i_new_board = (*new_board)->vertices;
        }
        else {
            i_new_board->next = (List *) malloc(sizeof(List));
            memory_used += sizeof(List);

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
        memory_used += sizeof(History);

        (*new_board)->solution->color = (*board)->solution->color;
        (*new_board)->solution->next = NULL;

        History *n_t = (*new_board)->solution;
        for (History *t = (*board)->solution->next; t != NULL; t = t->next) {
            n_t->next = (History *) malloc(sizeof(History));
            memory_used += sizeof(History);

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
            collapses_map(board, vertices_matrix[i], vertices_matrix[i]->color, vertices_matrix);
}

void collapses_map(Board *board, Vertice *root, int color, Vertice **vertices_matrix) {
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

        root->area += neighbor->area;
        remove_neighbor(neighbor, root);

        for (int i_color = 0; i_color < board->colors; ++i_color) {
            i_neighbor = neighbor->neighbors[i_color];

            if (DEBUG)
                printf(". Copiando lista de cor %d\n", i_color+1);

            while (i_neighbor != NULL) {
                int i_neighbor_id = i_neighbor->vertice->id;

                add_neighbor(root, i_neighbor->vertice);
                add_neighbor(i_neighbor->vertice, root);

                next = i_neighbor->next;
                remove_neighbor(i_neighbor->vertice, neighbor);
                remove_neighbor(neighbor, i_neighbor->vertice);

                if (DEBUG)
                    printf(".. %d copiado\n", i_neighbor_id);

                i_neighbor = next;
            }
        }

        i_root = i_root->next;
        remove_neighbor(root, neighbor);
        if (vertices_matrix != NULL)
            vertices_matrix[neighbor->id] = NULL;
        remove_vertice(board, neighbor);
    }

    // if we are changing root color, we need to update the neighborhood list to
    // the right color
    List *root_neighbors_to_add = NULL;
    if (color != root->color) {
        for (int i_color = 0; i_color < board->colors; ++i_color) {
            i_root = root->neighbors[i_color];
            while (i_root != NULL) {
                Vertice *neighbor = i_root->vertice;
                remove_neighbor(neighbor, root);

                List *add = (List *) malloc(sizeof(List));
                memory_used += sizeof(List);

                add->vertice = neighbor;
                add->next = root_neighbors_to_add;

                root_neighbors_to_add = add;

                i_root = i_root->next;
            }
        }
    }

    root->color = color;

    while (root_neighbors_to_add != NULL) {
        Vertice *add = root_neighbors_to_add->vertice;

        add_neighbor(add, root);

        List *remove = root_neighbors_to_add;
        root_neighbors_to_add = root_neighbors_to_add->next;

        remove->next = NULL;
        remove->vertice = NULL;
        free(remove);
        memory_used -= sizeof(List);
    }
}

void add_board(Search_Space **search_space, Board *board) {
    Search_Space *cur = *search_space;

    if (cur == NULL) { // empty queue
        cur = (Search_Space *) malloc(sizeof(Search_Space));
        memory_used += sizeof(Search_Space);

        cur->prev = cur;
        cur->next = cur;
        cur->board = board;
        (*search_space) = cur;
    }
    else if (board->f_parameter < cur->board->f_parameter) { // insert at the beginning
        Search_Space *add = (Search_Space *) malloc(sizeof(Search_Space));
        memory_used += sizeof(Search_Space);

        add->board = board;

        add->prev = cur->prev;
        cur->prev->next = add;

        add->next = cur;
        cur->prev = add;

        (*search_space) = add;
    }
    else { // insert at the middle
        cur = cur->next;

        // until we are back at the beginning
        while(cur != (*search_space) && cur->board->f_parameter < board->f_parameter) {
            cur = cur->next;
        }

        Search_Space *add = (Search_Space *) malloc(sizeof(Search_Space));
        memory_used += sizeof(Search_Space);

        add->board = board;

        add->prev = cur->prev;
        cur->prev->next = add;

        add->next = cur;
        cur->prev = add;
    }
}

Board* get_best_board(Search_Space **search_space) {
    Search_Space *cur = *search_space;
    Board *ret = cur->board;

    if (cur->next == cur) // queue is one element only
        (*search_space) = NULL;
    else // we always remove the head, so update it
        (*search_space) = cur->next;

    cur->prev->next = cur->next;
    cur->next->prev = cur->prev;

    cur->prev = NULL;
    cur->next = NULL;
    cur->board = NULL;
    free(cur);
    memory_used -= sizeof(Search_Space);

    return ret;
}

void update_history(Board **board) {
    History *t = (*board)->solution;
    if(t != NULL) {
        while (t->next != NULL)
            t = t->next;

        t->next = (History *) malloc(sizeof(History));
        memory_used += sizeof(History);

        t = t->next;
        t->color = (*board)->vertices->vertice->color + 1;
        t->next = NULL;
    }
    else {
        (*board)->solution = (History *) malloc(sizeof(History));
        memory_used += sizeof(History);

        t = (*board)->solution;
    }
    t->color = (*board)->vertices->vertice->color + 1;
    t->next = NULL;
}

double heuristic(Board *board) {
    double h = 0;

    // sum of vertices yet to flood
    int h_colors = 0;
    for (int c = 0; c < board->colors; c++) {
        h_colors += board->vertices_per_color[c];
    }
    // sum of vertices yet to flood

    // calc area of the board
    int area_flooded = board->vertices->vertice->area;
    int area_to_flood = 0;
    List *l = board->vertices->next;
    while (l != NULL) {
        area_to_flood += l->vertice->area;
        l = l->next;
    }
    // calc area of the board

    // calc area of root and adjacents
    int area_root_adj = 0;
    int n_root_neighbors = 0;
    for (int c = 0; c < board->colors; c++) {
        List *neigh = board->vertices->vertice->neighbors[c];

        while (neigh != NULL) {
            area_root_adj += neigh->vertice->area;
            ++n_root_neighbors;
            neigh = neigh->next;
        }
    }
    // calc area of root and adjacents

    // bfs
      // reset graph
    l = board->vertices;
    while (l != NULL) {
        l->vertice->visited = 0;
        l->vertice->distance = 0;
        l = l->next;
    }
      // reset graph

    List *queue = (List *) malloc(sizeof(List));
    memory_used += sizeof(List);
    queue->next = NULL;
    queue->vertice = board->vertices->vertice;
    queue->vertice->visited = 1;
    while (queue != NULL) {
        l = queue;
        Vertice *v = queue->vertice;
        queue = queue->next;
        l->next = NULL;
        l->vertice = NULL;
        free(l);
        memory_used -= sizeof(List);

        for (int c = 0; c < board->colors; c++) {
            List *neigh = v->neighbors[c];

            while (neigh != NULL) {
                if (neigh->vertice->visited == 0) {
                    neigh->vertice->visited = 1;
                    neigh->vertice->distance = v->distance + 1;

                    List *add = (List *) malloc(sizeof(List));
                    memory_used += sizeof(List);
                    add->vertice = neigh->vertice;
                    add->next = NULL;
                    List *queue_trav = queue;
                    List *prev = NULL;
                    while (queue_trav != NULL) {
                        prev = queue_trav;
                        queue_trav = queue_trav->next;
                    }
                    if (prev != NULL)
                        prev->next = add;
                    else
                        queue = add;
                }
                neigh = neigh->next;
            }
        }
    }

      // sum distances
    int sum_distances = 0;
    l = board->vertices;
    while (l != NULL) {
        sum_distances += l->vertice->distance;
        l = l->next;
    }
      // sum distances
    // bfs

    //h = h_colors;
    //h = h_colors * 0.85 + (area_to_flood/(double)area_flooded) * 0.15;
    //h = h_colors * 0.75 + (area_to_flood/(double)area_flooded) * 0.2 + area_root_adj * 0.05;
    h = ((h_colors / (double) board->colors) * 0.50) +
        (area_to_flood * 0.25) +
        (area_root_adj * 0.08) -
        (n_root_neighbors * 0.13) +
        (sum_distances * 0.27);

    return h;
}

History* a_star(Board *board) {
    // Estado inicial
    board->g_parameter = 0;
    board->f_parameter = heuristic(board);
    board->solution = NULL;

    Search_Space *search_space = NULL;
    add_board(&search_space, board);

    Board *q = get_best_board(&search_space);
    while (q->vertices->next != NULL) { // if this is false, then there is only one vertice in the graph, thus it is a board fully flooded
//        printf("Memory used %ld\n", memory_used);
//        printf("H = %lf\n", q->f_parameter);
        //printf("=======================================================\n");
        //print_list(q->vertices);
        //for (int i = 0; i < q->colors; i++)
        //    printf("Cor %d: %d\n", i+1, q->vertices_per_color[i]);
        //printf("\n\n");

        // generate sucessors of q board
        for (int new_color = 0; new_color < q->colors; new_color++) {
            Vertice *root = q->vertices->vertice;
            Board *new_board;

            if (root->neighbors[new_color] != NULL) {
                copy_board(&new_board, &q);

                Vertice *new_root = new_board->vertices->vertice;
                new_board->vertices_per_color[new_root->color] -= 1;
                collapses_map(new_board, new_root, new_color, NULL);
                new_board->vertices_per_color[new_root->color] += 1;
                update_history(&new_board);


                // if this sucessor is a solution, stop
                if(new_board->vertices->next == NULL) {
                    return new_board->solution;
                }

                // otherwise, inserts it into the queue to explore
                new_board->g_parameter = q->g_parameter + 1;
                new_board->f_parameter = new_board->g_parameter + heuristic(new_board);
                add_board(&search_space, new_board);
            }

//            if (memory_used >= MEMORY_THRESHOLD)
//                prune(&search_space);
        }

        remove_board(q);

        q = get_best_board(&search_space);
        if (q == NULL)
            return NULL;
    }
    return q->solution;
}

int main(int argc, char** argv) {
    memory_used = 0;

    // Activates debug mode if parameter was passed
    if (argc > 1)
        if (strcmp(argv[1], "-d") || strcmp(argv[1], "--debug"))
            DEBUG = 1;

    // Reads map as both matrix and list, creating a vertice for each cell
    // The matrix will be useful for preprocessing and the list will be used in the long run
    Board *board = (Board *) malloc(sizeof(Board));
    memory_used += sizeof(Board);

    board->g_parameter = 0;
    board->vertices = NULL;

    int lin, col;
    scanf("%d %d %d", &lin, &col, &(board->colors));

    board->vertices_per_color = (int *) calloc(board->colors, sizeof(int));
    memory_used += board->colors * sizeof(int);

    int n_vertices = lin * col;

    Vertice **vertices_matrix = (Vertice **) malloc(n_vertices * sizeof(Vertice *));
    memory_used += n_vertices * sizeof(Vertice *);

    Vertice *v;
    List *i_v;
    int color;
    for (int i = 0; i < n_vertices; ++i) {
        scanf("%d", &color);
        new_vertice(&v, board->colors, i, color-1, 1);

        vertices_matrix[i] = v;

        // alloc space on board's list of vertices to the new vertice
        if (board->vertices == NULL) {
            board->vertices = (List *) malloc(sizeof(List));
            memory_used += sizeof(List);

            i_v = board->vertices;
        }
        else {
            i_v->next = (List *) malloc(sizeof(List));
            memory_used += sizeof(List);

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
    free(vertices_matrix);
    memory_used -= n_vertices * sizeof(Vertice *);

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
