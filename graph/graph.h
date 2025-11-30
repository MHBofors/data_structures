#ifndef GRAPH_H
#define GRAPH_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct graph_t graph_t;

graph_t *graph_alloc(size_t vertex_size, size_t edge_size);
void graph_free(graph_t *graph);

size_t graph_vertex_count(graph_t *graph);
size_t graph_edge_count(graph_t *graph);
size_t graph_vertex_edge_count(graph_t *graph, size_t vertex_index);

bool graph_is_adjacent(graph_t *graph, size_t from_vertex_index,
                       size_t to_vertex_index);

bool graph_add_vertex(graph_t *graph, void *vertex_data);
bool graph_add_edge(graph_t *graph, size_t from_vertex_index,
                    size_t to_vertex_index, void *edge_data);

bool graph_set_vertex(graph_t *graph, size_t index, void *vertex_data);
bool graph_set_edge(graph_t *graph, size_t from_vertex_index,
                    size_t to_vertex_index, void *edge_data);

void *graph_get_vertex(graph_t *graph, size_t vertex_index,
                       void *out_vertex_data);
void *graph_get_edge(graph_t *graph, size_t from_vertex_index,
                     size_t to_vertex_index, void *out_edge_data);

#endif // GRAPH_H