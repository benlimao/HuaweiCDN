#include "adjlist.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <queue>

Adjlist::Adjlist(char * topo[MAX_EDGE_NUM])
{
    adjlist_buffer = topo;
    node_num = 0;
    edge_num = 0;
    usr_num = 0;
    server_cost = 0;
    vertexnode = NULL;
    edgenode = NULL;
    usrnode = NULL;

    solved_set.path_num = 0;
    start_time = clock();

    cnt = 0;

    for(int index = 0; index < 5; index++)
    {
        solved_array.cost_set[index] = INF;
    }

    //
    edge_count = 0;

    solved_set.server_num = 0;
    solved_set.path_num = 0;
    solved_set.path_set.clear();
    solved_set.server_set.clear();

    total_flow = 0;
}

Adjlist::~Adjlist()
{
    if(NULL != vertexnode)
    {
        delete []vertexnode;
    }
    if(NULL != edgenode)
    {
        delete []edgenode;
    }
    if(NULL != usrnode)
    {
        delete []usrnode;
    }
    if(NULL != tmp_node_set.vertexnode)
    {
        delete []tmp_node_set.vertexnode;
    }
    if(NULL != tmp_node_set.edgenode)
    {
        delete []tmp_node_set.edgenode;
    }
    if(NULL != tmp_node_set.usrnode)
    {
        delete []tmp_node_set.usrnode;
    }
}

void Adjlist::create_adjlist()
{
   get_basic_info();
   get_link_info();
   get_usr_info();
}

void Adjlist::get_basic_info()
{
    //临时buffer
    char *tmp;

    //获取网络节点数、链路数和消费节点数
    int basic_info[BASIC_INFO_NUM];
    tmp = adjlist_buffer[0];
    for(int index = 0; index < BASIC_INFO_NUM; index++)
    {
        basic_info[index] = atoi(tmp);
        while(' ' != tmp[0])
        {
            if('\n' == tmp[0])
            {
                break;
            }
            tmp+=1;
        }
        tmp+=1;
    }
    node_num = basic_info[0];
    edge_num = basic_info[1];
    usr_num = basic_info[2];

    //获取服务器设置费用
    server_cost = atoi(adjlist_buffer[2]);

    vertexnode = new VertexNode[node_num];
    edgenode = new EdgeNode[edge_num*2];
    usrnode = new UsrNode[usr_num];

    tmp_node_set.vertexnode = new VertexNode[node_num];
    tmp_node_set.edgenode = new EdgeNode[edge_num*2];
    tmp_node_set.usrnode = new UsrNode[usr_num];

}

void Adjlist::get_link_info()
{
    //临时buffer
    char *tmp;

    //1.邻接表初始化
    for(int index = 0; index < node_num; index++)
    {
        vertexnode[index].vertex = index;
        vertexnode[index].flag = false;
        vertexnode[index].firstedge = NULL;
        vertexnode[index].edge_num = 0;
        vertexnode[index].usr = -1;
    }

    //2.处理网络连接信息
    int link_info[LINK_INFO_NUM];
    for(int index = 0; index < edge_num; index++)
    {
        //2.1 获取网络连接信息
        tmp = adjlist_buffer[index + 4];
        for(int index = 0; index < LINK_INFO_NUM; index++)
        {
            link_info[index] = atoi(tmp);
            while(' ' != tmp[0])
            {
                if('\n' == tmp[0])
                {
                    break;
                }
                tmp+=1;
            }
            tmp+=1;
        }

        //2.2 将网络连接关系写入邻接表
        EdgeNode *tmpnode;
        edgenode[index*2].vertex = link_info[1];
        edgenode[index*2].total_bandwidth = link_info[2];
        edgenode[index*2].cost = link_info[3];
        edgenode[index*2].used_bandwidth = 0;
        edgenode[index*2].empty_bandwidth = link_info[2];
        edgenode[index*2].nextedge = NULL;
        edgenode[index*2].flag = false;
        edgenode[index*2].former = link_info[0];
        if(NULL == vertexnode[link_info[0]].firstedge)
        {
            vertexnode[link_info[0]].firstedge = &edgenode[index*2];
        }
        else
        {
            tmpnode = vertexnode[link_info[0]].firstedge;
            while(NULL != tmpnode->nextedge)
            {
                tmpnode = tmpnode->nextedge;
            }
            tmpnode->nextedge = &edgenode[index*2];
        }

        edgenode[index*2+1].vertex = link_info[0];
        edgenode[index*2+1].total_bandwidth = link_info[2];
        edgenode[index*2+1].cost = link_info[3];
        edgenode[index*2+1].used_bandwidth = 0;
        edgenode[index*2+1].empty_bandwidth = link_info[2];
        edgenode[index*2+1].nextedge = NULL;
        edgenode[index*2+1].flag = false;
        edgenode[index*2+1].former = link_info[1];
        if(NULL == vertexnode[link_info[1]].firstedge)
        {
            vertexnode[link_info[1]].firstedge = &edgenode[index*2+1];
        }
        else
        {
            tmpnode = vertexnode[link_info[1]].firstedge;
            while(NULL != tmpnode->nextedge)
            {
                tmpnode = tmpnode->nextedge;
            }
            tmpnode->nextedge = &edgenode[index*2+1];
        }
    }
    //
    EdgeNode *tmpedge;
    for(int index = 0; index < node_num; index++)
    {
        tmpedge = vertexnode[index].firstedge;
        while(NULL != tmpedge)
        {
            vertexnode[index].edge_num++;
            tmpedge = tmpedge->nextedge;
        }
    }
}

void Adjlist::get_usr_info()
{
    //临时buffer
    char *tmp;
    //将消费节点信息写入邻接表
    int usr_info[USR_INFO_NUM];
    for(int index = 0; index < usr_num; index++)
    {
        tmp = adjlist_buffer[index + 4 + edge_num + 1];
        for(int index = 0; index < USR_INFO_NUM; index++)
        {
            usr_info[index] = atoi(tmp);
            while(' ' != tmp[0])
            {
                if('\n' == tmp[0])
                {
                    break;
                }
                tmp+=1;
            }
            tmp+=1;
        }
        usrnode[index].usr = usr_info[0];
        usrnode[index].req_bandwidth = usr_info[2];
        usrnode[index].owned_bandwidth = 0;
        usrnode[index].total_bandwidth = usr_info[2];
        total_flow+=usr_info[2];
        usrnode[index].flag = false;
        usrnode[index].linkednode = &vertexnode[usr_info[1]];
        usrnode[index].isolated = false;
        vertexnode[usr_info[1]].usr = index;
    }
}

void Adjlist::show_adjlist()
{
    EdgeNode *tmpedge;
    VertexNode *tmpvertex;
    for(int index = 0; index < node_num; index++)
    {
        printf("%d", vertexnode[index].vertex);
        if(NULL != vertexnode[index].firstedge)
        {
            tmpedge = vertexnode[index].firstedge;
            printf("->(%d, %d)%d", tmpedge->total_bandwidth, tmpedge->cost, tmpedge->vertex);
            while(NULL != tmpedge->nextedge)
            {
                tmpedge = tmpedge->nextedge;
                printf("->(%d, %d)%d", tmpedge->total_bandwidth, tmpedge->cost, tmpedge->vertex);
            }
        }
        tmpvertex = &vertexnode[index];
        for(int index = 0; index < usr_num; index++)
        {
            if(usrnode[index].linkednode == tmpvertex)
            {
                printf("  %d(%d)", usrnode[index].usr, usrnode[index].req_bandwidth);
            }
        }
        printf(" %d", vertexnode[index].edge_num);
        printf("\n");
    }
}

void Adjlist::init_solution()
{
    get_isolated_usr();
    clear_solved_set();

    //
    search_usr_to_usr();   //找出离每个节点最近的消费节点，并将它们之间的路径存入min_usr_set中
    vector<PathInfo>::iterator iter;
    PathInfo tmp_pathinfo;
    Path tmp_path;
    for(iter = min_usr_set.begin(); iter != min_usr_set.end(); iter++)
    {
        if(true == is_effective_path((*iter).path))
        {
            tmp_pathinfo = *iter;
            devide_path(tmp_pathinfo.path, *tmp_pathinfo.path.begin());
            set_path(path_one);
            set_path(path_two);
        }
    }
    for(int index = 0; index < usr_num; index++)
    {
        if(usrnode[index].flag == false)
        {
            tmp_path.clear();
            tmp_path.push_back(usrnode[index].linkednode->vertex);
            set_path(tmp_path);
        }
    }
    min_solved_set = solved_set;
}

char* Adjlist::get_solved_buffer(SolvedSet solved_set)
{
    char topo_file[MAX_EDGE_NUM];
    char route[MAX_EDGE_NUM];
    char tmp[MAX_EDGE_NUM];
    vector<int>::iterator iter;
    solved_set.path_num = solved_set.path_set.size();
    //
    snprintf(topo_file, sizeof(topo_file), "%d\n\n", solved_set.path_num);
    //
    for(int index = 0; index < solved_set.path_num; index++)
    {
        snprintf(route, sizeof(route), "");
        for(iter = solved_set.path_set[index].begin(); iter != solved_set.path_set[index].end(); iter++)
        {
            snprintf(tmp, sizeof(tmp), "%d ", *iter);
            strncat(route, tmp, sizeof(route));
        }
        snprintf(tmp, sizeof(tmp), "\n");
        strncat(route, tmp, MAX_EDGE_NUM);
        strncat(topo_file, route, MAX_EDGE_NUM);
    }
    return topo_file;
}

void Adjlist::set_server_to_usr()
{
    //
    clear_solved_set();
    solved_set.path_num = usr_num;
    solved_set.server_num = usr_num;

    Path tmp_path;
    for(int index = 0; index < usr_num; index++)
    {
        tmp_path.clear();
        tmp_path.push_back(usrnode[index].linkednode->vertex);
        tmp_path.push_back(index);
        tmp_path.push_back(usrnode[index].req_bandwidth);
        usrnode[index].linkednode->flag = 1;
        solved_set.path_set.push_back(tmp_path);
        solved_set.server_set.push_back(usrnode[index].linkednode->vertex);
    }
    min_solved_set = solved_set;
}

bool Adjlist::is_usr_isolated(int usr)
{
    //
     EdgeNode *tmpedge;
     UsrNode usrnode = this->usrnode[usr];
    //1.
    tmpedge = usrnode.linkednode->firstedge;
    int total_bandwidth = 0;
    while(NULL != tmpedge->nextedge)
    {
        total_bandwidth+=reverse_edge(tmpedge)->empty_bandwidth;
        tmpedge = tmpedge->nextedge;
    }
    total_bandwidth+=reverse_edge(tmpedge)->empty_bandwidth;
    if(total_bandwidth < usrnode.req_bandwidth)
    {
        this->usrnode[usr].isolated = true;
        //printf("usr %d is type1 isolated!\n", usrnode.usr);
        return true;
    }
    //2.
    EdgeNode *tmp_edge;
    EdgeNode *revedge;
    int total_cost = 0;
    int min_cost = INF;
    int old_bandwidth = 0;
    int tmp_bandwidth = 0;
    total_bandwidth = 0;
    //
    for(int index = 0; index < usrnode.linkednode->edge_num; index++)
    {
        //
        min_cost = INF;
        tmpedge = usrnode.linkednode->firstedge;
        while(NULL != tmpedge)
        {
            revedge = reverse_edge(tmpedge);
            if(min_cost > revedge->cost && false == revedge->flag)
            {
                min_cost = revedge->cost;
                tmp_bandwidth = revedge->empty_bandwidth;
                tmp_edge = revedge;
            }
            tmpedge = tmpedge->nextedge;
        }
        //
        tmp_edge->flag = true;
        old_bandwidth = total_bandwidth;
        total_bandwidth+=tmp_bandwidth;
        if(total_bandwidth > usrnode.req_bandwidth)
        {
            tmp_bandwidth = usrnode.req_bandwidth - old_bandwidth;
            total_cost+=(min_cost*tmp_bandwidth);
            break;
        }
        total_cost+=(min_cost*tmp_bandwidth);
    }
    //
    tmpedge = usrnode.linkednode->firstedge;
    while(NULL != tmpedge)
    {
        reverse_edge(tmpedge)->flag = false;
        tmpedge = tmpedge->nextedge;
    }
    if(total_cost > server_cost)
    {
        this->usrnode[usr].isolated = true;
        //printf("usr %d is type2 isolated!\n", usrnode.usr);
        return true;
    }
   // printf("usr %d is not isolated!\n", usrnode.usr);
    return false;
}

void Adjlist::get_isolated_usr()
{
    isolated_usr.clear();
    Path path;

    for(int index = 0; index < usr_num; index++)
    {
        if(false == usrnode[index].flag)
        {
            if(true == is_usr_isolated(index))
            {
                //找到孤立节点，在该孤立节点设置服务器
                path.clear();
                path.push_back(usrnode[index].linkednode->vertex);
                set_path(path);
                isolated_usr.push_back(index);
            }
        }
    }
}

bool Adjlist::search_min_route(int start, int end)
{
    tmp_path_info.path.clear();
    //
    if(-1 == vertexnode[end].usr)
    {
        //printf("the end node is not a usr!\n");
        return false;
    }

    if(start == end)
    {
        tmp_path_info.cost = 0;
        tmp_path_info.path.push_back(start);
        tmp_path_info.start_node = start;
        tmp_path_info.end_node = end;
        tmp_path_info.start_usr = vertexnode[start].usr;
        tmp_path_info.end_usr = vertexnode[end].usr;
        return true;
    }

    //
    tmp_path_info.start_node = start;
    tmp_path_info.end_node = end;

    VertexNode ver_node = vertexnode[start];
    EdgeNode* edge_node = ver_node.firstedge;

    //初始化start
    bool known[node_num];
    int dist[node_num];
    int prever[node_num];
    int min_dist;
    int tmp_dist;
    int flag = -1;
    for(int index = 0; index < node_num; index++)
    {
        known[index] = false;
        dist[index] = INF;
        prever[index] = NOVERTEX;
    }

    //得到start本身信息
    known[start] = true;
    dist[start] = 0;
    while(NULL != edge_node)
    {
        if(false == edge_node->flag)    //若与start相邻节点的之间的链路带宽未全部被占用
        {
            dist[edge_node->vertex] = edge_node->cost;
        }
        edge_node = edge_node->nextedge;
    }

    //遍历node_num-1次，每次找出一个顶点的最短路径
    for(int index = 1; index < node_num; index++)
    {
        //在未获取最短路径的定点中，找到离目标顶点最近的顶点
        min_dist = INF;
        for(int index = 0; index < node_num; index++)
        {
            if(false == known[index] && dist[index] < min_dist)
            {
                min_dist = dist[index];
                flag = index;
            }
        }
        if(-1 == flag)
        {
            printf("there is no min route between %d and %d!\n", start, end);
            return false;
        }
        known[flag] = true;
        if(flag == end)
        {
            break;
        }

        //找到未知的离目标顶点最近的顶点flag最近的顶点
        ver_node = vertexnode[flag];
        edge_node = ver_node.firstedge;
        while(NULL != edge_node)
        {
            tmp_dist = edge_node->cost;
            tmp_dist = (tmp_dist == INF ? INF : (min_dist + tmp_dist));
            if(false == known[edge_node->vertex] && (tmp_dist < dist[edge_node->vertex]))
            {
                if(false == edge_node->flag)     //带宽未全部被占用
                {
                    dist[edge_node->vertex] = tmp_dist;
                    prever[edge_node->vertex] = flag;
                }
            }
            edge_node = edge_node->nextedge;
        }
    }

    if(flag != end)
    {
        printf("there is no min path!\n");
        return false;
    }
    //存入路径信息
        int tmp_prev;
        tmp_path_info.cost = dist[flag];
        tmp_prev = prever[end];
        int i = 0;
        tmp_path_info.path.push_back(end);
        while(tmp_prev != NOVERTEX)
         {
            tmp_path_info.path.push_back(tmp_prev);
            tmp_prev = prever[tmp_prev];
            i++;
        }
        tmp_path_info.path.push_back(start);
        reverse(tmp_path_info.path.begin(), tmp_path_info.path.end());
        /*
        printf("find the min path between %d and %d!\n", start, end);
        show_path(tmp_path_info.path);
        printf("\n");
        */
        return true;
}

bool Adjlist::search_nearest_usr(int vertex)
{
    tmp_path_info.path.clear();
    //
    tmp_path_info.start_node = vertex;

    VertexNode ver_node = vertexnode[vertex];
    EdgeNode* edge_node = ver_node.firstedge;

    //初始化
    bool known[node_num];
    int dist[node_num];
    int prever[node_num];
    int min_dist;
    int tmp_dist;
    int flag = -1;
    for(int index = 0; index < node_num; index++)
    {
        known[index] = false;
        dist[index] = INF;
        prever[index] = NOVERTEX;
    }

    //得到start本身信息
    known[vertex] = true;
    dist[vertex] = 0;
    while(NULL != edge_node)
    {
        if(false == edge_node->flag)    //若与start相邻节点的之间的链路带宽未全部被占用
        {
            dist[edge_node->vertex] = edge_node->cost;
        }
        edge_node = edge_node->nextedge;
    }

    //遍历node_num-1次，每次找出一个顶点的最短路径
    for(int index = 1; index < node_num; index++)
    {
        //在未获取最短路径的定点中，找到离目标顶点最近的顶点
        min_dist = INF;
        for(int index = 0; index < node_num; index++)
        {
            if(false == known[index] && dist[index] < min_dist)
            {
                min_dist = dist[index];
                flag = index;
            }
        }
        if(-1 == flag)
        {
            //printf("there is no min usr to %d!\n", vertex);
            return false;
        }
        known[flag] = true;
        if(vertexnode[flag].usr != -1)   //如果得知的节点为消费节点
        {
            break;
        }

        //找到未知的离目标顶点最近的顶点flag最近的顶点
        ver_node = vertexnode[flag];
        edge_node = ver_node.firstedge;
        while(NULL != edge_node)
        {
            tmp_dist = edge_node->cost;
            tmp_dist = (tmp_dist == INF ? INF : (min_dist + tmp_dist));
            if(false == known[edge_node->vertex] && (tmp_dist < dist[edge_node->vertex]))
            {
                if(false == edge_node->flag)     //带宽未全部被占用
                {
                    dist[edge_node->vertex] = tmp_dist;
                    prever[edge_node->vertex] = flag;
                }
            }
            edge_node = edge_node->nextedge;
        }
    }

    if(vertexnode[flag].usr == -1)
    {
        //printf("there is no min usr to vertex%d!\n", vertex);
        return false;
    }

    //存入路径信息
        tmp_path_info.end_node = flag;
        tmp_path_info.start_usr = vertexnode[vertex].usr;
        tmp_path_info.end_usr = vertexnode[flag].usr;
        int tmp_prev;
        tmp_path_info.cost = dist[flag];
        tmp_prev = prever[flag];
        int i = 0;
        tmp_path_info.path.push_back(flag);
        while(tmp_prev != NOVERTEX)
         {
            tmp_path_info.path.push_back(tmp_prev);
            tmp_prev = prever[tmp_prev];
            i++;
        }
        tmp_path_info.path.push_back(vertex);
        reverse(tmp_path_info.path.begin(), tmp_path_info.path.end());
        /*
        printf("find the min path between %d and %d!\n", start, end);
        show_path(tmp_path_info.path);
        printf("\n");
        */
        return true;
}

bool Adjlist::search_nearest_server(int vertex)
{
    if(vertexnode[vertex].flag == true)
    {
        return false;
    }

    tmp_path_info.path.clear();
    tmp_path_info.end_node = vertex;
    VertexNode ver_node = vertexnode[vertex];
    EdgeNode* edge_node = ver_node.firstedge;

    //初始化
    bool known[node_num];
    int dist[node_num];
    int prever[node_num];
    int min_dist;
    int tmp_dist;
    int flag = -1;
    for(int index = 0; index < node_num; index++)
    {
        known[index] = false;
        dist[index] = INF;
        prever[index] = NOVERTEX;
    }

    //得到start本身信息
    known[vertex] = true;
    dist[vertex] = 0;
    while(NULL != edge_node)
    {
        if(false == reverse_edge(edge_node)->flag)    //若与start相邻节点的之间的链路带宽未全部被占用
        {
            dist[edge_node->vertex] = edge_node->cost;
        }
        edge_node = edge_node->nextedge;
    }

    //遍历node_num-1次，每次找出一个顶点的最短路径
    for(int index = 1; index < node_num; index++)
    {
        //在未获取最短路径的定点中，找到离目标顶点最近的顶点
        min_dist = INF;
        for(int index = 0; index < node_num; index++)
        {
            if(false == known[index] && dist[index] < min_dist)
            {
                min_dist = dist[index];
                flag = index;
            }
        }
        if(-1 == flag)
        {
            //printf("there is no min usr to %d!\n", vertex);
            return false;
        }
        known[flag] = true;
        if(vertexnode[flag].flag == true)   //如果得知的节点为
        {
            break;
        }

        //找到未知的离目标顶点最近的顶点flag最近的顶点
        ver_node = vertexnode[flag];
        edge_node = ver_node.firstedge;
        while(NULL != edge_node)
        {
            tmp_dist = edge_node->cost;
            tmp_dist = (tmp_dist == INF ? INF : (min_dist + tmp_dist));
            if(false == known[edge_node->vertex] && (tmp_dist < dist[edge_node->vertex]))
            {
                if(false == reverse_edge(edge_node)->flag)     //带宽未全部被占用
                {
                    dist[edge_node->vertex] = tmp_dist;
                    prever[edge_node->vertex] = flag;
                }
            }
            edge_node = edge_node->nextedge;
        }
    }

    if(vertexnode[flag].flag == false)
    {
        //printf("there is no nearest server to vertex%d!\n", vertex);
        return false;
    }

    //存入路径信息
        tmp_path_info.start_node = flag;
        tmp_path_info.end_usr = vertexnode[vertex].usr;
        tmp_path_info.start_usr = vertexnode[flag].usr;
        int tmp_prev;
        tmp_path_info.cost = dist[flag];
        tmp_prev = prever[flag];
        int i = 0;
        tmp_path_info.path.push_back(flag);
        while(tmp_prev != NOVERTEX)
         {
            tmp_path_info.path.push_back(tmp_prev);
            tmp_prev = prever[tmp_prev];
            i++;
        }
        tmp_path_info.path.push_back(vertex);
        //reverse(tmp_path_info.path.begin(), tmp_path_info.path.end());
        return true;
}

int Adjlist::search_server_to_usr(int server, int usr)
{
    if(false == vertexnode[server].flag)
    {
        printf("the vertex is not a server!\n");
        return -1;
    }
    int cost = 0;
    while(usrnode[usr].req_bandwidth > 0)    //消费节点的需求带宽不为0，则不断寻找可以提供流量的最小路径
    {
        if(false == search_min_route(server, usrnode[usr].linkednode->vertex))     //服务器没有到消费节点的路径
        {
            printf("there is no path from server%d to usr%d!\n", server, usr);
            return -1;
        }
        set_path(tmp_path_info.path);    //设置路径
        cost+=(tmp_path_info.cost*get_max_bandwidth(tmp_path_info.path));   
    }
    return cost;
}

int Adjlist::search_flow_to_usr(int usr)
{
	//若消费节点的流量已满足要求，返回
    if(true == usrnode[usr].flag)
    {
        //printf("the flow to usr%d is enough!\n", usr);
        return -1;
    }

    record_current_state();
    //寻找满足消费节点流量的费用最小的路径集
    int cost = 0;
   // vector<int>::iterator iter;
   // vector<Path> tmp_path_set;
    Path tmp_path;
    int tmp_bandwidth = 0;  //现有带宽
    int tmp_cost = INF;    //现有费用
    while(usrnode[usr].flag == false)    //消费节点流量要求未满足，循环
    {
            if(true == search_nearest_server(usrnode[usr].linkednode->vertex))
            {
                    tmp_cost = tmp_path_info.cost;
                    tmp_bandwidth = get_max_bandwidth(tmp_path_info.path);    //路径上允许的最大带宽即为本次服务器流给消费节点的流量
                    if(tmp_bandwidth > usrnode[usr].req_bandwidth)   //若最大带宽超过了服务器的需求流量，则将路径上的流量减小到消费节点的需求流量
                    {
                        tmp_bandwidth = usrnode[usr].req_bandwidth;
                    }
                    tmp_path = tmp_path_info.path;   //现有路径更新
           }
            else
            {
                recover_former_state();
                //printf("usr%d is isolated!\n", usr);
                return -1;
            }
        cost+=(tmp_cost*tmp_bandwidth);   //费用更新
        if(cost >= server_cost)   //若费用大于单个服务器费用，则不设置任何路径
        {
            recover_former_state();
            return -1;
        }
        set_path(tmp_path);
       // tmp_path_set.push_back(tmp_path);
    }
    return cost;
}

void Adjlist::search_usr_to_usr()
{
    //
    PathInfo min_path;
    int start, end;
    int nearest_usr[usr_num];
    int min_cost;
    int tmp_cost;

    //分别找到每个消费节点最近的消费节点
    for(int i = 0; i < usr_num; i++)
    {
        if(true == usrnode[i].flag)    //如果消费节点已分配满足要求的带宽，则跳过
        {
            continue;
        }

        search_nearest_usr(usrnode[i].linkednode->vertex);
        min_path = tmp_path_info;
        min_usr_set.push_back(min_path);
    }
    /*
    vector<int>::iterator iter;
    int cnt = min_usr_set.size();
    for(int index = 0; index < cnt; index++)
    {
        printf("the nearest user of %d(%d) is %d(%d)\n", min_usr_set[index].start_usr, min_usr_set[index].start_node, min_usr_set[index].end_usr, min_usr_set[index].end_node);
        printf("the cost is %d\n", min_usr_set[index].cost);
        printf("the path is ");
        for(iter = min_usr_set[index].path.begin(); iter != min_usr_set[index].path.end(); iter++)
        {
            printf("%d->", *iter);
        }
        printf("\n\n");
    }
    */
}

void Adjlist::devide_path(Path path, int vertex)
{
    //将path从vertex一分为二
    Path path_one;
    Path path_two;
    vector<int>::iterator iter = path.begin();
    while(*iter != vertex)
    {
        path_one.push_back(*iter);
        iter++;
    }
    path_one.push_back(*iter);
    reverse(path_one.begin(), path_one.end());
    while(iter != path.end())
    {
        path_two.push_back(*iter);
        iter++;
    }
    //
    if(true == is_available_path(path_one))
    {
        //tmp_server_path.path_set.push_back(path_one);
        this->path_one = path_one;
    }
    if(true == is_available_path(path_two))
    {
        this->path_two = path_two;
        //tmp_server_path.path_set.push_back(path_two);
    }
    //
}

bool Adjlist::is_available_path(Path path)
{
    //判断路径最后一个节点是否为消费节点
    int flag = -1;
    Path reverse_path = path;
    reverse(reverse_path.begin(), reverse_path.end());
    vector<int>::iterator iter = reverse_path.begin();
    for(int index = 0; index < usr_num; index++)
    {
        if(usrnode[index].linkednode->vertex == *iter)
        {
            flag = index;
            break;
        }
    }
    if(-1 == flag)     //若不是消费节点，则退出
    {
        printf("not a usr path!");
        return false;
    }
    //判断该路径是否有效（路径费用超过单个服务器费用则无效）
    iter = path.begin();
    VertexNode tmp_vertex;
    EdgeNode *tmp_edge;
    int path_cost = 0;
    int effective_bandwidth = INF; //路径内最小带宽即为该路径有效带宽
    while(*iter != usrnode[flag].linkednode->vertex)    //求路径内最小带宽及路径上的单位总费用
    {
        tmp_edge = vertexnode[*iter].firstedge;
        iter++;
        while(NULL != tmp_edge)
        {
            if(tmp_edge->vertex == *iter)
            {
                break;
            }
            tmp_edge = tmp_edge->nextedge;
        }
        path_cost+=tmp_edge->cost;
        if(effective_bandwidth > tmp_edge->empty_bandwidth)
        {
            effective_bandwidth = tmp_edge->empty_bandwidth;
        }
    }
    if(effective_bandwidth > usrnode[flag].req_bandwidth)   //若最小带宽仍大于消费节点的需求带宽
    {
    effective_bandwidth = usrnode[flag].req_bandwidth;
    }
    path_cost*=effective_bandwidth;
    if(path_cost >= server_cost)    //费用超过单个服务器，无效
    {
        printf("not an available path! the cost is %d\n", path_cost);
        return false;
    }
   // printf("an available path!\n");
   // show_path(path);
 //   printf("the cost is %d\n\n", path_cost);
    return true;
}

bool Adjlist::is_effective_path(Path path)
{
    //判断路径最后一个节点是否为消费节点
    int flag = -1;
    Path reverse_path = path;
    reverse(reverse_path.begin(), reverse_path.end());
    vector<int>::iterator iter = reverse_path.begin();
    for(int index = 0; index < usr_num; index++)
    {
        if(usrnode[index].linkednode->vertex == *iter)
        {
            flag = index;
            break;
        }
    }
    if(-1 == flag)     //若不是消费节点，则退出
    {
        //printf("not a usr path!");
        return false;
    }
    //判断该路径是否有效
    iter = path.begin();
    //VertexNode tmp_vertex;
    EdgeNode *tmp_edge;
    int path_cost = 0;
    int effective_bandwidth = INF; //路径内最小带宽即为该路径有效带宽
    while(*iter != usrnode[flag].linkednode->vertex)    //求路径内最小带宽及路径上的单位总费用
    {
        tmp_edge = vertexnode[*iter].firstedge;
        iter++;
        while(NULL != tmp_edge)
        {
            if(tmp_edge->vertex == *iter)
            {
                break;
            }
            tmp_edge = tmp_edge->nextedge;
        }
        path_cost+=tmp_edge->cost;
        if(effective_bandwidth > tmp_edge->empty_bandwidth)
        {
            effective_bandwidth = tmp_edge->empty_bandwidth;
        }
    }
    if(effective_bandwidth > usrnode[flag].req_bandwidth)   //若最小带宽仍大于消费节点的需求带宽
    {
        effective_bandwidth = usrnode[flag].req_bandwidth;
        path_cost*=effective_bandwidth;
        if(path_cost >= server_cost)    //费用超过单个服务器，无效
        {
            //printf("not an effective path! the cost is %d\n", path_cost);
            return false;
        }
        /*
        printf("an effective path!\n");
        show_path(path);
        printf("the cost is %d\n\n", path_cost);
        */
        return true;
    }
    //printf("not an effective path!\n");
    //show_path(path);
    //printf("\n");
    return false;
}

int Adjlist::get_max_bandwidth(Path path)
{
    //
    if(true == path.empty())
    {
        //printf("empty paht!\n");
        return -1;
    }
    if(1 == path.size())
    {
        return usrnode[vertexnode[*path.begin()].usr].total_bandwidth;
    }
    int max_bandwidth = INF;
    int path_cost = 0;
    vector<int>::iterator iter = path.begin();
    VertexNode tmp_vertex;
    EdgeNode *tmp_edge;
    
    //
    while(iter != path.end())
    {
        tmp_vertex = vertexnode[*iter];
        tmp_edge = tmp_vertex.firstedge;
        iter++;
        if(iter == path.end())
        {
            break;
        }
        while(tmp_edge->vertex != *iter)
        {
            tmp_edge = tmp_edge->nextedge;
            if(tmp_edge == NULL)
            {
                return -1;
            }
        }

        if(max_bandwidth > tmp_edge->empty_bandwidth)
        {
            max_bandwidth = tmp_edge->empty_bandwidth;
        }
        path_cost+= tmp_edge->cost;
    }
    path_cost*=max_bandwidth;
    //
    //printf("the max bandwidth of the path is %d, the cost is %d\n", max_bandwidth, path_cost);

    return max_bandwidth;
}

void Adjlist::show_path(Path path)
{
    vector<int>::iterator iter;
    printf("the path is ");
    for(iter = path.begin(); iter != path.end(); iter++)
    {
        printf("%d->", *iter);
    }
    printf("\n");
}

EdgeNode *Adjlist::reverse_edge(EdgeNode *edgenode)
{
    EdgeNode *reversed_edgenode = vertexnode[edgenode->vertex].firstedge;
    while(NULL != reversed_edgenode)
    {
        if(reversed_edgenode->vertex == edgenode->former)
        {
            break;
        }
        reversed_edgenode = reversed_edgenode->nextedge;
    }
    //printf("the edgenode is %d to %d,the reversed edgenode is %d to %d\n", edgenode->former, edgenode->vertex, reversed_edgenode->former, reversed_edgenode->vertex);
    return reversed_edgenode;
}

void Adjlist::record_current_state()
{
    //
    for(int index = 0; index < node_num; index++)
    {
        tmp_node_set.vertexnode[index] = vertexnode[index];
    }
    for(int index = 0; index < usr_num; index++)
    {
        tmp_node_set.usrnode[index] = usrnode[index];
    }
    for(int index = 0; index < edge_num; index++)
    {
        tmp_node_set.edgenode[index] = edgenode[index];
    }

    //
    tmp_solved_set = solved_set;
}

void Adjlist::recover_former_state()
{
    for(int index = 0; index < node_num; index++)
    {
        vertexnode[index] = tmp_node_set.vertexnode[index];
    }
    for(int index = 0; index < usr_num; index++)
    {
        usrnode[index] = tmp_node_set.usrnode[index];
    }
    for(int index = 0; index < edge_num; index++)
    {
        edgenode[index] = tmp_node_set.edgenode[index];
    }

    //
    solved_set = tmp_solved_set;
}

void Adjlist::set_path(Path path)
{

    if(path.begin() == path.end())
    {
        return;
    }
    //获取路径中的消费节点，若最后一个节点不是消费节点，返回
    int usr = get_path_usr(path);
    if(-1 == usr)
    {
        return;
    }

    //消费节点的流量已满足要求，返回
    if(usrnode[usr].flag == true)
    {
        //printf("the usr bandwidth is enough, no need for the path!\n");
        //show_path(path);
        //printf("\n");
        return;
    }

    //获取路径的最大可用带宽，若大于消费节点需求流量，则只使用该需求流量的数值

    int bandwidth = get_max_bandwidth(path);

    if(bandwidth <= 0)
    {
        //printf("the bandwidth of the path is zero!\n");
        //show_path(path);
        //printf("\n");
        return;
    }
    if(bandwidth > usrnode[usr].req_bandwidth)
    {
        bandwidth = usrnode[usr].req_bandwidth;
    }

    //更新路径上各个链路的带宽信息
    vector<int>::iterator iter = path.begin();
    EdgeNode *tmp_edge;

    while(iter != path.end())
    {
        tmp_edge = vertexnode[*iter].firstedge;
        iter++;
        if(iter == path.end())
        {
            break;
        }
        while(NULL != tmp_edge)
        {
            if(tmp_edge->vertex == *iter)
            {
                tmp_edge->empty_bandwidth-=bandwidth;
                tmp_edge->used_bandwidth+=bandwidth;
                if(tmp_edge->used_bandwidth >= tmp_edge->total_bandwidth)
                {
                    tmp_edge->flag =true;
                }
                break;
            }
            tmp_edge = tmp_edge->nextedge;
        }
    }
    //更新消费节点的带宽信息
    usrnode[usr].owned_bandwidth+=bandwidth;
    usrnode[usr].req_bandwidth-=bandwidth;
    if(usrnode[usr].owned_bandwidth >= usrnode[usr].total_bandwidth)
    {
        usrnode[usr].flag = true;
    }
    //更新服务器放置信息
    set_server(*path.begin());
    //将路径信息存入解集
    Path tmp_path = path;
    tmp_path.push_back(usr);
    tmp_path.push_back(bandwidth);
    solved_set.path_set.push_back(tmp_path);
    /*
    printf("set a path!the bandwidth is %d\n", bandwidth);
    show_path(path);
    printf("\n");
    */

}

void Adjlist::set_server(int vertex)
{
    if(vertexnode[vertex].flag == false)
    {
        server_num++;
        solved_set.server_num++;
        solved_set.server_set.push_back(vertexnode[vertex].vertex);
        vertexnode[vertex].flag = true;
    }
    if(-1 != vertexnode[vertex].usr && usrnode[vertexnode[vertex].usr].flag == false)
    {
        Path tmp_path;
        tmp_path.push_back(vertexnode[vertex].vertex);
        tmp_path.push_back(vertexnode[vertex].usr);
        tmp_path.push_back(usrnode[vertexnode[vertex].usr].total_bandwidth);
        usrnode[vertexnode[vertex].usr].flag = true;
        usrnode[vertexnode[vertex].usr].owned_bandwidth = usrnode[vertexnode[vertex].usr].total_bandwidth;
        usrnode[vertexnode[vertex].usr].req_bandwidth = 0;
        solved_set.path_set.push_back(tmp_path);
    }
}

int Adjlist::get_path_usr(Path path)
{
    //判断路径最后一个节点是否为消费节点
    int flag = -1;
    Path reverse_path = path;
    reverse(reverse_path.begin(), reverse_path.end());
    vector<int>::iterator iter = reverse_path.begin();
    for(int index = 0; index < usr_num; index++)
    {
        if(usrnode[index].linkednode->vertex == *iter)
        {
            flag = index;
            break;
        }
    }
    if(-1 == flag)     //若不是消费节点，则退出
    {
        printf("not a usr path!\n");
        show_path(path);
        printf("\n");
        return flag;
    }
    //printf("a usr path!\n");
    return flag;
}

int Adjlist::get_usr_cost(int usr)
{
    if(true == usrnode[usr].linkednode->flag)
    {
        printf("the path cost of usr%d is %d!\n", usr, server_cost);
        return server_cost;
    }
    //
    vector<Path>::iterator path_iter;
    Path tmp_path;
    int tmp_bandwidth;
    int path_cost = 0;
    for(path_iter = solved_set.path_set.begin(); path_iter != solved_set.path_set.end(); path_iter++)
    {
        tmp_path = *path_iter;
        tmp_bandwidth = tmp_path.back();
        tmp_path.pop_back();
        if(tmp_path.back() == usr)
        {
            tmp_path.pop_back();
            path_cost+=(tmp_bandwidth*get_path_cost(tmp_path));
        }
    }
    printf("the path cost of usr%d is %d!\n", usr, path_cost);
    return path_cost;
}

int Adjlist::get_path_cost(Path path)
{
    //
    if(true == path.empty())
    {
        printf("empty paht!\n");
        return -1;
    }
    if(1 == path.size())
    {
        return usrnode[vertexnode[*path.begin()].usr].total_bandwidth;
    }

    int path_cost = 0;
    vector<int>::iterator iter = path.begin();
    VertexNode tmp_vertex;
    EdgeNode *tmp_edge;

    //
    while(iter != path.end())
    {
        tmp_vertex = vertexnode[*iter];
        tmp_edge = tmp_vertex.firstedge;
        iter++;
        if(iter == path.end())
        {
            break;
        }
        while(tmp_edge->vertex != *iter)
        {
            tmp_edge = tmp_edge->nextedge;
            if(tmp_edge == NULL)
            {
                return -1;
            }
        }
        path_cost+= tmp_edge->cost;
    }
    printf("the cost of the path is %d", path_cost);
    return path_cost;
}

void Adjlist::clear_solved_set()
{
    //
    solved_set.path_num = 0;
    solved_set.server_num = 0;
    solved_set.path_set.clear();
    //solved_set.server_set = isolated_usr;
    solved_set.server_set.clear();

    //
    for(int index = 0; index < node_num; index++)
    {
        vertexnode[index].flag = false;
    }
    for(int index = 0; index < edge_num*2; index++)
    {
        edgenode[index].empty_bandwidth =  edgenode[index].total_bandwidth;
        edgenode[index].used_bandwidth = 0;
        edgenode[index].flag = false;
    }
    for(int index = 0; index < usr_num; index++)
    {
        usrnode[index].owned_bandwidth = 0;
        usrnode[index].req_bandwidth = usrnode[index].total_bandwidth;
        usrnode[index].flag = false;
    }

}

int Adjlist::cal_total_cost(SolvedSet solved_set)
{
    int all_server_cost = server_cost*usr_num;
    int total_cost = 0;

    Path tmp_path;
    int tmp_bandwidth;
    EdgeNode *tmp_edge;
    vector<Path>::iterator path_iter = solved_set.path_set.begin();
    vector<int>::iterator iter;
    while(solved_set.path_set.end() != path_iter)
    {
        tmp_path = *path_iter;
        tmp_bandwidth = tmp_path.back();
        tmp_path.pop_back();
        tmp_path.pop_back();
        iter = tmp_path.begin();
        while(iter != tmp_path.end())
        {
            tmp_edge = vertexnode[*iter].firstedge;
            iter++;
            if(iter == tmp_path.end())
            {
                break;
            }
            while(tmp_edge->vertex != *iter)
            {
                tmp_edge = tmp_edge->nextedge;
            }
            total_cost+=(tmp_edge->cost*tmp_bandwidth);
        }
        path_iter++;
    }
    total_cost+=(solved_set.server_num*server_cost);
    printf("the total cost is %d/%d\nthe server num is %d, the server location is ", total_cost, all_server_cost, solved_set.server_num);
    vector<int> tmp_server_set = solved_set.server_set;
    sort(tmp_server_set.begin(), tmp_server_set.end());

    for(int index = 0; index < solved_set.server_set.size(); index++)
    {      
        printf("%d ", tmp_server_set[index]);
    }
    printf("\n\n");

    return total_cost;
}

void Adjlist::remove_path(Path path, int bandwidth)
{
    //
    int usr = get_path_usr(path);
    if(-1 == usr)
    {
        return;
    }

    //
    vector<int>::iterator iter = path.begin();
    EdgeNode *tmp_edge;
    while(iter != path.end())
    {
        tmp_edge = vertexnode[*iter].firstedge;
        iter++;
        if(iter == path.end())
        {
            break;
        }
        if(tmp_edge->vertex != *iter)
        {
            tmp_edge = tmp_edge->nextedge;
        }
        tmp_edge->used_bandwidth-=bandwidth;
        tmp_edge->empty_bandwidth+=bandwidth;
    }

    //
    if(true ==  usrnode[usr].flag)
    {
        usrnode[usr].flag = false;
    }
    usrnode[usr].owned_bandwidth-=bandwidth;
    usrnode[usr].req_bandwidth+=bandwidth;
}

void Adjlist::move_server(int src, int des)
{
    if(false == vertexnode[src].flag)
    {
        printf("the source%d is not a server!\n", src);
        return;
    }
    if(true == vertexnode[des].flag)
    {
        printf("the destination%d is already a server!\n", des);
        return;
    }
    //
    remove_server(src);
    set_server(des);
}

void Adjlist::remove_server(int server)
{
    if(false == vertexnode[server].flag)
    {
        printf("vertex%d is not a server!\n", server);
        return;
    }

    //
    vector<int>::iterator server_iter;
    for(server_iter = solved_set.server_set.begin(); server_iter != solved_set.server_set.end(); server_iter++)
    {
        if(*server_iter == server)
        {
            solved_set.server_num--;
            solved_set.server_set.erase(server_iter);
            vertexnode[server].flag = false;
            break;
        }
    }

    //
    vector<Path>::iterator path_iter;
    Path tmp_path;
    int tmp_bandwidth;
    for(path_iter = solved_set.path_set.begin(); path_iter != solved_set.path_set.end(); path_iter++)
    {
        tmp_path = *path_iter;
        if(*tmp_path.begin() == server)
        {
            tmp_bandwidth = tmp_path.back();
            tmp_path.pop_back();
            tmp_path.pop_back();
            remove_path(tmp_path, tmp_bandwidth);
            solved_set.path_set.erase(path_iter);
        }
    }
}

void Adjlist::solve_casedemo()
{
    clear_solved_set();

    Path tmp_path;
    set_server(0);
    set_server(3);
    set_server(22);
    //
    vector<int> random_usr_set;
    for(int index = 0; index < usr_num; index++)
    {
        random_usr_set.push_back(index);
    }
    random_shuffle(random_usr_set.begin(), random_usr_set.end());
    vector<int>::iterator iter;
    for(iter = random_usr_set.begin(); iter != random_usr_set.end(); iter++)
    {
        search_flow_to_usr(*iter);
    }
    for(int index = 0; index < usr_num; index++)
    {
        if(usrnode[index].flag == false)
        {
            tmp_path.clear();
            tmp_path.push_back(usrnode[index].linkednode->vertex);
            set_path(tmp_path);
        }
    }
    min_solved_set = solved_set;
}

void Adjlist::solve_opt(int cnt)
{
    int tmp_cost;
    int min_cost = INF;
    for(int index = 0; index < cnt; index++)
    {
        for(int index = 0; index < 5; index++)
        {
            clear_solved_set();

            Path tmp_path;
            vector<int>::iterator server_iter;
            for(server_iter = solved_array.server_set[index].begin(); server_iter != solved_array.server_set[index].end(); server_iter++)
            {
                set_server(*server_iter);
            }

            vector<int> random_usr_set;
            for(int index = 0; index < usr_num; index++)
            {
                random_usr_set.push_back(index);
            }
            random_shuffle(random_usr_set.begin(), random_usr_set.end());
            vector<int>::iterator iter;
            for(iter = random_usr_set.begin(); iter != random_usr_set.end(); iter++)
            {
                search_flow_to_usr(*iter);
            }
            for(int index = 0; index < usr_num; index++)
            {
                if(usrnode[index].flag == false)
                {
                    tmp_path.clear();
                    tmp_path.push_back(usrnode[index].linkednode->vertex);
                    set_path(tmp_path);
                }
            }
            tmp_cost = cal_total_cost(solved_set);
            printf("aaa\n");
            if(tmp_cost < min_cost)
            {
                min_cost = tmp_cost;
                min_solved_set = solved_set;
            }
            end_time = clock();
            if((end_time - start_time)/1000000 > 87)
            {
                return;
            }
        }
    }
}

void Adjlist::solve_random()
{
    vector<int> tmp_server_set;
    int tmp_vertex;
    get_isolated_usr();
    for(int index = 0; index < isolated_usr.size(); index++)
    {
        tmp_vertex = usrnode[isolated_usr[index]].linkednode->vertex;
        tmp_server_set.push_back(tmp_vertex);
    }

    int cnt = 0;
    if(node_num < 200)
    {
        cnt = 25;
        for(int index = 0; index < cnt; index++)
        {
            sa_add_server(tmp_server_set);
        }
    }
    if(node_num >= 200 && node_num < 400)
    {
        cnt = 50;
        for(int index = 0; index < cnt; index++)
        {
            sa_add_server(tmp_server_set);
        }
    }
    if(node_num >= 400)
    {
        cnt = 115;
        for(int index = 0; index < cnt; index++)
        {
            sa_add_server(tmp_server_set);
        }
    }
    random_server_set = tmp_server_set;
}

void Adjlist::sa_add_server(vector<int> &server_set)
{
    if(server_set.size() >= usr_num)
    {
        return;
    }
    //
    int a = 0;
    int b = node_num;
    int random;
    //
    while(1)
    {
        random = (rand() % (b-a))+ a;
        if(false == vertexnode[random].flag)
        {
            server_set.push_back(random);
            return;
        }
    }
}

void Adjlist::sa_remove_server(vector<int> &server_set)
{
    if(server_set.size() <= 1)
    {
        return;
    }
    //
    int a = 0;
    int b = server_set.size();
    int random;
    int tmp;
     vector<int>::iterator iter;
    //
    while(1)
    {
        random = (rand()%(b-a))+a;
        iter = server_set.begin()+random;
        if(-1 == vertexnode[random].usr || false == usrnode[vertexnode[random].usr].isolated)
       // if(false == usrnode[vertexnode[random].usr].isolated)
        {
            tmp = *iter;
            server_set.erase(iter);
            break;
        }
    }
}

void Adjlist::sa_move_sever(vector<int> &server_set)
{
    sa_remove_server(server_set);
    sa_add_server(server_set);
}

void Adjlist::solve_sa(int cnt, double init_T, double min_T, double lambda)
{
    //int opt_cnt = 0;
    //初始解
    //init_solution();
    //int min_cost = INF;
    srand((unsigned)time(NULL));
    for(int index = 0; index < cnt; index++)
    {
        //SA算法参数
        double tmp_T = init_T;    //初始温度
        //
        int min_cost = cal_total_cost(min_solved_set);   //最小费用
        //int min_cost = INF;
        int tmp_cost;    //当前费用
        vector<int> min_server_set = solved_set.server_set;   //最小费用的服务器集合
       // vector<int> min_server_set = random_server_set;
        vector<int> tmp_server_set;   //当前服务器集合
        vector<int>::iterator server_iter;   //服务器集合迭代器

        //模拟退火算法
        int a = 0;
        int b = 100;
        int random;

        while(tmp_T > min_T)
        {
            //srand((unsigned)time(NULL));
            this->cnt++;
            tmp_server_set = min_server_set;
            //随机策略
            random = (rand() % (b-a))+ a;

            if(0 <= random && 60 > random)
            {
               sa_remove_server(min_server_set);    //随机移除一个服务器
            }

            if(60 <= random && 100 > random)
            {
                sa_add_server(min_server_set);      //随机增加一个服务器
            }
            /*
            if(70 <= random && 100 > random)
            {
                sa_move_sever(min_server_set);          //随机改变一个服务器位置
            }
            */

            //根据当前的min_server_set求解
            clear_solved_set();
            for(server_iter = min_server_set.begin(); server_iter != min_server_set.end(); server_iter++)
            {
                set_server(*server_iter);
            }

            for(int index = 0; index < usr_num; index++)
            {
               search_flow_to_usr(index);
            }

            for(int index = 0; index < usr_num; index++)
            {
                if(usrnode[index].flag == false)
                {
                    set_server(usrnode[index].linkednode->vertex);
                }
            }
            tmp_cost = cal_total_cost(solved_set);
            //判断是否接受新解
            if(min_cost >= tmp_cost)    //当前费用小于之前的最小费用，接受新解
            {
                //opt_cnt = 0;
                min_cost = tmp_cost;
                min_solved_set = solved_set;
            }
            else
            {
                if(exp((min_cost - tmp_cost) / tmp_T) > exp((min_T - tmp_T) / tmp_T))   //以一定概率接受新解
               {
                    min_cost = tmp_cost;
                    min_solved_set = solved_set;
                }
                else     //抛弃新解
                {
                    min_server_set = tmp_server_set;
                }
            }
            tmp_T*=lambda;   //温度衰减

            //判断是否超时
            end_time = clock();
            if((end_time - start_time)/100000 > 875)
            {
                return;
            }
        }
    }
}

void Adjlist::solve_sa_spfa(int cnt, double init_T, double min_T, double lambda)
{
    //定时器

    //初始解
    //init_solution();
    //int min_cost = cal_total_cost(min_solved_set);   //最小费用
    for(int index = 0; index < cnt; index++)
    {
        srand((unsigned)time(NULL));
        //SA算法参数（只调这三个参数）
        double tmp_T = init_T;    //初始温度
        //double min_T = 0.1;      //最小温度
        //double lambda = 0.99;   //衰减指数

        //

        int tmp_cost;    //当前费用
        int min_cost = cal_total_cost(min_solved_set);
        // int min_cost = INF;
        vector<int> min_server_set = solved_set.server_set;   //最小费用的服务器集合
        vector<int> tmp_server_set;   //当前服务器集合
        vector<int>::iterator server_iter;   //服务器集合迭代器

        //模拟退火算法
        int a = 0;
        int b = 10;
        int random;

        while(tmp_T > min_T)
        {
            this->cnt++;
            tmp_server_set = min_server_set;
            //随机策略
            random = (rand() % (b-a))+ a;
            if(0 == random || 1 == random || 2 == random || 8 == random)
            {
               sa_move_sever(min_server_set);    //随机移除一个服务器
            }

            if(7 == random)
            {
                sa_move_sever(min_server_set);      //随机增加一个服务器
            }
            /*
            else
            {
                sa_remove_server(min_server_set);
            }
            */
            if(3 == random || 4 == random || 5 == random|| 6 == random || 9 == random)
            {
                sa_move_sever(min_server_set);          //随机改变一个服务器位置
            }
            //根据当前的min_server_set求解
            server_num = 0;
            clear_solved_set();
            for(server_iter = min_server_set.begin(); server_iter != min_server_set.end(); server_iter++)
            {
                //server_num++;
                printf("%d ", *server_iter);
                set_server(*server_iter);
            }
            printf("\n");

            insert_edge(min_server_set);
            create_super_node(min_server_set);

            tmp_cost = get_min_cost_flow(node_num, node_num+1);
            printf("the total cost is %d/%d!\n\n", tmp_cost, server_cost*usr_num);

            //判断是否接受新解
            if(min_cost > tmp_cost)    //当前费用小于之前的最小费用，接受新解
            {
                min_cost = tmp_cost;
                this->min_cost = min_cost;
                min_solved_set = solved_set;
            }
            else
            {
                //if(exp((min_cost - tmp_cost) / tmp_T) > exp((min_T - tmp_T) / tmp_T))   //以一定概率接受新解
              //  {
                  //  min_cost = tmp_cost;
                  //  this->min_cost = min_cost;
                  //  min_solved_set = solved_set;
             //   }
              //  else     //抛弃新解
               // {
                    min_server_set = tmp_server_set;
              //  }
            }
            tmp_T*=lambda;   //温度衰减

            //判断是否超时
            end_time = clock();
            if((end_time - start_time)/1000000 > 87)
            {
                return;
            }
        }
    }
}

void Adjlist::solve_sa_opt(int cnt, double init_T, double min_T, double lambda)
{
    for(int index = 0; index < cnt; index++)
    {
        //SA算法参数
        double tmp_T = init_T;    //初始温度
        //
        int min_cost = INF;
        int tmp_cost;    //当前费用
        vector<int> min_server_set = solved_set.server_set;   //最小费用的服务器集合
        vector<int> tmp_server_set;   //当前服务器集合
        vector<int>::iterator server_iter;   //服务器集合迭代器

        //模拟退火算法
        int a = 0;
        int b = 10;
        int random;

        while(tmp_T > min_T)
        {
            this->cnt++;
            tmp_server_set = min_server_set;
            //随机策略
            random = (rand() % (b-a))+ a;

            if(0 == random || 1 == random || 2 == random || 8 == random)
            {
               sa_remove_server(min_server_set);    //随机移除一个服务器
            }
            if(7 == random)
            {
                sa_add_server(min_server_set);      //随机增加一个服务器
            }
            if(3 == random || 4 == random || 5 == random|| 6 == random || 9 == random)
            {
                sa_move_sever(min_server_set);          //随机改变一个服务器位置
            }

            //根据当前的min_server_set求解
            clear_solved_set();
            for(server_iter = min_server_set.begin(); server_iter != min_server_set.end(); server_iter++)
            {
                set_server(*server_iter);
            }

            for(int index = 0; index < usr_num; index++)
            {
               search_flow_to_usr(index);
            }

            for(int index = 0; index < usr_num; index++)
            {
                if(usrnode[index].flag == false)
                {
                    set_server(usrnode[index].linkednode->vertex);
                }
            }
            tmp_cost = cal_total_cost(solved_set);

            SolvedSet tmp_solved_set = solved_set;
            clear_solved_set();
            server_num = 0;
            vector<int>::iterator server_iter;
            for(server_iter = tmp_solved_set.server_set.begin(); server_iter != tmp_solved_set.server_set.end(); server_iter++)
            {
                set_server(*server_iter);
            }
            insert_edge(solved_set.server_set);
            create_super_node(solved_set.server_set);
            tmp_cost = get_min_cost_flow(node_num, node_num+1);
            //判断是否接受新解
            if(min_cost >= tmp_cost)    //当前费用小于之前的最小费用，接受新解
            {
                //opt_cnt = 0;
                min_cost = tmp_cost;
                min_solved_set = solved_set;
            }
            else
            {
                if(exp((min_cost - tmp_cost) / tmp_T) > exp((min_T - tmp_T) / tmp_T))   //以一定概率接受新解
               {
                    min_cost = tmp_cost;
                    min_solved_set = solved_set;
                }
                else     //抛弃新解
                {
                    min_server_set = tmp_server_set;
                }
            }
            tmp_T*=lambda;   //温度衰减

            //判断是否超时
            end_time = clock();
            if((end_time - start_time)/1000000 > 87)
            {
                return;
            }
        }
    }
}

void Adjlist::insert_edge(vector<int> server_set)
{
    init_edges.resize(edge_num*4 + server_set.size()*2 + usr_num*2);
    final_edges.resize(edge_num*4 + server_set.size()*2 + usr_num*2);

    edges.resize(edge_num*4 + server_set.size()*2 + usr_num*2);
    head.resize(node_num+2);
    for(int index = 0; index < node_num+2; index++)
    {
        head[index] = -1;
    }
    path.resize(node_num+2);
    dist.resize(node_num+2);
    pre.resize(node_num+2);

    edge_count = 0;
    for(int index = 0; index < edge_num; index++)
    {
        edges[edge_count].u = edgenode[index*2].former;
        edges[edge_count].v = edgenode[index*2].vertex;
        edges[edge_count].vol = edgenode[index*2].total_bandwidth;
        edges[edge_count].cost = edgenode[index*2].cost;
        edges[edge_count].next = head[edgenode[index*2].former];
        head[edgenode[index*2].former] = edge_count++;

        edges[edge_count].u = edgenode[index*2].vertex;
        edges[edge_count].v = edgenode[index*2].former;
        edges[edge_count].vol = 0;         //vol为0，表示开始时候，该边的反向不通
        edges[edge_count].cost = -edgenode[index*2].cost;    //cost 为正向边的cost相反数，这是为了
        edges[edge_count].next = head[edgenode[index*2].vertex];
        head[edgenode[index*2].vertex] = edge_count++;

        edges[edge_count].u = edgenode[index*2+1].former;
        edges[edge_count].v = edgenode[index*2+1].vertex;
        edges[edge_count].vol = edgenode[index*2+1].total_bandwidth;
        edges[edge_count].cost = edgenode[index*2+1].cost;
        edges[edge_count].next = head[edgenode[index*2+1].former];
        head[edgenode[index*2+1].former] = edge_count++;

        edges[edge_count].u = edgenode[index*2+1].vertex;
        edges[edge_count].v = edgenode[index*2+1].former;
        edges[edge_count].vol = 0;         //vol为0，表示开始时候，该边的反向不通
        edges[edge_count].cost = -edgenode[index*2+1].cost;    //cost 为正向边的cost相反数，这是为了
        edges[edge_count].next = head[edgenode[index*2+1].vertex];
        head[edgenode[index*2+1].vertex] = edge_count++;
    }
}

void Adjlist::create_super_node(vector<int> server_set)
{
    //SuperSrc
    for(int index = 0; index < server_set.size(); index++)
    {
        edges[edge_count].u = node_num;
        edges[edge_count].v = vertexnode[solved_set.server_set[index]].vertex;
        edges[edge_count].vol = INF;
        edges[edge_count].cost = 0;
        edges[edge_count].next = head[node_num];
        head[node_num] = edge_count++;
    
        edges[edge_count].u = vertexnode[solved_set.server_set[index]].vertex;
        edges[edge_count].v = node_num;
        edges[edge_count].vol = 0;         //vol为0，表示开始时候，该边的反向不通
        edges[edge_count].cost = 0;    //cost 为正向边的cost相反数，这是为了
        edges[edge_count].next = head[vertexnode[solved_set.server_set[index]].vertex];
        head[vertexnode[solved_set.server_set[index]].vertex] = edge_count++;
    }

    //SuperDes
    for(int index = 0; index < usr_num; index++)
    {
        edges[edge_count].u = usrnode[index].linkednode->vertex;
        edges[edge_count].v = node_num+1;
        edges[edge_count].vol = usrnode[index].total_bandwidth;
        edges[edge_count].cost = 0;
        edges[edge_count].next = head[usrnode[index].linkednode->vertex];
        head[usrnode[index].linkednode->vertex] = edge_count++;
    
        edges[edge_count].u = node_num+1;
        edges[edge_count].v = usrnode[index].linkednode->vertex;
        edges[edge_count].vol = 0;         //vol为0，表示开始时候，该边的反向不通
        edges[edge_count].cost = 0;    //cost 为正向边的cost相反数，这是为了
        edges[edge_count].next = head[node_num+1];
        head[node_num+1] = edge_count++;
    }

    init_edges = edges;
    final_edges = edges;
}

bool Adjlist::spfa(int s, int t)
{
    //
    for(int index = 0; index < node_num + 2; index++)
    {
        pre[index] = NOVERTEX;
    }
    for(int index = 0; index < node_num + 2; index++)
    {
        dist[index] = INF;
    }
    //
    queue<int> spfa_queue;
    dist[s] = 0;
    spfa_queue.push(s);
    while (!spfa_queue.empty())
    {
        int u = spfa_queue.front();
        spfa_queue.pop();

        for (int e = head[u]; e != -1; e = edges[e].next)
        {
            int v = edges[e].v;
            if (edges[e].vol > 0 && dist[u] + edges[e].cost < dist[v])
            {
                dist[v] = dist[u] + edges[e].cost;
                pre[v] = u; //前一个点
                path[v] = e;//该点连接的前一个边
                spfa_queue.push(v);
            }
        }
    }

    if (pre[t] == -1)  //若终点t没有设置pre，说明不存在到达终点t的路径
    {
        return false;
    }

    Path tmp_path;
    for(int e = pre[t]; e != -1; e = pre[e])
    {
        tmp_path.push_back(e);
    }
    tmp_path.pop_back();
    reverse(tmp_path.begin(), tmp_path.end());
    tmp_path_info.path = tmp_path;
    /*
    printf("%d->", t);
    for(int e = pre[t]; e != -1; e = pre[e])
    {
        printf("%d->", e);
    }
    printf("\n");
    */

    return true;
}

int Adjlist::get_min_cost_flow(int s, int t)
{
    int total_cost = 0;
    int total_flow = 0;
    while (spfa(s, t))
    {
        int tmp_flow = INF;
        for (int u = t; u != s; u = pre[u])
        {
            if (edges[path[u]].vol < tmp_flow)
            {
                tmp_flow = edges[path[u]].vol;
            }
        }
        total_flow += tmp_flow;
        total_cost += dist[t] * tmp_flow;
        for (int u = t; u != s; u = pre[u])
        {
            edges[path[u]].vol -=  tmp_flow;   //正向边容量减少
            edges[path[u]^1].vol +=  tmp_flow; //反向边容量增加
        }
    }

    //printf("the total flow is %d!\n", total_flow);
    /*
    if(total_flow != this->total_flow)
    {
        return -1;
    }
    */
    for(int index = 0; index < usr_num; index++)
    {
        int num = edge_num*4 + server_num*2 + index*2;
        if(edges[num].vol > 0)
        {
            //set_server(edges[num].u);
            return INF;
        }
    }
    total_cost+=(server_cost*server_num);

    return total_cost;
}

int Adjlist::get_solved_set(int s, int t)
{
    for(int index = 0; index < edges.size(); index++)
    {
        edges[index].vol = init_edges[index].vol - edges[index].vol;
    }

    solved_set.path_set.clear();
    Path tmp_path;
    int usr;

    while (spfa(s, t))
    {
        tmp_path = tmp_path_info.path;
        usr = vertexnode[tmp_path.back()].usr;
        tmp_path.push_back(usr);
        int tmp_flow = INF;
        for (int u = t; u != s; u = pre[u])
        {
            if (edges[path[u]].vol < tmp_flow)
            {
                tmp_flow = edges[path[u]].vol;
            }
        }
        total_flow += tmp_flow;
        total_cost += dist[t] * tmp_flow;
        for (int u = t; u != s; u = pre[u])
        {
            edges[path[u]].vol -=  tmp_flow;   //正向边容量减少
            edges[path[u]^1].vol +=  tmp_flow; //反向边容量增加
        }
        tmp_path.push_back(tmp_flow);
        solved_set.path_set.push_back(tmp_path);
    }
}

int Adjlist::solve_case()
{
    SolvedSet tmp_solved_set;
    int tmp_cost;
    int min_cost = INF;


    //srand((unsigned)time(NULL));
    if(node_num < 200)
    {
        init_solution();
        solve_sa(15, 10, 0.1, 0.99);
        tmp_cost = cal_total_cost(min_solved_set);
        if(tmp_cost < min_cost)
        {
            tmp_solved_set = min_solved_set;
            min_cost = tmp_cost;
        }
    }
    if(node_num >= 200 && node_num < 400)
    {
        init_solution();
        solve_sa(10, 10, 0.1, 0.99);
        tmp_cost = cal_total_cost(min_solved_set);
        if(tmp_cost < min_cost)
        {
            tmp_solved_set = min_solved_set;
            min_cost = tmp_cost;
        }
    }
    if(node_num >= 400)
    {
        for(int index = 0; index < 1; index++)
        {
            init_solution();
            solve_sa(2, 10, 0.1, 0.99);
            solve_sa(4, 100, 0.1, 0.98);
            solve_sa(2, 10, 0.1, 0.99);
            /*
            solve_sa(2, 10, 0.1, 0.99);
            solve_sa(4, 500, 1, 0.96);
            solve_sa(2, 200, 1, 0.96);
            solve_sa(2, 80, 1, 0.98);
            solve_sa(1, 10, 1, 0.98);
            */
            tmp_cost = cal_total_cost(min_solved_set);
            if(tmp_cost < min_cost)
            {
                tmp_solved_set = min_solved_set;
                min_cost = tmp_cost;
            }
        }
    }

    clear_solved_set();
    server_num = 0;
    vector<int>::iterator server_iter;
    for(server_iter = tmp_solved_set.server_set.begin(); server_iter != tmp_solved_set.server_set.end(); server_iter++)
    {
        set_server(*server_iter);
    }
    insert_edge(solved_set.server_set);
    create_super_node(solved_set.server_set);
    get_min_cost_flow(node_num, node_num+1);
    get_solved_set(node_num, node_num+1);
    printf("cnt = %d!\n\n", cnt);
    return cal_total_cost(solved_set);
}
