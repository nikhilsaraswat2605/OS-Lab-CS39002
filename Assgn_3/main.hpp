#ifndef MAIN_HPP
#define MAIN_HPP
#include <bits/stdc++.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sstream>

using namespace std;

const int MAX_NODES = 1e5;
const int MAX_EDGES = 2e5;

struct node
{
    int degree;
    int head;
    node()
    {
        degree = 0;
        head = -1;
    }
};

struct edgeData
{
    int to, cost, flow;
    int nxt; /* nxt: is the index of the next edge */
};

struct graph
{
    int node_count, edge_count;
    node nodes[MAX_NODES];
    edgeData edges[MAX_EDGES];
    graph(int n)
    {
        node_count = n;
        edge_count = 0;
    }
    void addEdge(int from, int to, int cost)
    {
        edges[edge_count].to = to;
        edges[edge_count].cost = cost;
        edges[edge_count].nxt = nodes[from].head;
        nodes[from].head = edge_count;
        nodes[from].degree++;
        edge_count++;
    }
};

#endif