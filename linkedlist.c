#include "linkedlist.h"

node *create_node(char new_file[])
{
    node *new = (node *)(malloc(sizeof(node)));
    bzero(new->filename, 512);
    strcpy(new->filename, new_file);
    new->next = NULL;
    return new;
}

node *insert_node(node *root, node *ele)
{
    node *curr = root;
    while (curr->next != NULL)
        curr = curr->next;

    curr->next = ele;
    return root;
}

node *delete_node(node *root, char del_file[])
{
    node *curr = root;
    node *last = root;
    while (curr != NULL && curr->next != NULL)
    {
        curr = curr->next;
        if (strcmp(curr->filename, del_file) == 0)
        {
            last->next = curr->next;
            node *temp = curr;
            curr = curr->next;
            free(temp);
        }
        last = last->next;
    }
    return root;
}

int search_node(node *root, char search_file[])
{
    node *curr = root;
    int found = 0;
    while (curr->next != NULL)
    {
        curr = curr->next;
        if (strcmp(curr->filename, search_file) == 0)
        {
            found = 1;
            break;
        }
    }
    return found;
}

void print_list(node *root)
{
    node *curr = root;
    while (curr->next != NULL)
    {
        curr = curr->next;
        printf("%s\n", curr->filename);
    }
}