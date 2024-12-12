
#include "tries.h"

// Function to initialize a queue
Queue *createQueue(int capacity)
{
    Queue *queue = (Queue *)malloc(sizeof(Queue));
    queue->array = (Node *)malloc(capacity * sizeof(Node));
    queue->front = queue->rear = -1;
    queue->capacity = capacity;
    return queue;
}

// Function to check if the queue is empty
int isQueueEmpty(Queue *queue)
{
    return queue->front == -1;
}

// Function to enqueue a node
void enqueue(Queue *queue, Node node)
{
    if (queue->rear == queue->capacity - 1)
    {
        // Queue is full
        return;
    }

    queue->array[++queue->rear] = node;

    if (queue->front == -1)
    {
        // If the queue was empty, set the front
        queue->front = 0;
    }
}

// Function to dequeue a node
Node dequeue(Queue *queue)
{
    if (isQueueEmpty(queue))
    {
        // Queue is empty
        return NULL;
    }

    Node node = queue->array[queue->front];

    if (queue->front == queue->rear)
    {
        // If there was only one element in the queue, reset the queue
        queue->front = queue->rear = -1;
    }
    else
    {
        // Move to the next element in the queue
        queue->front++;
    }

    return node;
}

Node createNode(char *name, int access, struct ss_id *ss)
{
    Node new_Node = (Node)malloc(sizeof(struct node));

    if (new_Node == NULL)
    {
        fprintf(stderr, "Memory allocation error\n");
        exit(EXIT_FAILURE);
    }

    new_Node->name = strdup(name);

    if (new_Node->name == NULL)
    {
        fprintf(stderr, "Memory allocation error\n");
        exit(EXIT_FAILURE);
    }

    new_Node->give_access = (access == 1) ? true : false;
    new_Node->ss = ss;
    new_Node->children = NULL;
    new_Node->next_sibling = NULL;
    new_Node->parent = NULL;
    new_Node->redundant_data = false;

    return new_Node;
}

Node insertNode(Node root, char *prev, char *name, int access, struct ss_id *ss)
{
    Node newNode = createNode(name, access, ss);

    if (root == NULL)
    {
        return newNode;
    }

    // Queue for BFS
    Node queue[1000];
    int front = -1, rear = -1;

    queue[++rear] = root;

    while (front != rear)
    {
        Node current = queue[++front];
        int flag = 0;

        while (current != NULL)
        {
            // Check if the node with the specified name already exists
            if (strcmp(current->name, name) == 0)
            {
                // printf("Node with name '%s' already exists. Skipping insertion.\n", name);
                if (!current->give_access)
                    current->give_access = newNode->give_access;
                return root; // Return without making changes
            }

            if (strcmp(current->name, prev) == 0)
            {
                flag = 1;
                break;
            }

            if (current->next_sibling != NULL)
            {
                queue[++rear] = current->next_sibling;
            }

            current = current->children;
        }

        if (flag == 1)
        {
            // If the 'prev' token is found, insert the new node
            if (current->children == NULL)
            {
                current->children = newNode;
                newNode->parent = current;
                // if (newNode->parent != NULL)
                //     printf("printing parent...%s\n", newNode->parent->name);
                // else
                // {
                //     printf("parent is null\n");
                // }
            }
            else
            {
                // Traverse to the end of the children and check if the name already exists
                current = current->children;

                while (current->next_sibling != NULL)
                {
                    // Check if the node with the specified name already exists
                    if (strcmp(current->name, newNode->name) == 0)
                    {
                        // printf("Node with name '%s' already exists. Skipping insertion.\n", newNode->name);
                        if (!current->give_access)
                            current->give_access = newNode->give_access;
                        return root; // Return without making changes
                    }

                    current = current->next_sibling;
                }

                // Check the last node in the children list
                if (strcmp(current->name, newNode->name) == 0)
                {
                    // printf("Node with name '%s' already exists. Skipping insertion.\n", newNode->name);
                    if (!current->give_access)
                        current->give_access = newNode->give_access;
                    return root; // Return without making changes
                }
                // If the name doesn't exist, add the new node
                if (current->next_sibling != NULL)
                {
                    newNode->parent = current->next_sibling->parent;
                    current->next_sibling = newNode;
                }
                else
                {
                    newNode->parent = current->parent;
                    current->next_sibling = newNode;
                }

                // Print parent information
                // if (newNode->parent == NULL)
                // {
                //     printf("parent is null\n");
                // }
            }
            break;
        }
    }
    return root;
}

Node search(Node root, char *path)
{
    char **finals = tokenize_final_path(path);
    // count number of string in finals
    int count = 0;
    while (finals[count] != NULL)
    {
        count++;
    }
    // printf("count is %d\n", count);
    if (root == NULL || finals == NULL)
    {
        return NULL;
    }

    Queue *queue = createQueue(1000);
    enqueue(queue, root);

    while (!isQueueEmpty(queue) && *finals != NULL)
    {
        Node current = dequeue(queue);
        // printf("printing current name... %s\n", current->name);
        Node extra = current;
        Node temp = NULL;

        while (current != NULL)
        {
            // printf("printing final name... %s\n", finals[0]);

            if (strcmp(current->name, finals[0]) == 0)
            {
                // printf("final[0] found.. now entered if condn\n");
                if (count != 1)
                {
                    while (*finals != NULL)
                    {
                        int found = 0;
                        Node child = current->children;
                        // if (child != NULL)
                        //     printf("printing child name... %s\n", child->name);
                        // else
                        // {
                        //     printf("fuck it is NULL\n");
                        // }
                        // if (child == NULL)
                        // {
                        //     printf("child is null\n");
                        // }
                        // if (child->next_sibling != NULL)
                        //     printf("child ka sibling... %s\n", child->next_sibling->name);
                        // else
                        // {
                        //     printf("fuck it is NULL\n");
                        // }
                        int i = 1;
                        while (child != NULL && i != count)
                        {
                            int flag = 0;
                            child->name = trim(child->name);
                            finals[i] = trim(finals[i]);
                            // printf("%d...final print krwao bc... %s\n", i, finals[i]);
                            // printf("naming %s\n", child->name);
                            if (strcmp(child->name, finals[i]) == 0)
                            {
                                // printf("found baby doll\n");
                                if (i = count - 1)
                                {
                                    found = 1;
                                    temp = child;

                                    break;
                                }
                                if (child->children != NULL)
                                {
                                    flag = 1;
                                    child = child->children;
                                }
                                else
                                    break;
                                // printf("chalo print krwate hain after ...%s\n", temp->name);

                                i++;
                            }

                            if (child->next_sibling != NULL && flag != 1)
                            {
                                child = child->next_sibling;
                                // printf("printing  name... %s\n", child->name);
                            }
                        }
                        // printf("\t\t%d\n", found);
                        if (found == 0)
                        {
                            return NULL;
                        }
                        else
                            break;
                    }

                    // printf("name of root we are returning...%s\n", temp->name);
                    return temp;
                }
                else
                {
                    return current;
                }
            }
            // Enqueue siblings for further exploration
            if (current->next_sibling != NULL)
            {
                enqueue(queue, current->next_sibling);
            }
            // Do not update current to the next sibling here, as it may have children
            current = current->next_sibling;
        }

        // Enqueue children for further exploration
        Node child = extra->children;
        while (child != NULL)
        {
            enqueue(queue, child);
            child = child->next_sibling;
        }
    }

    return NULL;
}

// combine
char *combine(char *cwd, char *path)
{
    char *final_path = (char *)malloc(sizeof(char) * 1024);
    strcpy(final_path, cwd);
    strcat(final_path, path);
    return final_path;
}

char *combine_with_slash(char *name, char *path)
{
    // add a '/' before name and then combine path + new_name
    char *final_path = (char *)malloc(sizeof(char) * 100);
    strcpy(final_path, path);
    strcat(final_path, "/");
    strcat(final_path, name);
    return final_path;
}

char **tokenize_final_path(char *path)
{
    char *token = strtok(path, "/");
    int capacity = 512;
    int i = 0;

    // Allocate memory for the array of pointers
    char **final_token = (char **)malloc(capacity * sizeof(char *));
    // malloc

    if (final_token == NULL)
    {
        // Handle memory allocation failure
        return NULL;
    }

    while (token != NULL)
    {
        // Check if there's enough space in the array
        if (i >= capacity)
        {
            // Double the capacity of the array
            capacity *= 2;
            char **temp = (char **)realloc(final_token, capacity * sizeof(char *));
            if (temp == NULL)
            {
                // Handle memory reallocation failure
                free(final_token);
                return NULL;
            }
            final_token = temp;
        }

        // Allocate memory for the token and copy it
        // final_token[i] = strdup(token);
        final_token[i] = (char *)malloc(1024);
        if (final_token[i] == NULL)
        {
            // Handle memory allocation failure
            for (int j = 0; j < i; ++j)
            {
                free(final_token[j]);
            }
            free(final_token);
            return NULL;
        }
        strncpy(final_token[i], token, 1023); // Copy at most 1023 characters
        final_token[i][1023] = '\0';          // Null-terminate the string
        // Move to the next token
        token = strtok(NULL, "/");
        i++;
    }

    // Resize the array to the actual number of tokens
    char **temp = (char **)realloc(final_token, (i + 1) * sizeof(char *));
    if (temp == NULL)
    {
        // Handle memory reallocation failure
        for (int j = 0; j < i; ++j)
        {
            free(final_token[j]);
        }
        free(final_token);
        return NULL;
    }
    final_token = temp;

    // Set the last element to NULL to indicate the end of the array
    final_token[i] = NULL;

    return final_token;
}

void print_trie(Node root)
{
    Node temp = root;
    // printf("empty\n");
    if (temp == NULL)
    {
        // printf("done\n");
        return;
    }
    printf("%s\n", temp->name);
    print_trie(temp->children);
    print_trie(temp->next_sibling);
}

// Function to print the trie using BFS
void printBFS(Node root)
{
    if (root == NULL)
    {
        return;
    }

    // Queue for BFS
    Queue *queue = createQueue(1000);
    enqueue(queue, root);

    while (!isQueueEmpty(queue))
    {
        Node current = dequeue(queue);
        printf("%s\n", current->name);

        // Enqueue children
        Node child = current->children;
        while (child != NULL)
        {
            enqueue(queue, child);
            child = child->next_sibling;
        }
    }
}

// trim white space from front and back of the string
char *trim(char *str)
{
    char *end;

    // Trim leading space
    while (isspace((unsigned char)*str))
        str++;

    if (*str == 0)
        return str;

    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end))
        end--;
    end[1] = '\0';
    return str;
}

Node deleteNode(Node root, char *delete_node)
{
    if (root == NULL)
    {
        return NULL;
    }
    if (strcmp(root->name, delete_node) == 0)
    {
        free(root);
        return NULL;
    }
    root->next_sibling = deleteNode(root->next_sibling, delete_node);
    root->children = deleteNode(root->children, delete_node);

    return root;
}

// function to shift the node and its subtree to another specified node

Node shift_subtree(Node root, char *source, char *destination)
{
    Node temp = root;
    Node source_node = search(root, source);
    Node destination_node = search(root, destination);
    if (source_node == NULL || destination_node == NULL)
    {
        printf("Either source or destination node is not present in the tree\n");
        return root;
    }
    if (source_node->parent == NULL)
    {
        printf("Source node is the root node\n");
        return root;
    }
    if (source_node->parent == destination_node)
    {
        printf("Destination node is the parent of source node\n");
        return root;
    }
    source_node->redundant_data = true;
    if (source_node->parent->children == source_node)
    {
        source_node->redundant_data = 1;
        source_node->parent->children = source_node->next_sibling;
    }
    else
    {
        Node temp = source_node->parent->children;
        while (temp->next_sibling != source_node)
        {
            temp = temp->next_sibling;
        }
        temp->next_sibling = source_node->next_sibling;
    }
    source_node->parent = destination_node;
    source_node->next_sibling = destination_node->children;
    destination_node->children = source_node;
    return root;
}

// pass the destination ka struct
Node copy_subtree(Node root, char *source, char *destination, struct ss_id *ss)
{
    Node temp = root;
    Node source_node = search(root, source);
    Node destination_node = search(root, destination);
    if (source_node == NULL || destination_node == NULL)
    {
        printf("Either source or destination node is not present in the tree\n");
        return root;
    }
    if (source_node->parent == NULL)
    {
        printf("Source node is the root node\n");
        return root;
    }
    if (source_node->parent == destination_node)
    {
        printf("Destination node is the parent of source node\n");
        return root;
    }
    Node new_node = (Node)malloc(sizeof(struct node));
    new_node->name = (char *)malloc(sizeof(char) * 1024);
    strcpy(new_node->name, source_node->name);
    printf("newnode name before... %s\t", new_node->name);

    new_node->name = combine_with_dot(new_node->name);
    printf("/t newnode name after... %s \n", new_node->name);

    // Node existingNode = search(root, new_node->name);
    // if (existingNode != NULL)
    // {
    // printf("Node with the new name already exists at the destination\n");
    // change name
    // change name
    // }
    // new_node->redundant_data = false;
    new_node->ss = ss;
    new_node->parent = destination_node;
    new_node->children = NULL;
    new_node->next_sibling = destination_node->children;
    destination_node->children = new_node;
    Node temp1 = source_node->children;
    while (temp1 != NULL)
    {
        copy_subtree(root, temp1->name, new_node->name, ss);
        temp1 = temp1->next_sibling;
    }
    return root;
}

char *combine_with_dot(char *string)
{
    // Check if there is a '.' in the string
    char *dotPtr = strchr(string, '.');

    if (dotPtr == NULL)
    {
        // No dot found, append "(copy)" to the string
        char *result = (char *)malloc(strlen(string) + strlen("(copy)") + 1);
        strcpy(result, string);
        strcat(result, "(copy1)");
        return result;
    }
    else
    {
        // Dot found, modify the string
        size_t index = dotPtr - string;
        char *result = (char *)malloc(strlen(string) + strlen("(copy).") + 1);
        strncpy(result, string, index);
        result[index] = '\0';
        strcat(result, "(copy1).");
        strcat(result, dotPtr + 1);
        return result;
    }
}
