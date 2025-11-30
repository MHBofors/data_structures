#include "graph.h"

#define VERTEX_ARRAY_INIT_SIZE 32
#define EDGE_ARRAY_INIT_SIZE 32

struct graph_t {
	size_t vertex_size;
	size_t vertex_count;
	size_t vertex_capacity;

	size_t edge_size;
	size_t edge_count;
	size_t edge_capacity;

	size_t *column_indices;
	size_t *row_indices;

	void *vertex_array;
	void *edge_array;
};

graph_t *graph_alloc(size_t vertex_size, size_t edge_size) {
	graph_t *graph = NULL;
	void *vertex_array = NULL;
	void *edge_array = NULL;
	size_t *column_indices = NULL;
	size_t *row_indices = NULL;

	if ((graph = malloc(sizeof(graph_t))) &&
	    (vertex_array = malloc(VERTEX_ARRAY_INIT_SIZE * vertex_size)) &&
	    (edge_array = malloc(EDGE_ARRAY_INIT_SIZE * edge_size)) &&
	    (column_indices = malloc(EDGE_ARRAY_INIT_SIZE * sizeof(size_t))) &&
	    (row_indices = malloc(VERTEX_ARRAY_INIT_SIZE * sizeof(size_t)))) {
		*graph = (graph_t){.vertex_size = vertex_size,
		                   .vertex_count = 0,
		                   .vertex_capacity = VERTEX_ARRAY_INIT_SIZE,
		                   .edge_size = edge_size,
		                   .edge_count = 0,
		                   .edge_capacity = EDGE_ARRAY_INIT_SIZE,
		                   .column_indices = column_indices,
		                   .row_indices = row_indices,
		                   .vertex_array = vertex_array,
		                   .edge_array = edge_array};

		return graph;
	} else {
		free(graph);
		free(vertex_array);
		free(edge_array);
		free(column_indices);
		free(row_indices);
		return NULL;
	}
}

void graph_free(graph_t *graph) {
	if (graph) {
		free(graph->vertex_array);
		free(graph->edge_array);
		free(graph->column_indices);
		free(graph->row_indices);
		free(graph);
	}
}

size_t graph_vertex_count(graph_t *graph) { return graph->vertex_count; }

size_t graph_edge_count(graph_t *graph) { return graph->edge_count; }

size_t graph_vertex_out_edge_count(graph_t *graph, size_t vertex_index) {
	if (vertex_index >= graph->vertex_count) {
		return 0;
	}

	return graph->row_indices[vertex_index + 1] -
	       graph->row_indices[vertex_index];
}

bool graph_is_adjacent(graph_t *graph, size_t from_vertex_index,
                       size_t to_vertex_index) {
	if (from_vertex_index >= graph->vertex_count ||
	    to_vertex_index >= graph->vertex_count) {
		return false;
	}

	size_t row_start = graph->row_indices[from_vertex_index];
	size_t row_end = graph->row_indices[from_vertex_index + 1];

	for (size_t i = row_start; i < row_end; i++) {
		if (graph->column_indices[i] == to_vertex_index) {
			return true;
		}
	}

	return false;
}

bool graph_add_vertex(graph_t *graph, void *vertex_data) {
	if (graph->vertex_size != 0 && vertex_data == NULL) {
		return false;
	}

	if (graph->vertex_capacity + 1 <= graph->vertex_count) {
		size_t new_capacity = 2 * graph->vertex_capacity;

		void *new_vertex_array;
		size_t *new_row_indices;
		if ((new_vertex_array = realloc(graph->vertex_array,
		                                new_capacity * graph->vertex_size)) &&
		    (new_row_indices =
		         realloc(graph->row_indices, new_capacity * sizeof(size_t)))) {
			graph->vertex_array = new_vertex_array;
			graph->row_indices = new_row_indices;
			graph->vertex_capacity = new_capacity;
		} else {
			return false;
		}
	}

	if (graph->vertex_size != 0) {
		memcpy((char *)graph->vertex_array +
		           graph->vertex_size * graph->vertex_count,
		       vertex_data, graph->vertex_size);
	}

	graph->row_indices[graph->vertex_count] = graph->edge_count;
	graph->vertex_count++;

	return true;
}

bool graph_insert_edge(graph_t *graph, size_t from_vertex_index,
                       size_t to_vertex_index, void *edge_data) {
	if (graph_is_adjacent(graph, from_vertex_index, to_vertex_index)) {
		return false;
	}

	if (from_vertex_index >= graph->vertex_count ||
	    to_vertex_index >= graph->vertex_count) {
		return false;
	}

	if (graph->edge_capacity <= graph->edge_count) {
		size_t new_capacity = 2 * graph->edge_capacity;

		void *new_edge_array;
		size_t *new_column_indices;
		if ((new_edge_array =
		         realloc(graph->edge_array, new_capacity * graph->edge_size)) &&
		    (new_column_indices = realloc(graph->column_indices,
		                                  new_capacity * sizeof(size_t)))) {
			graph->edge_array = new_edge_array;
			graph->column_indices = new_column_indices;
			graph->edge_capacity = new_capacity;
		} else {
			return false;
		}
	}

	memmove(graph->column_indices +
	            sizeof(size_t) *
	                (graph->row_indices[from_vertex_index + 1] + 1),
	        graph->column_indices +
	            sizeof(size_t) * graph->row_indices[from_vertex_index + 1],
	        sizeof(size_t) *
	            (graph->edge_count - graph->row_indices[from_vertex_index]));

	if (edge_data != NULL) {
		memmove(
		    (char *)graph->edge_array +
		        graph->edge_size *
		            (graph->row_indices[from_vertex_index + 1] + 1),
		    (char *)graph->edge_array +
		        graph->edge_size * graph->row_indices[from_vertex_index + 1],
		    graph->edge_size *
		        (graph->edge_count - graph->row_indices[from_vertex_index]));

		memcpy((char *)graph->edge_array +
		           graph->edge_size * graph->row_indices[from_vertex_index + 1],
		       edge_data, graph->edge_size);
	}

	for (size_t i = from_vertex_index + 1; i <= graph->vertex_count; i++) {
		graph->row_indices[i]++;
	}

	graph->column_indices[graph->edge_count] = to_vertex_index;
	graph->edge_count++;
}

bool graph_set_vertex(graph_t *graph, size_t index, void *vertex_data) {}

bool graph_set_edge(graph_t *graph, size_t from_vertex_index,
                    size_t to_vertex_index, void *edge_data) {}

void *graph_get_vertex(graph_t *graph, size_t vertex_index,
                       void *out_vertex_data) {
	if (vertex_index >= graph->vertex_count) {
		return NULL;
	}

	void *vertex_ptr =
	    (char *)graph->vertex_array + graph->vertex_size * vertex_index;

	if (out_vertex_data) {
		memcpy(out_vertex_data, vertex_ptr, graph->vertex_size);
		return out_vertex_data;
	} else {
		return vertex_ptr;
	}
}

void *graph_get_edge(graph_t *graph, size_t from_vertex_index,
                     size_t to_vertex_index, void *out_edge_data) {
	size_t row_start = graph->row_indices[from_vertex_index];
	size_t row_end = graph->row_indices[from_vertex_index + 1];

	for (size_t i = row_start; i < row_end; i++) {
		if (graph->column_indices[i] == to_vertex_index) {
			void *edge_ptr = (char *)graph->edge_array + graph->edge_size * i;

			if (out_edge_data) {
				memcpy(out_edge_data, edge_ptr, graph->edge_size);
				return out_edge_data;
			} else {
				return edge_ptr;
			}
		}
	}

	return NULL;
}
