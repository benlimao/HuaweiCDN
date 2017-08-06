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
    //(char *)"17\n\n0 8 0 20\n21 8 0 20\n9 11 1 13\n21 22 2 20\n23 22 2 8\n1 3 3 11\n24 3 3 17\n27 3 3 26\n24 3 3 10\n18 17 4 11\n1 19 5 26\n1 16 6 15\n15 13 7 13\n4 5 8 18\n2 25 9 15\n0 7 10 10\n23 24 11 23";

    write_result(topo_file, filename);
}
