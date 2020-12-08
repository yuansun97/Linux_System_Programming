/**
 * Deadlock Demolition
 * CS 241 - Spring 2020
 * 
 * Cited: haonan3
 */
#include "graph.h"
#include "libdrm.h"
#include "set.h"
#include <pthread.h>

// You probably will need some global variables here to keep track of the
// resource allocation graph.
static pthread_mutex_t gm = PTHREAD_MUTEX_INITIALIZER;
static graph* g = NULL;

struct drm_t {
    // Declare your struct's variables here. Think about what you will need.
    // Hint: You will need at least a synchronization primitive.
    pthread_mutex_t m;
};

void cleangraph(graph *g) {
    vector *v = graph_vertices(g);
    int s = vector_size(v);
    for(int i = 0; i < s; i++) {
        graph_set_vertex_value( g, vector_get(v, i), (void*) 0 );
    }
}

int isCycle(graph* g, void* node){
    int res = 0;
    if (!graph_contains_vertex(g, node)) {
        return 0;
    }
    if (graph_get_vertex_value(g, node)) {
        res = 1;
    }
    else {
        graph_set_vertex_value(g, node, (void*) 1);
        vector* v = graph_neighbors(g, node);
        int s = vector_size(v);
        for(int i = 0; i < s; i++){
            res += isCycle(g, vector_get(v, i));
        }
    }
    return res;
}




drm_t *drm_init() {
    /* Your code here */
    if(!g) {
        g = shallow_graph_create();
    }
    drm_t* drm = malloc( sizeof(drm_t) );
    graph_add_vertex( g, drm );
    pthread_mutex_init( &(drm->m), NULL );
    return drm;
}

int drm_post(drm_t *drm, pthread_t *thread_id) {
    /* Your code here */
    pthread_mutex_lock(&gm);
    if( graph_contains_vertex(g,drm) && graph_contains_vertex(g,thread_id) 
        && (graph_adjacent(g,drm,thread_id)) ) {
        graph_remove_edge(g, drm, thread_id);
        pthread_mutex_unlock(&drm->m);
        pthread_mutex_unlock(&gm);
        return 1;
    }
    pthread_mutex_unlock(&gm);
    return 0;
}

int drm_wait(drm_t *drm, pthread_t *thread_id) {
    /* Your code here */
    if(!graph_contains_vertex(g, drm)){
        return 0;
    }
    pthread_mutex_lock(&gm);
    graph_add_vertex(g, thread_id);
    graph_add_edge(g, thread_id, drm);
    if(isCycle(g, thread_id)){
        graph_remove_edge(g, thread_id, drm);
        cleangraph(g);
        pthread_mutex_unlock(&gm);
        return 0;
    }
    cleangraph(g);
    pthread_mutex_unlock(&gm);
    pthread_mutex_lock(&drm->m);
    pthread_mutex_lock(&gm);
    graph_remove_edge(g, thread_id, drm);
    graph_add_edge(g, drm, thread_id);
    pthread_mutex_unlock(&gm);
    return 1;
}

void drm_destroy(drm_t *drm) {
    /* Your code here */
    pthread_mutex_lock(&gm);
    vector* v = graph_vertices(g);
    for(unsigned int i = 0; i < vector_size(v); i++){
        vector* n = graph_neighbors(g, vector_get(v, i));
        for(unsigned int j = 0; j < vector_size(n); j++){
            if((drm_t *)vector_get(v,i) == drm || (drm_t *)vector_get(n, j) == drm){
                graph_remove_edge(g,vector_get(v, i), vector_get(n, j));
            }
        }
    }
    for(unsigned int i = 0; i < vector_size(v); i++){
        if( (drm_t *)vector_get(v,i) == drm ){
            vector_erase(v, i);
            break;
        }
    }
    pthread_mutex_destroy(&drm->m);
    free(drm);
    drm = NULL;
    pthread_mutex_unlock(&gm);
    return;
}
