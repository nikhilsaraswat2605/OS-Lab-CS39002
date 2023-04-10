#ifndef CONSUMER_HPP
#define CONSUMER_HPP

#include "main.hpp"

void dijsktra(int index, graph *s, int iter, vector<int> &src_nodes, int &op_count, clock_t start, double &total_time_taken);
void update_non_src_nodes(vector<int> non_src, graph *s, vector<int> &dist, vector<int> &parent, int &op_count);
void multi_src_dijkstra_optimize(vector<int> &src, vector<int> &dist, graph *s, vector<int> &parent, int &op_count);
void optimized_consumer_process(int id, int shmid);
void consumer_process(int id, int shmid);

#endif