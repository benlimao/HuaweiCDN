#include "deploy.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <algorithm>

void deploy_server(char * topo[MAX_EDGE_NUM], int line_num,char * filename)
{
    Adjlist adjlist(topo);
    adjlist.create_adjlist();

    SolvedSet tmp_solved_set;
    int min_cost = INF;
    int tmp_cost;

    //srand((unsigned)time(NULL));
    if(adjlist.node_num < 200)
    {
        for(int index = 0; index < 20; index++)
        {
           tmp_cost = adjlist.solve_case();
           if(tmp_cost < min_cost)
           {
               min_cost = tmp_cost;
               tmp_solved_set = adjlist.solved_set;
           }
        }
    }
    if(adjlist.node_num >= 200 && adjlist.node_num <= 400)
    {
        for(int index = 0; index < 10; index++)
        {
           tmp_cost = adjlist.solve_case();
           if(tmp_cost < min_cost)
           {
               min_cost = tmp_cost;
               tmp_solved_set = adjlist.solved_set;
           }
        }
    }
    if(adjlist.node_num > 400)
    {
        tmp_cost = adjlist.solve_case();
        tmp_solved_set = adjlist.solved_set;
    }

    // 需要输出的内容
    adjlist.cal_total_cost(tmp_solved_set);
    char * topo_file = adjlist.get_solved_buffer(tmp_solved_set);

    write_result(topo_file, filename);
}
