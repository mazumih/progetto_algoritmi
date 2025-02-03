/* Nome: Mihai
   Cognome: Mazuru
   Classe: A
   Matricola: 0001080591
   E-mail: mihai.mazuru@studio.unibo.it */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <string.h>

const int NODE_UNDEF = -1;
/* costante aggiunta per evitare l'uso dei magic numbers */
const int LINES_IN_FILE = 4;

/* strutture utilizzate durante il corso */
typedef struct {
    int key;
    double prio;
} HeapElem;

typedef struct {
    HeapElem *heap;
    int *pos; 
    int n; 
    int size; 
} MinHeap;

typedef struct Edge {
    int src;            
    int dst;            
    double weight;      
    struct Edge *next;
} Edge;

typedef enum { GRAPH_UNDIRECTED, GRAPH_DIRECTED } Graph_type;

typedef struct {
    int n;              
    int m;              
    Graph_type t;       
    Edge **edges;       
    int *in_deg;        
    int *out_deg;       
} Graph;

/* funzioni per creare/modficare/cancellare minheap */
void minheap_clear( MinHeap *h )
{
    int i;
    assert(h != NULL);
    for (i=0; i<h->size; i++) {
        h->pos[i] = -1;
    }
    h->n = 0;
}

MinHeap *minheap_create(int size)
{
    MinHeap *h = (MinHeap*)malloc(sizeof(*h));
    assert(h != NULL);
    assert(size > 0);

    h->size = size;
    h->heap = (HeapElem*)malloc(size * sizeof(*(h->heap)));
    assert(h->heap != NULL);
    h->pos = (int*)malloc(size * sizeof(*(h->pos)));
    assert(h->pos != NULL);
    minheap_clear(h);
    return h;
}

void minheap_destroy( MinHeap *h )
{
    assert(h != NULL);

    h->n = h->size = 0;
    free(h->heap);
    free(h->pos);
    free(h);
}

static int valid(const MinHeap *h, int i)
{
    assert(h != NULL);

    return ((i >= 0) && (i < h->n));
}

static void swap(MinHeap *h, int i, int j)
{
    HeapElem tmp;

    assert(h != NULL);
    assert(valid(h, i));
    assert(valid(h, j));
    assert(h->pos[h->heap[i].key] == i);
    assert(h->pos[h->heap[j].key] == j);

    tmp = h->heap[i];
    h->heap[i] = h->heap[j];
    h->heap[j] = tmp;

    h->pos[h->heap[i].key] = i;
    h->pos[h->heap[j].key] = j;
}

static int parent(const MinHeap *h, int i)
{
    assert(valid(h, i));

    return (i+1)/2 - 1;
}

static void move_up(MinHeap *h, int i)
{
    int p;

    assert(valid(h, i));

    p = parent(h, i);
    while ( valid(h, p) && (h->heap[i].prio < h->heap[p].prio) ) {
        swap(h, i, p);
        i = p;
        p = parent(h, i);
    }
}

int minheap_is_full(const MinHeap *h)
{
    assert(h != NULL);

    return (h->n == h->size);
}

void minheap_insert(MinHeap *h, int key, double prio)
{
    int i;

    assert( !minheap_is_full(h) );
    assert((key >= 0) && (key < h->size));
    assert(h->pos[key] == -1);

    i = h->n++;
    h->pos[key] = i;
    h->heap[i].key = key;
    h->heap[i].prio = prio;
    move_up(h, i);
}

int minheap_is_empty(const MinHeap *h)
{
    assert(h != NULL);

    return (h->n == 0);
}

static int rchild(const MinHeap *h, int i)
{
    assert(valid(h, i));

    return 2*i + 2;
}

static int lchild(const MinHeap *h, int i)
{
    assert(valid(h, i));

    return 2*i + 1;
}

static int min_child(const MinHeap *h, int i)
{
    int l, r, result = -1;

    assert(valid (h, i));

    l = lchild(h, i);
    r = rchild(h, i);

    if (valid(h, l)) {
        result = l;
        if (valid(h, r) && (h->heap[r].prio < h->heap[l].prio)) {
            result = r;
        }
    }

    return result;
}

static void move_down(MinHeap *h, int i)
{
    int done = 0;

    assert(valid(h, i));

    do {
        const int dst = min_child(h, i);
        if (valid(h, dst) && (h->heap[dst].prio < h->heap[i].prio)) {
            swap(h, i, dst);
            i = dst;
        } else {
            done = 1;
        }
    } while (!done);
}

int minheap_min(const MinHeap *h)
{
    assert( !minheap_is_empty(h) );

    return h->heap[0].key;
}

int minheap_delete_min(MinHeap *h)
{
    int result;

    assert( !minheap_is_empty(h) );

    result = minheap_min(h);
    swap(h, 0, h->n-1);
    assert( h->heap[h->n - 1].key == result );
    h->pos[result] = -1;
    h->n--;
    if (!minheap_is_empty(h)) {
        move_down(h, 0);
    }
    return result;
}

void minheap_change_prio(MinHeap *h, int key, double newprio)
{
    int j;
    double oldprio;

    assert(h != NULL);
    assert(key >= 0 && key < h->size);
    j = h->pos[key];
    assert( valid(h, j) );
    oldprio = h->heap[j].prio;
    h->heap[j].prio = newprio;
    if (newprio > oldprio) {
        move_down(h, j);
    } else {
        move_up(h, j);
    }
}

/* funzioni per creare/modificare/cancellare grafi */
Graph *graph_create( int n, Graph_type t )
{
    int i;
    Graph *g = (Graph*)malloc(sizeof(*g));
    assert(g != NULL);
    assert(n > 0);

    g->n = n;
    g->m = 0;
    g->t = t;
    g->edges = (Edge**)malloc(n * sizeof(Edge*));
    assert(g->edges != NULL);
    g->in_deg = (int*)malloc(n * sizeof(*(g->in_deg)));
    assert(g->in_deg != NULL);
    g->out_deg = (int*)malloc(n * sizeof(*(g->out_deg)));
    assert(g->out_deg != NULL);
    for (i=0; i<n; i++) {
        g->edges[i] = NULL;
        g->in_deg[i] = g->out_deg[i] = 0;
    }
    return g;
}

void graph_destroy(Graph *g)
{
    int i;

    assert(g != NULL);

    for (i=0; i<g->n; i++) {
        Edge *edge = g->edges[i];
        while (edge != NULL) {
            Edge *next = edge->next;
            free(edge);
            edge = next;
        }
        g->edges[i] = NULL; /* e' superfluo */
    }
    free(g->edges);
    free(g->in_deg);
    free(g->out_deg);
    g->n = 0;
    g->edges = NULL;
    free(g);
}

static Edge *new_edge(int src, int dst, double weight, Edge *next)
{
    Edge *edge = (Edge*)malloc(sizeof(Edge));
    assert(edge != NULL);

    edge->src = src;
    edge->dst = dst;
    edge->weight = weight;
    edge->next = next;
    return edge;
}

static int graph_adj_insert(Graph *g, int src, int dst, double weight)
{
    g->edges[src] = new_edge(src, dst, weight, g->edges[src]);
    g->in_deg[dst]++;
    g->out_deg[src]++;
    return 0;
}

int graph_n_nodes(const Graph *g)
{
    assert(g != NULL);

    return g->n;
}

Graph_type graph_type(const Graph *g)
{
    return g->t;
}

void graph_add_edge(Graph *g, int src, int dst, double weight)
{
    int status = 0;

    assert(g != NULL);

    assert((src >= 0) && (src < graph_n_nodes(g)));
    assert((dst >= 0) && (dst < graph_n_nodes(g)));

    status = graph_adj_insert(g, src, dst, weight);
    if (graph_type(g) == GRAPH_UNDIRECTED) {
        status |= graph_adj_insert(g, dst, src, weight);
    }
    if (status == 0)
        g->m++;
    else
        fprintf(stderr, "Ho ignorato l'arco duplicato (%d,%d)\n", src, dst);
}

Edge *graph_adj(const Graph *g, int v)
{
    assert(g != NULL);
    assert((v >= 0) && (v < graph_n_nodes(g)));

    return g->edges[v];
}

/* Legge il file per una seconda volta e restituisce il numero di righe. 

   Dato che la struttura del file è sempre la stessa, si sa già che alla 
   quarta riga troveremo il numero di righe. Per non utilizzare più 
   variabili per la lettura (spreco di memoria) sovrascrivo la stessa. */
int get_lines( FILE *f )
{
    int lines, i;

    for (i = 0; i < LINES_IN_FILE; i++) {
        fscanf(f, "%d", &lines);
    }

    return lines;
}

/* Stampa in modo ricorsivo il percorso dalla sorgene (0, 0) 
   alla destinazione (n - 1, m - 1).

   La stampa del percorso finisce con la coppia (-1, -1). 
   Per avere questa ultima stampa si utilizza il fatto 
   che sappiamo che la destinazione coincide con il nodo n - 1. 
   
   Per stampare nel formato richiesto caratterizzato da 2 
   coordinate si utilizzano la divisione e il modulo per 
   convertire la numerazione dei vari nodi. */
void print_percoro( const Graph *g, const int *p, const int src, int dst, const int n) 
{
    assert(p != NULL);

    if (p[dst] != NODE_UNDEF) {
        print_percoro(g, p, src, p[dst], n);
    }

    printf("%d %d\n", dst / n, dst % n);
    
    if (g-> n - 1 == dst) {
        printf("-1 -1\n");
    }
} 

/* Stampa il costo del cammino minimo.
   Si fa utilizzo di un cast per convertire la distanza 
   da double a intero in modo da rispettare le specifiche del progetto.  */
void print_costo ( const double *d, int dst) 
{
    int costo = (int) d[dst];

    printf("%d", costo);
}

/* Legge il file è immagazzina tutte le informazioni al suo interno. 

   Utilizza questa informazioni per creare un grafo orientato con 
   (righe * colonne) nodi e costruisce una matrice in cui 
   salva tutte le altezze delle varie celle.

   In modo da semplificare il calcolo del cammino minimo nel vari 
   archi che vengono creati per ogni posizione e in ogni direzione disponibile 
   viene già calcolato il dislivello tra le varia celle e viene insierito 
   come peso dell'arco. Per semplificare ulteriormente i calcoli oltre al
   dislivello viene inserito anche il peso di ogni cella in modo da rispettare
   la formula. In ogni arco viene aggiunto il peso di una casella, tranne
   per la prima, in cui viene aggiunto il peso di 2 caselle. 
   Questo permette di avere il conto giusto delle caselle sempre, dato che la 
   posizione di partenza sarà sempre la casella (0, 0).
   
   Inoltre all'interno del grafo i nodi non hanno la doppia coordinata
   ma vengono identificati da numero progressivo. */
Graph *from_matrix_to_graph(FILE *f)
{
    
    Graph *g;
    int C_cell, C_height, n, m, nodo = 0, base;
    int **matrix;
    int i, j;
    double disl = 0;

    fscanf(f, "%d", &C_cell);
    fscanf(f, "%d", &C_height);
    fscanf(f, "%d", &n);
    fscanf(f, "%d", &m);

    g = graph_create(n * m, GRAPH_DIRECTED);
    
    matrix = (int **)malloc(n * sizeof(int *));

    for (i = 0; i < n; i++) {
        matrix[i] = (int *)malloc(m * sizeof(int));
    }

    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            fscanf(f, "%d", &matrix[i][j]);
        }
    }

    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            if (j == 0 && i == 0) {
                base = C_cell;
            } else {
                base = 0;
            }
            if (j + 1 < m) {
                disl = ((matrix[i][j] - matrix[i][j + 1]) * (matrix[i][j] - matrix[i][j + 1])) * C_height + C_cell + base;
                graph_add_edge(g, nodo, nodo + 1, disl);
            }
            if (j - 1 >= 0) {
                disl = ((matrix[i][j] - matrix[i][j - 1]) * (matrix[i][j] - matrix[i][j - 1])) * C_height + C_cell + base;
                graph_add_edge(g, nodo, nodo - 1, disl);
            }
            if (i + 1 < n) {
                disl = ((matrix[i][j] - matrix[i + 1][j]) * (matrix[i][j] - matrix[i + 1][j])) * C_height + C_cell + base;
                graph_add_edge(g, nodo, nodo + m, disl);
            }
            if (i - 1 >= 0) {
                disl = ((matrix[i][j] - matrix[i - 1][j]) * (matrix[i][j] - matrix[i - 1][j])) * C_height + C_cell + base;
                graph_add_edge(g, nodo, nodo - m, disl);
            }
            nodo++;
        }
    }

    for (i = 0; i < n; i++) {
        free(matrix[i]);
    }

    free(matrix);

    return g;
}

/* Implementazione dell'algoritmo di Dijkstra studiato a lezione.
   Fa utilizzo del concetto di grafo e di minheap che utilizza per la creazione
   e gestione di una coda a proprità in cui vengono inseriti i vari nodi del grafo. */
void dijkstra( const Graph *g, double *d, int *p) 
{
    int i, min, s = 0;
    MinHeap *Queque;
    Edge *edge; 

    Queque = minheap_create(g->n);

    for (i = 0; i < g->n; i++) {
        d[i] = HUGE_VAL;
        p[i] = NODE_UNDEF;
        minheap_insert(Queque, i, d[i]);
    }
    d[s] = 0;

    minheap_change_prio(Queque, s, d[s]);

    while (!minheap_is_empty(Queque)) { 
        min = minheap_delete_min(Queque);

        edge = graph_adj(g, min);
        while (edge != NULL) {
            if (d[edge->dst] > d[edge->src] + edge->weight) {
                d[edge->dst] = d[edge->src] + edge->weight;
                p[edge->dst] = edge->src;
                minheap_change_prio(Queque, edge->dst, d[edge->dst]);
            }
            edge = edge->next;
        }
    }

    minheap_destroy(Queque);
}

int main( int argc, char *argv[] )
{
    Graph *g;
    double *d;
    int *p, lines;

    FILE *filein = stdin;
    
    if ( argc != 2 ) {
        fprintf(stderr, "Inserire il file\n");
    }

    if (strcmp(argv[1], "-") != 0) {
        filein = fopen(argv[1], "r");
        if (filein == NULL) {
            fprintf(stderr, "Can not open %s\n", argv[1]);
            return EXIT_FAILURE;
        }
    }

    g = from_matrix_to_graph(filein);  

    if (filein != stdin) fclose(filein);

    filein = fopen(argv[1], "r");
    if (filein == NULL) {
        fprintf(stderr, "Can not open %s for the second time\n", argv[1]);
        return EXIT_FAILURE;
    }

    lines = get_lines(filein);

    if (filein != stdin) fclose(filein);

    assert(g != NULL);

    d = (double*)malloc(g->n * sizeof(*d)); assert(d != NULL);
    p = (int*)malloc(g->n * sizeof(*p)); assert(p != NULL);

    dijkstra(g, d, p);
    print_percoro(g, p, 0, g-> n - 1, lines);
    print_costo(d, g-> n - 1);
    graph_destroy(g);

    free(p);
    free(d);

    return EXIT_SUCCESS;
}