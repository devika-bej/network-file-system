#include "headers.h"
#include "defs.h"

typedef struct node node;

struct node
{
    char filename[512];
    node *next;
};

node *create_node(char new_file[]);

node *insert_node(node *root, node *ele);

node *delete_node(node *root, char del_file[]);

int search_node(node *root, char search_file[]);

void print_list(node *root);