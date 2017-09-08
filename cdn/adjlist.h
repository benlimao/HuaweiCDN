#ifndef ADJLIST_H
#define ADJLIST_H

#include "deploy.h"
#include <vector>
#include <time.h>
using namespace std;

#define BASIC_INFO_NUM   3
#define LINK_INFO_NUM     4
#define USR_INFO_NUM      3
#define INF                           1000000
#define NOVERTEX               -1

typedef vector<int> Path;    //按节点顺序存放一条路径
typedef vector<Path> PathSet;  

struct EdgeNode     //链路结构体
{
    int vertex;     //链路指向的节点编号
    int total_bandwidth;   //链路上的总带宽
    int used_bandwidth;		//链路上已使用的带宽
    int empty_bandwidth;	//链路上可使用的带宽
    int cost;		//单位带宽的费用
    int former; //
    bool flag;		//链路标志
    EdgeNode *nextedge; 
};

struct VertexNode	//网络节点结构体
{
    int vertex;     //网络节点编号
    int usr;
    int edge_num;	//链路数
    bool flag;	//是否放置服务器的标志
    EdgeNode* firstedge;	
};

struct UsrNode      //消费节点结构体
{
    int usr;	//消费节点编号
    int total_bandwidth;	//消费节点需要的总流量
    int owned_bandwidth;	//已流向消费节点的流量
    int req_bandwidth;		//消费节点还需要的流量
    bool flag;		//消费节点的要求是否已经满足的标志
    bool isolated;
    VertexNode* linkednode;    //消费节点对应的网络节点
};

struct NodeSet
{
    VertexNode *vertexnode;	//网络节点数组
    EdgeNode *edgenode;	//链路数组
    UsrNode *usrnode;	//消费节点数组
};

struct PathInfo    //路径信息结构体
{
    int start_usr;   
    int end_usr;	
    int start_node;
    int end_node;
    Path path;    
    int cost;
};

struct SolvedSet	//解集结构体
{	
    int path_num;	//路径数量
    int server_num;
    vector<int> server_set;
    vector<Path> path_set;
};

class Adjlist
{
public:

    Adjlist(char * topo[MAX_EDGE_NUM]);
    ~Adjlist();

    //处理输入文件
    void create_adjlist();    //创建邻接表
    void get_basic_info();    //根据输入文件得到基本信息
    void get_link_info();     //根据输入文件得到网络节点的信息
    void get_usr_info();      //根据输入文件得到用户节点的信息
    void show_adjlist();      //输出邻接表

    //
    bool search_min_route(int start, int end);   //求两个网络节点之间费用最小的路径
    bool search_nearest_usr(int vertex);
    bool search_nearest_server(int vertex);
    void search_usr_to_usr();	//寻找所有消费节点最近的消费节点
    void devide_path(Path path, int vertex);	//将一条路径从vertx节点一分为二
    int search_server_to_usr(int server, int usr); //寻找并设置消费节点到一个服务器的最短路径，返回路径的费用
    int search_flow_to_usr(int usr); //

    //
    bool is_available_path(Path path);
    bool is_effective_path(Path path);    
    bool is_usr_isolated(int usr);    //判断消费节点是否孤立

    //
    EdgeNode *reverse_edge(EdgeNode *edgenode);   //获取链路的反向链路
    int get_max_bandwidth(Path path);    //获取一条路径所能承受的最大带宽
    void get_isolated_usr();	//获取当前所有的孤立节点
    int get_path_usr(Path path);	//获取路径终点的消费节点编号
    int get_usr_cost(int usr);   //
    int get_path_cost(Path path); //

    //
    void record_current_state();
    void recover_former_state();

    //
    void clear_solved_set();    //重置解集
    void set_path(Path path); //
    void remove_path(Path path, int bandwidth);
    void set_server(int vertex);
    void remove_server(int server);
    void move_server(int src, int des);
    char* get_solved_buffer(SolvedSet solved_set);  //根据解集生成buffer
    int cal_total_cost(SolvedSet solved_set); //根据解集计算总费用

    void show_path(Path path);	//打印路径

    //Solution
    void init_solution();      //求初始解
    void set_server_to_usr();	//在每个消费节点对应的网络节点上放置服务器
    void solve_casedemo();
    void sa_add_server(vector<int> &server_set);
    void sa_remove_server(vector<int> &server_set);
    void sa_move_sever(vector<int> &server_set);
    void solve_sa(int cnt, double init_T, double min_T, double lambda);
    void solve_sa_spfa(int cnt, double init_T, double min_T, double lambda);
    void solve_sa_opt(int cnt, double init_T, double min_T, double lambda);
    void solve_opt(int cnt);
    void solve_random();
    int solve_case();

    //
    void insert_edge(vector<int> server_set);
    void create_super_node(vector<int> server_set);
    bool spfa(int s, int t);
    int get_min_cost_flow(int s, int t);
    int get_solved_set(int s, int t);

public:
    //
    char **adjlist_buffer;    //输入文件的字符串
    int node_num;	//网络节点数
    int edge_num;	//网络节点两两之间的链路数
    int usr_num;	//消费节点数
    int server_cost;	//单个服务器费用
    int total_cost;   

    //
    VertexNode *vertexnode;	//网络节点数组
    EdgeNode *edgenode;	//链路数组
    UsrNode *usrnode;	//消费节点数组
    NodeSet tmp_node_set;

    //
    PathInfo tmp_path_info;	
    vector<PathInfo> min_usr_set;	//消费节节点间最短路径集合
    Path path_one;
    Path path_two;

    //
    SolvedSet solved_set;
    SolvedSet tmp_solved_set;
    SolvedSet min_solved_set;
    vector<int> isolated_usr;
    struct SolvedArray
    {
        vector<int> server_set[5];
        int cost_set[5];
    }solved_array;
    vector<int> random_server_set;

    //
    clock_t start_time;
    clock_t end_time;
    int cnt;

    //MinCostFlow
    struct Edge
    {
        int u;
        int v;
        int vol;
        int cost;
        int next;
    };
    vector<Edge> edges;
    vector<Edge> init_edges;
    vector<Edge> final_edges;
    vector<int> head;
    vector<int> pre;
    vector<int> path;
    vector<int> dist;
    int edge_count;
    int min_cost;
    int server_num;
    int total_flow;
};

#endif // ADJLIST_H
