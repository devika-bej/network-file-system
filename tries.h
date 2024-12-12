#include "headers.h"
#include "defs.h"

#ifndef TRIES_H
#define TRIES_H

typedef struct node *Node;
typedef struct ss_id ss_struct;

typedef struct server_info server_info;
typedef struct client_info client_info;
typedef struct naming_info naming_info;

struct server_info
{
    int ss_sock;
    struct sockaddr_in ss_addr, nm_addr;
    socklen_t ss_size;
    int server_port;  // to which server can connect
    int ss_accepting; // to connect to that ss
};

struct client_info
{
    int client_sock;
    struct sockaddr_in client_addr, nm_addr;
    socklen_t client_size;
    int client_port;
    int clt_connection;
};

struct naming_info
{
    int nm_sock;
    struct sockaddr_in nm_addr;
    socklen_t nm_size;
    // int nm_port;
    char command[1024];
    int port_to_connect;
    int clt_connection;
    int flag; //0 for create 1 for delete 2 for copy
    char tmp1[1024];
    char tmp2[1024];
    char look[1024];
    char temp[1024];
    int nm_to_ss;
    int clt_to_ss;
    int stop;
    struct ss_id *ss;
    int searched;
};

struct node
{
    char *name;
    bool give_access; // if access == 1 then it is not a cwd if == 0 it is cwd for some
    struct ss_id *ss;
    struct node *children;
    struct node *next_sibling;
    struct node *parent;
    bool redundant_data;
};

typedef struct Queue
{
    Node *array;
    int front, rear, capacity;
} Queue;

Queue *createQueue(int capacity);
int isQueueEmpty(Queue *queue);
void enqueue(Queue *queue, Node node);
Node dequeue(Queue *queue);
Node createNode(char *name, int access, struct ss_id *ss);
Node insertNode(Node root, char *prev, char *name, int access, struct ss_id *ss);
Node search(Node root, char *path);
char *combine(char *cwd, char *path);
char *combine_with_slash(char *name, char *path);
char **tokenize_final_path(char *path);
void print_trie(Node root);
void printBFS(Node root);
char *trim(char *str);
Node deleteNode(Node root, char *delete_node);
Node shift_subtree(Node root, char *source, char *destination);
char *combine_with_dot(char *string);
Node copy_subtree(Node root, char *source, char *destination, struct ss_id *ss);
void combine_for_LRU(char *command, int port_nm, int port_clt,char *cwd);
void append_data(char data[1024], char *filename);
#endif