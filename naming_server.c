#include "tries.h"
#include "defs.h"

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
void combine_for_LRU(char *command, int port_nm, int port_clt, char *cwd);
void append_data(char data[1024], char *filename);
void append_to_file(char data[1024], char *filename);
void combine_with_comma(char *port_of_ack_sender, char *command, char *ack);
char **search_and_extract(char *searchToken);
void extract_tokens(char *line, char *tokens[4]);

sem_t ss_semaphore;
char *ip = "127.0.0.1";
Node root = NULL;

char ack[1024];
int ss_count_for_nm = 0;

pthread_mutex_t mutex_ss = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_tries = PTHREAD_MUTEX_INITIALIZER;

struct ss_id
{
    int ss_id;
    int nm_connect_to_ss;
    int clt_connect_to_ss;
    char *cwd;
    int path_count;
};
// how to use semaphore?
void *nm_connection(void *arg)
{

    // char ack[1024];
    // malloc ack

    char *ack;
    ack = (char *)malloc(1024 * sizeof(char));
    char *buffer_nm;
    buffer_nm = (char *)malloc(1024 * sizeof(char));
    bzero(buffer_nm, 1024);
    char *compare_ack_success;
    compare_ack_success = (char *)malloc(2 * sizeof(char));
    bzero(compare_ack_success, 2);

    printf("NM thread created\n");
    naming_info *nm = (naming_info *)arg;

    // char buffer_nm[1024];

    strcpy(buffer_nm, nm->command);
    int nm_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (nm_sock < 0)
    {
        perror("nm socket error");
        exit(1);
    }

    memset(&nm->nm_addr, 0, sizeof(nm->nm_addr));
    nm->nm_addr.sin_family = AF_INET;
    nm->nm_addr.sin_port = htons(nm->port_to_connect);
    inet_pton(AF_INET, ip, &nm->nm_addr.sin_addr);

    int nm_connection = connect(nm_sock, (struct sockaddr *)&nm->nm_addr, sizeof(nm->nm_addr));
    if (nm_connection < 0)
    {
        perror("nm connection error");
        exit(1);
    }
    printf("nm connected to ss\n");
    // convert port to char
    char port_nm[10];
    sprintf(port_nm, "%d", nm->nm_to_ss);
    combine_with_comma(port_nm, "Connected Naming server to Storage server for delete/create/copy", "Connection done");
    int sending = send(nm_sock, buffer_nm, strlen(buffer_nm), 0);
    if (sending < 0)
    {
        perror("sending");
        exit(1);
    }
    // printf("command sent\n");
    // receive
    bzero(buffer_nm, 1024);
    int recv1 = recv(nm_sock, buffer_nm, 1024, 0);
    if (recv1 < 0)
    {
        perror("receive");
        exit(1);
    }
    strcpy(ack, buffer_nm);
    printf("ack received %s\n", ack);
    printf("nm->flag %d\n", nm->flag);
    char compare_ack_success1[10];

    sprintf(compare_ack_success1, "%d", SUCCESSFUL);
    combine_with_comma(compare_ack_success1, "Acknowledgemenet received", compare_ack_success);

    if (nm->flag == 1)
    {
        // printf("flag ke andar delete wala\n");
        if (strcmp(ack, compare_ack_success) == 0)
        {
            root = deleteNode(root, nm->tmp1);
            printf("%s\n", ack);
            printBFS(root);
            if (nm->stop == 1)
                combine_for_LRU(nm->tmp2, nm->nm_to_ss, nm->clt_to_ss, nm->ss->cwd);
            else
                combine_for_LRU(nm->tmp2, nm->nm_to_ss, nm->clt_to_ss, nm->ss->cwd);
        }
        else
        {
            if (nm->stop == 1)
                combine_for_LRU(nm->look, nm->nm_to_ss, nm->clt_to_ss, nm->ss->cwd);
            else
                combine_for_LRU(nm->temp, nm->nm_to_ss, nm->clt_to_ss, nm->ss->cwd);

            printf("%s\n", ack);
        }
    }
    else if (nm->flag == 0)
    {
        // printf("inside create wala if\n");

        if (strcmp(ack, compare_ack_success) == 0)
        {
            char *insert;
            insert = (char *)malloc(1024 * sizeof(char));
            bzero(insert, 1024);
            insert = combine_with_slash(nm->tmp2, nm->look);

            char *hogya;
            hogya = (char *)malloc(1024 * sizeof(char));
            bzero(hogya, 1024);
            hogya = combine(nm->ss->cwd, insert);

            printf("insert %s\n", insert);
            char **final_nm;
            final_nm = (char **)malloc(1024 * sizeof(char *));
            for (int i = 0; i < 1024; i++)
            {
                final_nm[i] = (char *)malloc(1024 * sizeof(char));
            }
            final_nm = tokenize_final_path(hogya);
            // count tokens
            // printf("final sahi se mil gaya\n");
            int count_nm = 0;
            while (final_nm[count_nm] != NULL)
            {
                count_nm++;
            }
            // printf("count_nm done as %d\n", count_nm);
            for (int i = 0; i < count_nm; i++)
            {
                if (i == 0)
                {
                    root = insertNode(root, NULL, final_nm[i], 0, nm->ss);
                    // printf("inserted at i = 0\n");
                }
                else if (i = count_nm - 1)
                {
                    root = insertNode(root, final_nm[i - 1], final_nm[i], 1, nm->ss);
                    // printf("inserted at i = count_nm-1\n");
                }
                else
                {
                    root = insertNode(root, final_nm[i - 1], final_nm[i], 0, nm->ss);
                    // printf("inserted at other i\n");
                }
            }
            // printf("done ig?\n");
            printBFS(root);
            printf("%s\n", ack);
            // char *to_add = trim(nm->tmp2);
            char *lru_text;
            lru_text = (char *)malloc(1024 * sizeof(char));
            bzero(lru_text, 1024);
            lru_text = combine_with_slash(nm->tmp2, nm->tmp1);
            combine_for_LRU(lru_text, nm->nm_to_ss, nm->clt_to_ss, nm->ss->cwd);
        }
        else
        {
            printf("%s\n", ack);
        }
    }
    else
    {
        // copy
        printf("sorry\n");
    }
    char sending_info2[1024];
    strcpy(sending_info2, ack);
    printf("sending after create/delete/ copy %s\n", sending_info2);
    int sending4 = send(nm->clt_connection, sending_info2, sizeof(sending_info2), 0);
    if (sending4 < 0)
    {
        perror("sending");
        exit(1);
    }
    // convert port to char
    char port_nm1[10];
    sprintf(port_nm1, "%d", nm->clt_connection);
    combine_with_comma(port_nm1, "sending Naming server to client ack", sending_info2);

    return NULL;
}

void *ss_alive(void *arg)
{
    char *buffer_ali_ss;
    buffer_ali_ss = (char *)malloc(1024 * sizeof(char));
    bzero(buffer_ali_ss, 1024);
    // allocate memory to server_info *ss
    server_info *ss = (server_info *)(malloc(sizeof(server_info)));

    ss = (server_info *)arg;
    ss_struct *ss_node = (ss_struct *)(malloc(sizeof(ss_struct)));

    // receive 2 ports
    int recv2 = recv(ss->ss_accepting, buffer_ali_ss, 1024, 0);
    if (recv2 < 0)
    {
        perror("receive");
        exit(1);
    }
    printf("Received ports...\n");
    printf("ports are %s\n", buffer_ali_ss);

    char *token_ali_ss;
    token_ali_ss = (char *)malloc(sizeof(char) * 1024);
    // bzero(token_ali_ss, 1024);
    token_ali_ss = strtok(buffer_ali_ss, " \n");
    printf(" token 1 : %s\n", token_ali_ss);
    ss_node->nm_connect_to_ss = atoi(token_ali_ss);

    token_ali_ss = strtok(NULL, " \n");
    printf(" token 2 : %s\n", token_ali_ss);
    if (strlen(token_ali_ss) < 4)
    {
        printf("so far token is %s\n", token_ali_ss);
        char *new;
        new = (char *)malloc(1024 * sizeof(char));
        bzero(new, 1024);
        // strcpy(new, buffer_ali_ss);
        // printf("new %s\n",new);
        // bzero(buffer_ali_ss, 1024);
        int recv2 = recv(ss->ss_accepting, new, 1024, 0);
        if (recv2 < 0)
        {
            perror("receive");
            exit(1);
        }
        printf("now token is %s\n", token_ali_ss);
        // printf("second recv %s\n", new);
        // token_ali_ss = trim(token_ali_ss);
        // printf(" token 3 : %s\n", token_ali_ss);
        // new = trim(new);
        // printf(" token 3 : %s\n", token_ali_ss);
        // strcat(token_ali_ss, new);
        // strcat without using strcat
        int len = strlen(token_ali_ss);
        printf("length %d and the content %s\n", len, token_ali_ss);
        for (int i = 0; i < strlen(new); i++)
        {
            token_ali_ss[len + i] = new[i];
        }
        printf(" token 2 in if : %s\n", token_ali_ss);
    }
    printf(" token 2 : %s\n", token_ali_ss);
    ss_node->clt_connect_to_ss = atoi(token_ali_ss);
    printf("nm_to_ss_port: %d\n", ss_node->nm_connect_to_ss);
    printf("clt_to_ss_port: %d\n", ss_node->clt_connect_to_ss);

    printf("clt_to_ss_port 2: %d\n", ss_node->clt_connect_to_ss);

    // convert to char
    char name[20];
    sprintf(name, "%d", ss_node->nm_connect_to_ss);
    combine_with_comma(name, "port received from ss", "receive successful");

    // assigning unique id as nm wala port
    ss_node->ss_id = ss_node->nm_connect_to_ss;

    // receive to get the cwd from ss
    bzero(buffer_ali_ss, 1024);
    // printf("okok\n");
    printf("%s\n", buffer_ali_ss);
    char *new_buf;
    new_buf = (char *)malloc(1024 * sizeof(char));
    // bzero(new_buf, 1024);
    memset(new_buf, '\0', 1024);
    int flag = 0;
    while (flag == 0)
    {
        int recv3 = recv(ss->ss_accepting, new_buf, 1024, 0);
        if (recv3 < 0)
        {
            perror("receive");
            exit(1);
        }

        if (strlen(new_buf) > 0)
        {
            flag = 1;
            printf("ab aaya %s\n", new_buf);
        }
        else
            printf("empty\n");
    }
    printf("Received cwd...\n");
    printf("%s\n", new_buf);
    // char *cwd_ali_ss = (char *)(malloc(sizeof(char) * 1024));
    char cwd_ali_ss[1024];
    // bzero(cwd_ali_ss, 1024);
    printf("ok \n");
    strcpy(cwd_ali_ss, new_buf);
    printf("cwd: %s\n", cwd_ali_ss);
    ss_node->cwd = cwd_ali_ss;
    // strcpy(ss_node->cwd, cwd_ali_ss);
    printf("Location of ss with ss_id %d: %s\n", ss_node->ss_id, ss_node->cwd);
    combine_with_comma(ss_node->cwd, "cwd of ss", "SUCCESSFUL");

    // store number of paths for that ss number
    bzero(buffer_ali_ss, 1024);
    int recv4 = recv(ss->ss_accepting, buffer_ali_ss, 1024, 0); // Remove this line from ss->ss_accepting
    if (recv4 < 0)
    {
        perror("receive");
        exit(1);
    }

    ss_node->path_count = atoi(buffer_ali_ss);
    printf("%d\n", ss_node->path_count);
    // printf("\n");
    // IMPLEMENT TRIES USING CWD + FILEPATHS
    // send  path and struct

    // pthread_mutex_lock(&mutex_tries);
    sem_wait(&ss_semaphore);
    for (int i = 0; i < ss_node->path_count; i++)
    {
        char *new_buf_for_paths;
        new_buf_for_paths = (char *)malloc(1024 * sizeof(char));
        bzero(new_buf_for_paths, 1024);
        int flag_path = 0;
        // while (flag_path == 0)
        // {
        int receive = recv(ss->ss_accepting, new_buf_for_paths, 1024, 0);
        if (receive < 0)
        {
            perror("receive");
            exit(1);
        }
        //     if (strlen(new_buf_for_paths) > 0)
        //     {
        //         flag_path = 1;
        //         printf("ab aaya %s\n", new_buf_for_paths);
        //     }
        //     else
        //     {
        //         printf("empty\n");
        //     }
        // }
        // while (flag_path == 0)
        // {
        //     int receive = recv(ss->ss_accepting, new_buf_for_paths, 1024, 0);
        //     if (receive < 0)
        //     {
        //         perror("receive");
        //         exit(1);
        //     }
        //     else if (strlen(new_buf_for_paths) > 0)
        //     {
        //         flag_path = 1;
        //         new_buf_for_paths[receive] = '\0'; // Null-terminate the received data
        //         printf("ab aaya %s\n", new_buf_for_paths);
        //     }
        //     else
        //     {
        //         printf("empty\n");
        //     }
        // }
        printf("Received path %d...\n", i + 1);
        printf("%s\n", new_buf_for_paths);
        // malloc cwd_path

        char *cwd_path = (char *)malloc(sizeof(char) * 1024);
        cwd_path = combine(ss_node->cwd, new_buf_for_paths);
        printf("%s\n", cwd_path);
        char **add = (char **)malloc(sizeof(char *) * 512);
        for (int wtf = 0; wtf < 512; wtf++)
        {
            add[wtf] = (char *)malloc(sizeof(char) * 1024);
        }
        add = tokenize_final_path(cwd_path);
        int tokenIndex = 0; // Rename the variable to avoid conflict with the outer loop

        while (add[tokenIndex] != NULL)
        {
            // printf("%s\n", add[tokenIndex]);
            if (tokenIndex == 0 && add[tokenIndex + 1] != NULL)
            {
                // printf("home\n");
                root = insertNode(root, NULL, add[tokenIndex], 0, ss_node);
            }
            else if (tokenIndex != 0 && add[tokenIndex + 1] != NULL)
            {
                // printf("intermediate\n");
                // printf("%s...\n", add[tokenIndex - 1]);
                root = insertNode(root, add[tokenIndex - 1], add[tokenIndex], 0, ss_node);
            }
            else if (tokenIndex != 0 && add[tokenIndex + 1] == NULL)
            {
                // printf("last\n");
                root = insertNode(root, add[tokenIndex - 1], add[tokenIndex], 1, ss_node);
            }
            else
            {
                printf("How is this even possible when the token is NULL and the next token is obviously NULL\n");
            }
            // printf("%s\n", add[tokenIndex]);

            tokenIndex++;
        }
        // free array of strings
        // for (int wtff = 0; wtff < 512; wtff++)
        // {
        //     free(add[wtff]);
        // }
        // free(add);
        // bzero(new_buf_for_paths, 1024);
        // free(new_buf_for_paths);
        // free(cwd_path);
    }
    sem_post(&ss_semaphore);
    char *name2;
    name2 = (char *)malloc(1024 * sizeof(char));
    sprintf(name2, "%d", ss_node->path_count);
    combine_with_comma(name2, "files sent by ss", "RECEIVED SUCCESSLLY");

    // pthread_mutex_unlock(&mutex_ss);
    // printf("Teri maa \n");
    // delete node
    printf("printing dfs...\n");
    print_trie(root);
    printf("\n");
    printf("printing bfs...\n");
    printBFS(root);

    // root = copy_subtree(root, "f", "d", ss_node);
    // printf("printing dfs...\n");
    // print_trie(root);
    // printf("\n");
    // printf("printing bfs...\n");
    // printBFS(root);

    // root = deleteNode(root, "b");
    // printf("printing dfs...\n");
    // print_trie(root);
    // printf("\n");
    // printf("printing bfs...\n");
    // printBFS(root);

    // root = shift_subtree(root, "d", "c");
    // printf("printing dfs...\n");
    // print_trie(root);
    // printf("\n");
    // printf("printing bfs...\n");
    // printBFS(root);

    // Node ok = NULL;
    // ok = search(root, "a.txt");
    // printf("%s\n", ok->name);

    // struct timeval timeout;
    // timeout.tv_sec = 3;
    // timeout.tv_usec = 100;
    // if (setsockopt(ss->ss_accepting, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
    // {
    //     perror("setsockopt:");
    //     exit(1);
    // }
    // while (1)
    // {
    //     // printf("Checking SS Connection %d\n", ss_node->ss_id);
    //     bzero(buffer_ali_ss, 1024);
    //     int receive = recv(ss->ss_accepting, buffer_ali_ss, sizeof(buffer_ali_ss), 0);
    //     if (receive < 0)
    //     {
    //         if (errno != EAGAIN || errno != EWOULDBLOCK)
    //         {
    //             perror("receive");
    //             exit(1);
    //         }
    //         printf("SS %d is dead\n", ss_node->ss_id);
    //         break;
    //     }
    //     printf("buffer_ali_ss: %s\n", buffer_ali_ss);
    //     if (atoi(buffer_ali_ss) == 0)
    //     {
    //         // remove from trie
    //         printf("SS %d is dead\n", ss_node->ss_id);
    //         break;
    //     }
    //     else
    //     {
    //         printf("SS %d is alive\n", ss_node->ss_id);
    //     }
    // }
    return NULL;
}

void *multi_client(void *arg)
{
    printf("thread created for client\n");
    char *buffer_clt;
    buffer_clt = (char *)malloc(sizeof(char) * 1024);
    bzero(buffer_clt, 1024);
    client_info *clt;
    // malloc
    clt = (client_info *)malloc(sizeof(client_info));
    clt = (client_info *)arg;
    // timeout ack
    char *port3;
    // malloc
    port3 = (char *)(malloc(sizeof(char) * 10));
    // convert port to char in port3
    sprintf(port3, "%d", clt->client_port);
    // combine_with_comma(port3, "client connection", "SUCCESSFUL");

    sprintf(buffer_clt, "%d", CONNECT_CLT_TO_NM);
    int sending = send(clt->clt_connection, buffer_clt, sizeof(buffer_clt), 0);
    if (sending < 0)
    {
        perror("send");
        exit(1);
    }
    printf("sent ack to the client\n");
    while (1)
    {

        bzero(buffer_clt, 1024);
        char *course;
        // malloc
        course = (char *)malloc(sizeof(char) * 1024);
        bzero(course, 1024);

        int receive2 = recv(clt->clt_connection, buffer_clt, 1024, 0);
        if (receive2 < 0)
        {
            if (errno == 104)
            {
                break;
            }
            perror("receive");
            exit(1);
        }
        strcpy(course, buffer_clt);
        char *course1;
        // malloc
        course1 = (char *)malloc(sizeof(char) * 1024);
        // bdzero
        bzero(course1, 1024);
        course1 = trim(course);
        combine_with_comma(port3, "command received", course1);

        // printf("buffer_clt %s...\n", buffer_clt);
        // printf("course %s...\n", course1);
        bzero(buffer_clt, 1024);
        strcpy(buffer_clt, course1);
        // split the string into tokens
        char **tokens;
        tokens = (char **)malloc(sizeof(char *) * 5);
        for (int i = 0; i < 5; i++)
        {
            tokens[i] = malloc(sizeof(char) * 1024);
        }
        char *used_to_tokenize;
        used_to_tokenize = (char *)malloc(sizeof(char) * 1024);
        used_to_tokenize = strtok(buffer_clt, " ");
        int i = 0;
        while (used_to_tokenize != NULL)
        {
            strcpy(tokens[i], used_to_tokenize);
            used_to_tokenize = strtok(NULL, " ");
            i++;
        }

        if (strcmp(tokens[0], "create") == 0)
        {
            // create dir/file name path

            char *tmp1;
            // malloc
            tmp1 = (char *)malloc(sizeof(char) * 1024);
            tmp1 = trim(tokens[3]);
            char *tmp2;
            // malloc
            tmp2 = (char *)malloc(sizeof(char) * 1024);
            tmp2 = trim(tokens[2]);
            Node find = NULL;
            int port_to_send;
            char *search_for_it;
            // malloc
            search_for_it = (char *)malloc(sizeof(char) * 1024);
            search_for_it = trim(tokens[3]);
            int flag = -1;
            int searched = 0;
            // char **found = search_and_extract(search_for_it);

            find = search(root, search_for_it);
            if (find != NULL)
            { // not null
                flag = 0;
                printf("printing name of the node found... %s\n", find->name);
            }
            else
            {
                flag = 1;
                printf("Node not found.\n");
            }

            // printf("send info to clt to connect to ss to %s %s having path %s\n", tokens[0], tokens[1], tokens[2]);
            if (flag == 1)
            {
                // not found
                port_to_send = 2;
                char *sending_info;
                // malloc
                sending_info = (char *)malloc(sizeof(char) * 1024);
                bzero(sending_info, 1024);
                sprintf(sending_info, "%d", port_to_send);
                int sending = send(clt->clt_connection, sending_info, sizeof(sending_info), 0);
                if (sending < 0)
                {
                    perror("sending");
                    exit(1);
                }
                printf("Node not found\n");
            }
            else
            {
                // printf("entered if\n");
                if (find->give_access)
                {
                    // printf("entered if 2\n");

                    // printf("entered if 4\n");

                    port_to_send = find->ss->nm_connect_to_ss;
                    printf("%d the port for nm to ss\n", find->ss->nm_connect_to_ss);

                    naming_info *nm = (naming_info *)(malloc(sizeof(naming_info)));
                    nm->port_to_connect = port_to_send;
                    strcpy(nm->command, course);
                    nm->nm_sock = clt->client_sock;
                    nm->nm_addr = clt->client_addr;
                    nm->nm_size = clt->client_size;
                    nm->clt_connection = clt->clt_connection;
                    nm->flag = 0;
                    // printf("okok 1\n");
                    bzero(nm->tmp1, 1024);
                    bzero(nm->tmp2, 1024);
                    bzero(nm->temp, 1024);
                    bzero(nm->look, 1024);
                    strcpy(nm->tmp1, search_for_it);
                    strcpy(nm->tmp2, tmp2);
                    strcpy(nm->look, tmp1);
                    // nm->temp = temp;
                    // nm->stop = stop;
                    nm->searched = searched;
                    // nm->ss = NULL;
                    nm->ss = malloc(sizeof(struct ss_id));

                    nm->ss->ss_id = 0;

                    // printf("okok 2\n");

                    nm->ss->cwd = malloc(sizeof(char) * 1024);
                    bzero(nm->ss->cwd, 1024);
                    strcpy(nm->ss->cwd, find->ss->cwd);

                    nm->ss->path_count = 0;
                    nm->ss->nm_connect_to_ss = port_to_send;
                    // printf("okok 3\n");

                    nm->ss->clt_connect_to_ss = find->ss->clt_connect_to_ss;

                    nm->nm_to_ss = port_to_send;

                    nm->clt_to_ss = find->ss->clt_connect_to_ss;
                    // printf("okok 3\n");
                    pthread_t *nm_thread;
                    nm_thread = malloc(sizeof(pthread_t));
                    if (pthread_create(nm_thread, NULL, nm_connection, nm) != 0)
                    {
                        perror("Thread creation error");
                        exit(1);
                    }

                    if (pthread_join(*nm_thread, NULL) != 0)
                    {
                        perror("join");
                        exit(1);
                    }
                    // combine_for_LRU(combined, find->ss->nm_connect_to_ss, find->ss->clt_connect_to_ss, find->ss->cwd);
                    combine_for_LRU(search_for_it, find->ss->nm_connect_to_ss, find->ss->clt_connect_to_ss, find->ss->cwd);
                }
                else
                {
                    port_to_send = 5;
                    printf("access denied\n");
                    char *sending_info5;
                    // malloc
                    sending_info5 = (char *)malloc(sizeof(char) * 1024);
                    bzero(sending_info5, 1024);
                    sprintf(sending_info5, "%d", port_to_send);
                    int sending1 = send(clt->clt_connection, sending_info5, sizeof(sending_info5), 0);
                    if (sending1 < 0)
                    {
                        perror("sending");
                        exit(1);
                    }
                }
            }
        }
        else if (strcmp(tokens[0], "delete") == 0)
        {
            // delete dir/file name /path
            Node find = NULL;
            char *tmp1;
            // malloc
            tmp1 = (char *)malloc(sizeof(char) * 1024);
            tmp1 = trim(tokens[2]);
            // printf(" tmp1 %s\n", tmp1);
            char *tmp2;
            // malloc
            tmp2 = (char *)malloc(sizeof(char) * 1024);
            tmp2 = trim(tokens[3]);
            // printf(" tmp2 %s\n", tmp2);
            int searched1 = 0;
            int stop = 0;
            int flag = -1;
            char **found1;
            found1 = (char **)malloc(sizeof(char *) * 1024);
            for (int i = 0; i < 1024; i++)
            {
                found1[i] = (char *)malloc(sizeof(char) * 1024);
            }
            char *look;
            // malloc
            look = (char *)malloc(sizeof(char) * 1024);
            look = combine_with_slash(tmp1, tmp2);
            char *temp;
            // malloc
            temp = (char *)malloc(sizeof(char) * 1024);
            temp = (char *)malloc(sizeof(char) * 1024);
            int port_to_send;

            // found1 = (char **)malloc(sizeof(char *) * 1024);
            if (strcmp(tmp2, "/") != 0)
            {
                // char **found1 = search_and_extract(look);
                // printf("debug madarchod\n");
                stop = 1;
                char **found;
                // malloc
                found = (char **)malloc(sizeof(char *) * 1024);
                for (int i = 0; i < 1024; i++)
                {
                    found[i] = (char *)malloc(sizeof(char) * 1024);
                }
                found = search_and_extract(look);
                if (found != NULL)
                {
                    printf("found in LRU\n");
                    // print found tokens
                    for (int i = 0; i <= 3; i++)
                    {
                        printf("%s\n", found[i]);
                    }
                }
                else
                {
                    printf("not found in LRU\n");
                }
                find = search(root, look);
                if (find != NULL)
                {
                    flag = 0;
                    printf("printing name of the node found... %s\n", find->name);
                }
                else
                {
                    flag = 1;
                    printf("Node not found.\n");
                }
            }
            else
            {
                strcpy(temp, "/");
                strcat(temp, tmp1);
                // char **found1 = search_and_extract(temp);
                // if (found1 == NULL)
                // {
                char **found;
                // malloc
                found = (char **)malloc(sizeof(char *) * 1024);
                for (int i = 0; i < 1024; i++)
                {
                    found[i] = (char *)malloc(sizeof(char) * 1024);
                }
                found = search_and_extract(temp);
                if (found != NULL)
                {
                    printf("found in LRU\n");
                    // print found tokens
                    for (int i = 0; i <= 3; i++)
                    {
                        printf("%s\n", found[i]);
                    }
                }
                else
                {
                    printf("not found in LRU\n");
                }
                find = search(root, look);
                if (find != NULL)
                {
                    flag = 0;
                    printf("printing name of the node found... %s\n", find->name);
                }
                else
                {
                    flag = 1;
                    printf("Node not found.\n");
                }
            }

            if (flag == 1)
            {
                // not found
                port_to_send = 2;
                char *sending_info;
                // malloc
                sending_info = (char *)malloc(sizeof(char) * 1024);
                bzero(sending_info, 1024);
                sprintf(sending_info, "%d", port_to_send);
                int sending3 = send(clt->clt_connection, sending_info, sizeof(sending_info), 0);
                if (sending3 < 0)
                {
                    perror("sending");
                    exit(1);
                }
            }
            else // flag 0 or -1
            {
                if (find->give_access)
                {

                    port_to_send = find->ss->nm_connect_to_ss;

                    naming_info *nm = (naming_info *)(malloc(sizeof(naming_info)));
                    nm->port_to_connect = port_to_send;
                    strcpy(nm->command, course);
                    nm->nm_sock = clt->client_sock;
                    nm->nm_addr = clt->client_addr;
                    nm->nm_size = clt->client_size;
                    nm->clt_connection = clt->clt_connection;
                    nm->flag = 1;
                    bzero(nm->tmp1, 1024);
                    bzero(nm->tmp2, 1024);
                    bzero(nm->temp, 1024);
                    bzero(nm->look, 1024);
                    strcpy(nm->tmp1, tmp1);
                    strcpy(nm->tmp2, tokens[2]);
                    strcpy(nm->look, tokens[3]);
                    strcpy(nm->temp, temp);
                    nm->stop = stop;
                    nm->nm_to_ss = port_to_send;

                    nm->clt_to_ss = find->ss->clt_connect_to_ss;

                    nm->ss = malloc(sizeof(struct ss_id));
                    nm->ss->nm_connect_to_ss = port_to_send;
                    nm->ss->clt_connect_to_ss = nm->clt_to_ss;

                    nm->ss->cwd = find->ss->cwd;

                    nm->ss->path_count = 0;

                    pthread_t nm_thread;
                    if (pthread_create(&nm_thread, NULL, nm_connection, nm) != 0)
                    {
                        perror("Thread creation error");
                        exit(1);
                    }
                    if (pthread_join(nm_thread, NULL) != 0)
                    {
                        perror("join");
                        exit(1);
                    }
                    combine_for_LRU(tmp2, find->ss->nm_connect_to_ss, find->ss->clt_connect_to_ss, find->ss->cwd);
                }
                else
                {
                    port_to_send = 5;
                    printf("access denied\n");
                    char *sending_info3;
                    // malloc
                    sending_info3 = (char *)malloc(sizeof(char) * 1024);
                    bzero(sending_info3, 1024);
                    sprintf(sending_info3, "%d", port_to_send);
                    int sending4 = send(clt->clt_connection, sending_info3, sizeof(sending_info3), 0);
                    if (sending4 < 0)
                    {
                        perror("sending");
                        exit(1);
                    }
                }
            }
        }
        else if (strcmp(tokens[0], "copy") == 0)
        {
            // token 0 copy
            // token 1 dir/file
            // token 2 source
            // token 3 destination
            printf("We started late so couldn't do this part\n lmao made it easy for the TA \n");
            // find the struct and pass to the copy_subtree
            printf("send command to ss to %s a new %s having path %s\n", tokens[0], tokens[1], tokens[2]);
        }
        else if (strcmp(tokens[0], "read") == 0 || strcmp(tokens[0], "write") == 0 || strcmp(tokens[0], "get") == 0)
        { // token 0 copy
            // token 1 name
            // token 2 path
            Node find = NULL;
            int port_to_send;
            int flag = -1;
            int stop = 0;
            char *tmp1;
            // malloc
            tmp1 = (char *)malloc(sizeof(char) * 1024);
            tmp1 = trim(tokens[1]);
            char *tmp;
            // malloc
            tmp = (char *)malloc(sizeof(char) * 1024);
            tmp = trim(tokens[2]);
            char *look;
            // malloc
            look = (char *)malloc(sizeof(char) * 1024);
            look = combine_with_slash(tmp1, tmp);
            char *temp;
            // malloc
            temp = (char *)malloc(sizeof(char) * 1024);
            temp = (char *)malloc(sizeof(char) * 100);
            char **found3;
            // malloc
            found3 = (char **)malloc(sizeof(char *) * 1024);
            for (int i = 0; i < 1024; i++)
            {
                found3[i] = (char *)malloc(sizeof(char) * 1024);
            }
            int searched3 = 0;
            if (strcmp(tmp, "/") != 0)
            {
                stop = 1;
                // found3 = search_and_extract(look);
                char **found;
                // malloc
                found = (char **)malloc(sizeof(char *) * 1024);
                for (int i = 0; i < 1024; i++)
                {
                    found[i] = (char *)malloc(sizeof(char) * 1024);
                }
                found = search_and_extract(look);
                if (found != NULL)
                {
                    printf("found in LRU\n");
                    // print found tokens
                    for (int i = 0; i <= 3; i++)
                    {
                        printf("%s\n", found[i]);
                    }
                }
                else
                {
                    printf("not found in LRU\n");
                }
                find = search(root, look);
                if (find != NULL)
                {
                    flag = 0;
                    printf("printing name of the node found... %s\n", find->name);
                }
                else
                {
                    flag = 1;
                    printf("Node not found.\n");
                }
            }
            else
            {

                strcpy(temp, "/");
                strcat(temp, tmp1);
                char **found;
                // malloc
                found = (char **)malloc(sizeof(char *) * 1024);
                for (int i = 0; i < 1024; i++)
                {
                    found[i] = (char *)malloc(sizeof(char) * 1024);
                }
                found = search_and_extract(temp);
                if (found != NULL)
                {
                    printf("found in LRU\n");
                    // print found tokens
                    for (int i = 0; i <= 3; i++)
                    {
                        printf("%s\n", found[i]);
                    }
                }
                else
                {
                    printf("not found in LRU\n");
                }
                find = search(root, temp);

                if (find != NULL)
                {
                    flag = 0;
                    printf("printing name of the node found... %s\n", find->name);
                }
                else
                {
                    flag = 1;

                    printf("Node not found.\n");
                }
            }

            if (flag == 1)
            {
                // not found
                port_to_send = 2;
                char sending_info[1024];
                bzero(sending_info, 1024);
                sprintf(sending_info, "%d", port_to_send);
                int sending = send(clt->clt_connection, sending_info, sizeof(sending_info), 0);
                if (sending < 0)
                {
                    perror("sending");
                    exit(1);
                }

                // combine_with_comma(port_to_send, course, "NOT FOUND");
            }
            else
            {
                if (find->give_access)
                {
                    if (stop == 1)
                    {
                        combine_for_LRU(look, find->ss->nm_connect_to_ss, find->ss->clt_connect_to_ss, find->ss->cwd);
                    }
                    else
                    {
                        combine_for_LRU(temp, find->ss->nm_connect_to_ss, find->ss->clt_connect_to_ss, find->ss->cwd);
                    }

                    port_to_send = find->ss->clt_connect_to_ss;

                    char sending_info[1024];
                    bzero(sending_info, 1024);
                    sprintf(sending_info, "%d", port_to_send);
                    printf("sending sending info %s\n", sending_info);
                    printf("sizeof sending_info = %ld\n", sizeof(sending_info));
                    int sending = send(clt->clt_connection, sending_info, sizeof(sending_info), 0);
                    printf("sent sending info %s\n", sending_info);
                    if (sending < 0)
                    {
                        perror("sending");
                        exit(1);
                    }

                    // get ack from clt
                    char *ack1;
                    // malloc
                    ack1 = (char *)malloc(sizeof(char) * 1024);
                    int ack1_recv = recv(clt->clt_connection, ack1, 1024, 0);
                    if (ack1_recv < 0)
                    {
                        if (errno == 104)
                        {
                            break;
                        }
                        perror("ack1 recv");
                        exit(1);
                    }

                    // // combine_with_comma(port_to_send, course, ack_recv);
                }
                else
                {
                    port_to_send = 5;
                    printf("access denied\n");
                    char *sending_info;
                    // malloc
                    sending_info = (char *)malloc(sizeof(char) * 1024);
                    bzero(sending_info, 1024);
                    sprintf(sending_info, "%d", port_to_send);
                    int sending = send(clt->clt_connection, sending_info, sizeof(sending_info), 0);
                    if (sending < 0)
                    {
                        perror("sending");
                        exit(1);
                    }
                    if (stop == 1)
                    {
                        combine_for_LRU(look, find->ss->nm_connect_to_ss, find->ss->clt_connect_to_ss, find->ss->cwd);
                    }
                    else
                    {
                        combine_for_LRU(temp, find->ss->nm_connect_to_ss, find->ss->clt_connect_to_ss, find->ss->cwd);
                    }

                    // combine_with_comma(port_to_send, course, "ACCESS DENIED");
                }
            }
        }
        else
        {
            // printf("ok bye 1 \t%d\n", errno);
            printf("invalid command\n");
            int will = 1;
            char *sending_info;
            // malloc
            sending_info = (char *)malloc(sizeof(char) * 1024);
            bzero(sending_info, 1024);
            sprintf(sending_info, "%d", will);
            int sending = send(clt->clt_connection, sending_info, sizeof(sending_info), 0);
            if (sending < 0)
            {
                perror("sending");
                exit(1);
            }
            // combine_with_comma(0buffer
        }
        printf("Waiting for another command...\n");
    }
}

void *ss_connection(void *arg)
{
    // printf("Storage server thread created\n");
    server_info *ss = (server_info *)arg;

    // Binding, Listening, and Accepting connections
    if (bind(ss->ss_sock, (struct sockaddr *)&ss->ss_addr, sizeof(ss->ss_addr)) < 0)
    {
        perror("Binding");
        exit(1);
    }
    if (listen(ss->ss_sock, 1) < 0)
    {
        perror("Listening");
        exit(1);
    }
    printf("nm listening to ss...\n");
    int check = sem_init(&ss_semaphore, 0, 1);
    if (check != 0)
    {
        perror("sem_init");
        exit(1);
    }
    while (1)
    {
        ss->ss_accepting = accept(ss->ss_sock, (struct sockaddr *)&ss->nm_addr, &ss->ss_size);
        if (ss->ss_accepting < 0)
        {
            perror("Accepting");
            exit(1);
        }

        pthread_mutex_lock(&mutex_ss);
        ss_count_for_nm++;
        pthread_mutex_unlock(&mutex_ss);
        // combine_with_comma(1234, "initialization of ss", "CONNECTION SUCCESSFUL");

        // create thread for ss
        pthread_t ss_alive_thread;
        pthread_create(&ss_alive_thread, NULL, ss_alive, (void *)ss);

        // if (pthread_join(ss_alive_thread, NULL))
        // {
        //     perror("pthread_join");
        //     exit(1);
        // }
    }
}

void *clt_connection(void *arg)
{
    client_info *clt = (client_info *)arg;
    if (bind(clt->client_sock, (struct sockaddr *)&clt->client_addr, sizeof(clt->client_addr)) < 0)
    {
        perror("Binding");
        exit(1);
    }
    if (listen(clt->client_sock, 1) < 0)
    {
        perror("Listening");
        exit(1);
    }
    printf("nm listening to client...\n");
    while (1)
    {
        clt->clt_connection = accept(clt->client_sock, (struct sockaddr *)&clt->nm_addr, &clt->client_size);
        if (clt_connection < 0)
        {
            perror("Accepting");
            exit(1);
        }
        char port_clt_1[10];
        strcpy(port_clt_1, "2345");
        combine_with_comma(port_clt_1, "initialization of CLIENT", "CONNECTION SUCCESSFUL");
        pthread_t clt_multi;
        pthread_create(&clt_multi, NULL, multi_client, (void *)clt);
    }
}

int main()
{

    // keeping a fix port for nm
    int port1 = 1234;
    int port2 = 2345;
    int SERVER_IS_CLT, CLT_IS_CLT;
    struct sockaddr_in SER_ADDR, CLT_ADDR;
    socklen_t SER_SIZE, CLT_SIZE;

    char buffer[1024];
    bzero(buffer, 1024);

    SERVER_IS_CLT = socket(AF_INET, SOCK_STREAM, 0);
    if (SERVER_IS_CLT < 0)
    {
        perror("1. server socket error");
        exit(1);
    }
    printf("Server socket done!\n");

    CLT_IS_CLT = socket(AF_INET, SOCK_STREAM, 0);
    if (CLT_IS_CLT < 0)
    {
        perror("2. client socket error");
        exit(1);
    }
    // printf("Client socket done!\n");

    // initialize storage server
    memset(&SER_ADDR, 0, sizeof(SER_ADDR));
    SER_ADDR.sin_family = AF_INET;
    SER_ADDR.sin_port = htons(port1);
    // printf("%d\n",SER_ADDR.sin_port);
    // SER_ADDR.sin_port = 0;
    inet_pton(AF_INET, ip, &SER_ADDR.sin_addr);

    // initialize client
    memset(&CLT_ADDR, 0, sizeof(CLT_ADDR));
    CLT_ADDR.sin_family = AF_INET;
    CLT_ADDR.sin_port = htons(port2);
    // CLT_ADDR.sin_port = 0;
    inet_pton(AF_INET, ip, &CLT_ADDR.sin_addr);

    // take addresses
    SER_SIZE = sizeof(struct sockaddr_in);
    CLT_SIZE = sizeof(struct sockaddr_in);

    // take values for structs
    // server struct
    server_info *ss = (server_info *)(malloc(sizeof(server_info)));
    ss->ss_sock = SERVER_IS_CLT;
    ss->ss_addr = SER_ADDR;
    ss->ss_size = SER_SIZE;
    ss->server_port = port1;
    // ss->server_port = getsockname(SERVER_IS_CLT, (struct sockaddr *)(&SER_ADDR), &SER_SIZE);
    if (ss->server_port < 0)
    {
        perror("getsockname error");
        exit(1);
    }

    // client struct
    client_info *clt = (client_info *)(malloc(sizeof(client_info)));
    clt->client_sock = CLT_IS_CLT;
    clt->client_addr = CLT_ADDR;
    clt->client_size = CLT_SIZE;
    clt->client_port = port2;
    // clt->client_port = getsockname(CLT_IS_CLT, (struct sockaddr *)(&CLT_ADDR), &CLT_SIZE);
    if (clt->client_port < 0)
    {
        perror("getsockname error");
        exit(1);
    }

    pthread_t ss_thread;
    pthread_t clt_thread;
    FILE *fp1;
    fp1 = fopen("LRU.txt", "w");
    if (fp1 == NULL)
    {
        perror("File opening error");
        exit(1);
    }
    fclose(fp1);
    FILE *fp2;
    fp2 = fopen("bookkeeping.txt", "w");
    if (fp2 == NULL)
    {
        perror("File opening error");
        exit(1);
    }
    fclose(fp2);
    if (pthread_create(&ss_thread, NULL, ss_connection, ss) != 0)
    {
        perror("Thread creation error");
        exit(1);
    }

    if (pthread_create(&clt_thread, NULL, clt_connection, clt) != 0)
    {
        perror("Thread creation error");
        exit(1);
    }

    if (pthread_join(ss_thread, NULL) < 0)
    {
        perror("Thread join error");
        exit(1);
    }
    if (pthread_join(clt_thread, NULL) < 0)
    {
        perror("Thread join error");
        exit(1);
    }

    close(SERVER_IS_CLT);
    close(CLT_IS_CLT);
    return 0;
}