#include "graph.h"

#include <stdio.h>
#include <stdlib.h>

void new_vertice(Vertice **v, int n_colors, int id, int color) {
    *v = (Vertice *) malloc(sizeof(Vertice));
    (*v)->id = id;
    (*v)->color = color;
    (*v)->neighbors = (List **) malloc(n_colors * sizeof(List *));
    for (int i = 0; i < n_colors; ++i)
        (*v)->neighbors[i] = NULL;
    (*v)->copy = NULL;
}

void add_neighbor(Vertice *src, Vertice *dest) {
    if (DEBUG)
        printf("Criando %d -> %d\n", src->id, dest->id);

    List *new = (List *) malloc(sizeof(List));
    new->vertice = dest;
    new->next = NULL;

    int i_color = dest->color;
    if (src->neighbors[i_color] == NULL) {
        src->neighbors[i_color] = new;
    }
    else {
        List *i = src->neighbors[i_color];
        while (i->next != NULL && i->vertice->id != dest->id)
            i = i->next;
        if (i->vertice->id != dest->id)
            i->next = new;
    }
}

void remove_neighbor(Vertice *src, Vertice *dest) {
    if (DEBUG)
        printf("Removendo %d -> %d\n", src->id, dest->id);

    int i_color = dest->color;

    List *i_src = src->neighbors[i_color];
    List *prev = NULL;

    while (i_src->vertice->id != dest->id) {
        prev = i_src;
        i_src = i_src->next;
    }

    if (prev == NULL)
        src->neighbors[i_color] = i_src->next;
    else
        prev->next = i_src->next;

    i_src->next = NULL;
    i_src->vertice = NULL;
    free(i_src);
}
