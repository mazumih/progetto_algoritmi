#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef struct Edge {
    int src;            /* nodo sorgente        */
    int dst;            /* nodo destinazione    */
    double weight;      /* peso dell'arco       */
    struct Edge *next;
} Edge;

typedef enum { GRAPH_UNDIRECTED, GRAPH_DIRECTED } Graph_type;

typedef struct {
    int n;              /* numero di nodi               */
    int m;              /* numero di archi              */
    Graph_type t;       /* tipo di grafo (orientato/non orientato) */
    Edge **edges;       /* array di liste di adiacenza  */
    int *in_deg;        /* grado entrante dei nodi      */
    int *out_deg;       /* grado uscente dei nodi       */
} Graph;

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

int graph_n_edges(const Graph *g)
{
    assert(g != NULL);

    return g->m;
}

Edge *graph_adj(const Graph *g, int v)
{
    assert(g != NULL);
    assert((v >= 0) && (v < graph_n_nodes(g)));

    return g->edges[v];
}

void graph_write_to_file( FILE *f, const Graph* g )
{
    int v;
    int n, m, t;

    assert(g != NULL);
    assert(f != NULL);

    n = graph_n_nodes(g);
    m = graph_n_edges(g);
    t = graph_type(g);

    fprintf(f, "%d %d %d\n", n, m, t);
    for (v=0; v<n; v++) {
        const Edge *e;
        for (e = graph_adj(g, v); e != NULL; e = e->next) {
            assert(e->src == v);
            /* Se il grafo è non orientato, dobbiamo ricordarci che
               gli archi compaiono due volte nelle liste di
               adiacenza. Nel file pero' dobbiamo riportare ogni arco
               una sola volta, dato che sarà la procedura di lettura a
               creare le liste di adiacenza in modo corretto. Quindi,
               ogni coppia di archi (u,v), (v,u) deve comparire una
               sola volta nel file. Per comodità, salviamo nel file la
               versione di ciascun arco in cui il nodo sorgente è
               minore del nodo destinazione. */
            if ((graph_type(g) == GRAPH_DIRECTED) || (e->src < e->dst)) {
                fprintf(f, "%d %d %f\n", e->src, e->dst, e->weight);
            }
        }
    }
}

int main (int argc, char *argv[]) 
{
    Graph *g;
    int C_cell, C_height, n, m, nodo = 0, base;
    int **matrix;
    int i, j;
    long int disl = 0;

    FILE *filein = stdin, *fileout = stdout;

    filein = fopen(argv[1], "r");
    if ( filein == NULL) {
        return EXIT_FAILURE;
    }

    fscanf(filein, "%d", &C_cell);
    fscanf(filein, "%d", &C_height);
    /* numero delle righe */
    fscanf(filein, "%d", &n);
    /* numero delle colonne */
    fscanf(filein, "%d", &m);

    g = graph_create(n * m, GRAPH_DIRECTED);
    
    /* malloc della matrice */
    matrix = (int **)malloc(n * sizeof(int *));

    for (i = 0; i < n; i++) {
        matrix[i] = (int *)malloc(m * sizeof(int));
    }

    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            fscanf(filein, "%d", &matrix[i][j]);
            /* in qualche modo inserisco tutti i dislivelli */
        }
    }

    fclose(filein);

    /* come numerare i nodi? */
    /* la matrice DEVE contenere i dislivelli 
        NON può contenere anche gli indici */

    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            if (j == 0 && i == 0) {
                base = C_cell;
            } else {
                base = 0;
            }
            if (j + 1 < m) {
                disl = ((matrix[i][j] - matrix[i][j + 1]) * (matrix[i][j] - matrix[i][j + 1])) * C_height + C_cell + base;
                /* src e dst da definire */
                graph_add_edge(g, nodo, nodo + 1, disl);
            }
            if (j - 1 >= 0) {
                disl = ((matrix[i][j] - matrix[i][j - 1]) * (matrix[i][j] - matrix[i][j - 1])) * C_height + C_cell + base;
                /* src e dst da definire */
                graph_add_edge(g, nodo, nodo - 1, disl);
            }
            if (i + 1 < n) {
                disl = ((matrix[i][j] - matrix[i + 1][j]) * (matrix[i][j] - matrix[i + 1][j])) * C_height + C_cell + base;
                /* src e dst da definire */
                graph_add_edge(g, nodo, nodo + m, disl);
            }
            if (i - 1 >= 0) {
                disl = ((matrix[i][j] - matrix[i - 1][j]) * (matrix[i][j] - matrix[i - 1][j])) * C_height + C_cell + base;
                /* src e dst da definire */
                graph_add_edge(g, nodo, nodo - m, disl);
            }
            nodo++;
        }
    }

    fileout = fopen(argv[2], "w");
    if (fileout == NULL) {
        return EXIT_FAILURE;
    }

    graph_write_to_file(fileout, g);

    fclose(fileout);

    graph_destroy(g);

    return EXIT_SUCCESS;
    /* bisogna introdurre una funzione che stampa le coordinate facendo uso del modulo e della divisione del NODO */

    /* AGGIORNAMENTO: compila
     adesso bisogna scoprire cosa produce */
}



