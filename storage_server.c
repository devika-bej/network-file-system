#include "linkedlist.h"

typedef struct ss_info ss_info;
typedef struct clt_comm_info clt_comm_info;
typedef struct nm_comm_info nm_comm_info;

sem_t read_ll_lock, write_ll_lock;
node *read_ll;
node *write_ll;

struct ss_info
{
    int ss_nm_socket, ss_clt_socket, nm_ss_socket;
    struct sockaddr_in ss_nm_add, ss_clt_add, nm_ss_add;
    socklen_t ss_nm_add_size, ss_clt_add_size, nm_ss_add_size;
    int nm_port, clt_port, ss_port;
    int n_path;
    char **paths;
};

struct clt_comm_info
{
    char buf[1024];
    int ss_clt_socket;
    int clt_socket;
};

struct nm_comm_info
{
    char buf[1024];
    int ss_nm_socket;
    int nm_socket;
};

void *ss_to_nm(void *arg)
{
    ss_info info = *(ss_info *)(arg);
    if (connect(info.nm_ss_socket, (struct sockaddr *)(&info.nm_ss_add), sizeof(info.nm_ss_add)) < 0)
    {
        perror("Connect");
        exit(1);
    }

    char buf[1024];

    // send ports
    bzero(buf, 1024);
    sprintf(buf, "%d %d", info.nm_port, info.clt_port);
    if (send(info.nm_ss_socket, buf, 1024, 0) < 0)
    {
        perror("Send");
        exit(1);
    }

    // send current working directory, because paths are relative to this
    bzero(buf, 1024);
    getcwd(buf, 1023);
    if (send(info.nm_ss_socket, buf, 1024, 0) < 0)
    {
        perror("Send");
        exit(1);
    }

    // sending number of paths and then the paths
    bzero(buf, 1024);
    sprintf(buf, "%d", info.n_path);
    if (send(info.nm_ss_socket, buf, 1024, 0) < 0)
    {
        perror("Send");
        exit(1);
    }

    for (int i = 1; i <= info.n_path; ++i)
    {
        bzero(buf, 1024);
        sprintf(buf, "%s", info.paths[i]);
        if (send(info.nm_ss_socket, buf, 1024, 0) < 0)
        {
            perror("Send");
            exit(1);
        }
        // printf("sent path %s\n", buf);
    }

    // while (1)
    // {
    //     bzero(buf, 1024);
    //     sprintf(buf, "1");
    //     if (send(info.nm_ss_socket, buf, 1024, 0) < 0)
    //     {
    //         perror("Send");
    //         exit(1);
    //     }
    // }

    return NULL;
}

void send_file(char *path, char *filename, int nm_socket)
{
    char *buffer = (char *)(malloc(1024 * sizeof(char)));
    bzero(buffer, 1024);
    sprintf(buffer, "%s", filename);
    if (send(nm_socket, buffer, sizeof(buffer), 0) < 0)
    {
        perror("Send");
        exit(1);
    }

    char *dirnm = (char *)(malloc(512 * sizeof(char)));
    getcwd(dirnm, 512);
    char *abs_path = (char *)(malloc(1024 * sizeof(char)));
    strcpy(abs_path, dirnm);
    strcat(abs_path, path);
    chdir(path);

    FILE *fp = fopen(filename, "r");
    bzero(buffer, 1024);
    while (fgets(buffer, 1023, fp) != NULL)
    {
        if (send(nm_socket, buffer, sizeof(buffer), 0) < 0)
        {
            perror("Send");
            exit(1);
        }
        bzero(buffer, 1024);
    }
    fclose(fp);
    bzero(buffer, 1024);
    sprintf(buffer, "%d", DATA_END);
    if (send(nm_socket, buffer, sizeof(buffer), 0) < 0)
    {
        perror("Send");
        exit(1);
    }

    chdir(dirnm);
}

void recv_file(char *path, int nm_socket)
{
    char *buffer = (char *)(malloc(1024 * sizeof(char)));
    bzero(buffer, 1024);
    if (recv(nm_socket, buffer, 1024, 0) < 0)
    {
        perror("Receive");
        exit(1);
    }
    char *filename = (char *)(malloc(512 * sizeof(char)));
    char *label = "_copy\0";
    strcpy(filename, buffer);
    char *pos = strchr(filename, '.');
    if (pos == NULL)
        strcat(filename, label);
    else
    {
        int idx = pos - filename;
        bzero(filename, 512);
        strncpy(filename, buffer, idx);
        strncpy(filename + idx, label, sizeof(label));
        strncpy(filename + idx + sizeof(label), buffer + idx, sizeof(buffer) - idx);
    }

    char *dirnm = (char *)(malloc(512 * sizeof(char)));
    getcwd(dirnm, 512);
    char *abs_path = (char *)(malloc(1024 * sizeof(char)));
    strcpy(abs_path, dirnm);
    strcat(abs_path, path);
    chdir(path);

    FILE *fp = fopen(filename, "w");
    bzero(buffer, 1024);
    while (1)
    {
        bzero(buffer, 1024);
        if (recv(nm_socket, buffer, 1024, 0) < 0)
        {
            perror("Receive");
            exit(1);
        }
        char ack[5];
        bzero(ack, 5);
        sprintf(ack, "%d", DATA_END);
        if (strcmp(ack, buffer) == 0)
            break;
        fprintf(fp, "%s", buffer);
    }
    fclose(fp);

    chdir(dirnm);
}

void *nm_comm_func(void *arg)
{
    nm_comm_info info = *(nm_comm_info *)arg;
    // tokenizing the command from nm
    char *toks[6];
    int tokc = 0;
    char *token = (char *)(malloc(256 * sizeof(char)));
    token = strtok(info.buf, " \t\n");
    while (token != NULL)
    {
        toks[tokc] = (char *)(malloc(256 * sizeof(char)));
        strcpy(toks[tokc++], token);
        token = strtok(NULL, " \t\n");
    }

    if (strcmp(toks[0], "copy") == 0)
    {
        if (strcmp(toks[5], "source") == 0)
        {
            if (strcmp(toks[1], "file") == 0)
            {
                send_file(toks[3], toks[2], info.nm_socket);
            }
        }
        else if (strcmp(toks[5], "destination") == 0)
        {
            if (strcmp(toks[1], "file") == 0)
            {
                recv_file(toks[4], info.nm_socket);
            }
        }
        else
        {
            printf("Invalid command\n");
            goto done;
        }
    }

    char *dirnm = (char *)(malloc(512 * sizeof(char)));
    getcwd(dirnm, 512);
    char *path = (char *)(malloc(1024 * sizeof(char)));
    strcpy(path, dirnm);
    strcat(path, toks[3]);
    chdir(path);

    if (strcmp(toks[0], "create") == 0)
    {
        if (strcmp(toks[1], "dir") == 0)
        {
            if (mkdir(toks[2], 0777) == 0)
            {
                printf("Created directory successfully\n");
                bzero(info.buf, 1024);
                sprintf(info.buf, "%d", SUCCESSFUL);
                if (send(info.nm_socket, info.buf, sizeof(info.buf), 0) < 0)
                {
                    perror("Send");
                    exit(1);
                }
            }
            else
            {
                printf("Could not create directory\n");
                bzero(info.buf, 1024);
                sprintf(info.buf, "%d", UNSUCCESSFUL);
                if (send(info.nm_socket, info.buf, sizeof(info.buf), 0) < 0)
                {
                    perror("Send");
                    exit(1);
                }
            }
        }
        else if (strcmp(toks[1], "file") == 0)
        {
            if (creat(toks[2], 0777) >= 0)
            {
                printf("Created file successfully\n");
                bzero(info.buf, 1024);
                sprintf(info.buf, "%d", SUCCESSFUL);
                if (send(info.nm_socket, info.buf, sizeof(info.buf), 0) < 0)
                {
                    perror("Send");
                    exit(1);
                }
            }
            else
            {
                printf("Could not create file\n");
                bzero(info.buf, 1024);
                sprintf(info.buf, "%d", UNSUCCESSFUL);
                if (send(info.nm_socket, info.buf, sizeof(info.buf), 0) < 0)
                {
                    perror("Send");
                    exit(1);
                }
            }
        }
        else
        {
            printf("Wrong argument, should be file or dir\n");
        }
    }
    else if (strcmp(toks[0], "delete") == 0)
    {
        if (strcmp(toks[1], "dir") == 0)
        {
            char *comm[4] = {"rm", "-r", toks[2], NULL};

            int pid = fork();
            int success = 0;
            if (pid < 0)
            {
                perror("Fork");
                exit(1);
            }
            if (pid == 0)
            {
                execvp(comm[0], comm);
                exit(1);
            }
            else
            {
                int status;
                waitpid(pid, &status, 0);
                if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
                {
                    success = 1;
                }
                else
                {
                    success = 0;
                }
            }

            if (success)
            {
                printf("Deleted directory successfully\n");
                bzero(info.buf, 1024);
                sprintf(info.buf, "%d", SUCCESSFUL);
                if (send(info.nm_socket, info.buf, sizeof(info.buf), 0) < 0)
                {
                    perror("Send");
                    exit(1);
                }
            }
            else
            {
                printf("Could not delete directory\n");
                bzero(info.buf, 1024);
                sprintf(info.buf, "%d", UNSUCCESSFUL);
                if (send(info.nm_socket, info.buf, sizeof(info.buf), 0) < 0)
                {
                    perror("Send");
                    exit(1);
                }
            }
        }
        else if (strcmp(toks[1], "file") == 0)
        {
            if (remove(toks[2]) == 0)
            {
                printf("Deleted file successfully\n");
                bzero(info.buf, 1024);
                sprintf(info.buf, "%d", SUCCESSFUL);
                if (send(info.nm_socket, info.buf, sizeof(info.buf), 0) < 0)
                {
                    perror("Send");
                    exit(1);
                }
            }
            else
            {
                printf("Could not delete file\n");
                bzero(info.buf, 1024);
                sprintf(info.buf, "%d", UNSUCCESSFUL);
                if (send(info.nm_socket, info.buf, sizeof(info.buf), 0) < 0)
                {
                    perror("Send");
                    exit(1);
                }
            }
        }
        else
        {
            printf("Wrong argument, should be file or dir\n");
        }
    }

    chdir(dirnm);
done:
    return NULL;
}

void *nm_to_ss(void *arg)
{
    ss_info info = *(ss_info *)(arg);
    int nm_socket;
    struct sockaddr_in nm_add;
    socklen_t nm_add_size;
    char buf[1024];
    nm_add_size = sizeof(nm_add);

    while (1)
    {
        nm_socket = accept(info.ss_nm_socket, (struct sockaddr *)(&nm_add), &nm_add_size);
        if (nm_socket < 0)
        {
            perror("Accept");
            exit(1);
        }
        printf("Naming server connected\n");

        bzero(buf, 1024);
        if (recv(nm_socket, buf, 1024, 0) < 0)
        {
            perror("receive");
            exit(1);
        }

        nm_comm_info *nm_info = (nm_comm_info *)(malloc(sizeof(nm_comm_info)));
        strcpy(nm_info->buf, buf);
        nm_info->ss_nm_socket = info.ss_nm_socket;
        nm_info->nm_socket = nm_socket;
        pthread_t *nm_comm = (pthread_t *)(malloc(sizeof(pthread_t)));
        pthread_create(nm_comm, NULL, nm_comm_func, (void *)nm_info);
    }

done:
    return NULL;
}

void *clt_comm_func(void *arg)
{
    clt_comm_info info = *(clt_comm_info *)(arg);

    // tokenizing the command from client
    char *toks[6];
    int tokc = 0;
    char *token = (char *)(malloc(256 * sizeof(char)));
    token = strtok(info.buf, " \t\n");
    while (token != NULL)
    {
        toks[tokc] = (char *)(malloc(256 * sizeof(char)));
        strcpy(toks[tokc++], token);
        token = strtok(NULL, " \t\n");
    }

    char *dirnm = (char *)(malloc(512 * sizeof(char)));
    getcwd(dirnm, 512);
    char *path = (char *)(malloc(1024 * sizeof(char)));
    strcpy(path, dirnm);
    strcat(path, toks[2]);
    chdir(path);

    if (strcmp(toks[0], "read") == 0)
    {
        sem_wait(&write_ll_lock);
        // check if no one is writing in that file
        int found = search_node(write_ll, toks[1]);
        if (found)
        {
            printf("Cannot allow reading file\n");
            bzero(info.buf, 1024);
            sprintf(info.buf, "%d", UNSUCCESSFUL);
            if (send(info.clt_socket, info.buf, sizeof(info.buf), 0) < 0)
            {
                perror("Send");
                exit(1);
            }
            sem_post(&write_ll_lock);
            goto end;
        }
        sem_post(&write_ll_lock);

        FILE *fp;
        fp = fopen(toks[1], "r");
        sem_wait(&read_ll_lock);
        // add the file to read ll
        node *ele = create_node(toks[1]);
        read_ll = insert_node(read_ll, ele);
        sem_post(&read_ll_lock);
        while (fgets(info.buf, 1023, fp) != NULL)
        {
            // printf("");
            if (send(info.clt_socket, info.buf, sizeof(info.buf), 0) < 0)
            {
                perror("Send");
                exit(1);
            }
            // printf("sent: %s\n", info.buf);
            bzero(info.buf, 1024);
        }
        bzero(info.buf, 1024);
        sprintf(info.buf, "%d", DATA_END);
        if (send(info.clt_socket, info.buf, sizeof(info.buf), 0) < 0)
        {
            perror("Send");
            exit(1);
        }
        sem_wait(&read_ll_lock);
        // remove the file from read ll
        read_ll = delete_node(read_ll, toks[1]);
        sem_post(&read_ll_lock);
        fclose(fp);

        bzero(info.buf, 1024);
        sprintf(info.buf, "%d", SUCCESSFUL);
        if (send(info.clt_socket, info.buf, sizeof(info.buf), 0) < 0)
        {
            perror("Send");
            exit(1);
        }
    }
    else if (strcmp(toks[0], "write") == 0)
    {
        sem_wait(&write_ll_lock);
        int found = 1;
        // check if no one is writing in that file
        found = search_node(write_ll, toks[1]);
        if (found)
        {
            printf("Cannot allow writing in file\n");
            bzero(info.buf, 1024);
            sprintf(info.buf, "%d", UNSUCCESSFUL);
            if (send(info.clt_socket, info.buf, sizeof(info.buf), 0) < 0)
            {
                perror("Send");
                exit(1);
            }
            sem_post(&write_ll_lock);
            goto end;
        }
        sem_post(&write_ll_lock);

        sem_wait(&read_ll_lock);
        // check if no one is reading that file
        found = search_node(read_ll, toks[1]);
        if (found)
        {
            printf("Cannot allow writing in file\n");
            bzero(info.buf, 1024);
            sprintf(info.buf, "%d", UNSUCCESSFUL);
            if (send(info.clt_socket, info.buf, sizeof(info.buf), 0) < 0)
            {
                perror("Send");
                exit(1);
            }
            sem_post(&read_ll_lock);
            goto end;
        }
        sem_post(&read_ll_lock);

        sem_wait(&write_ll_lock);
        // add the file to write ll
        node *ele = create_node(toks[1]);
        write_ll = insert_node(write_ll, ele);
        sem_post(&write_ll_lock);

        FILE *fp;
        fp = fopen(toks[1], "w");
        // bzero(info.buf, 1024);
        // fputs(info.buf, fp);
        fclose(fp);

        fp = fopen(toks[1], "a");
        while (1)
        {
            bzero(info.buf, 1024);
            if (recv(info.clt_socket, info.buf, 1023, 0) < 0)
            {
                perror("Receive");
                exit(1);
            }
            char ack[5];
            bzero(ack, 5);
            sprintf(ack, "%d", DATA_END);
            if (strcmp(ack, info.buf) == 0)
                break;
            fprintf(fp, "%s", info.buf);
        }
        fclose(fp);
        sem_wait(&write_ll_lock);
        // remove the file from write ll
        write_ll = delete_node(write_ll, toks[1]);
        sem_post(&write_ll_lock);

        bzero(info.buf, 1024);
        sprintf(info.buf, "%d", SUCCESSFUL);
        if (send(info.clt_socket, info.buf, sizeof(info.buf), 0) < 0)
        {
            perror("Send");
            exit(1);
        }
    }
    else if (strcmp(toks[0], "get") == 0)
    {
        struct stat fs;
        if (stat(toks[1], &fs) < 0)
        {
            bzero(info.buf, 1024);
            sprintf(info.buf, "%d", UNSUCCESSFUL);
            if (send(info.clt_socket, info.buf, sizeof(info.buf), 0) < 0)
            {
                perror("Send");
                exit(1);
            }
            perror("Stat");
            exit(1);
        }
        sprintf(info.buf, "Permissions: %X    Size: %ld", fs.st_mode, fs.st_size);
        if (send(info.clt_socket, info.buf, sizeof(info.buf), 0) < 0)
        {
            perror("Send");
            exit(1);
        }

        bzero(info.buf, 1024);
        sprintf(info.buf, "%d", SUCCESSFUL);
        if (send(info.clt_socket, info.buf, sizeof(info.buf), 0) < 0)
        {
            perror("Send");
            exit(1);
        }
    }
    else
    {
        printf("invalid command\n");
    }

end:
    chdir(dirnm);

    return NULL;
}

void *clt_to_ss(void *arg)
{
    ss_info info = *(ss_info *)(arg);

    while (1)
    {
        int clt_socket;
        struct sockaddr_in clt_add;
        socklen_t clt_add_size;
        char buf[1024];
        clt_add_size = sizeof(clt_add);
        clt_socket = accept(info.ss_clt_socket, (struct sockaddr *)(&clt_add), &clt_add_size);
        if (clt_socket < 0)
        {
            continue;
        }
        printf("Client connected\n");

        bzero(buf, 1024);
        if (recv(clt_socket, buf, 1024, 0) < 0)
        {
            perror("receive");
            exit(1);
        }

        clt_comm_info *clt_info = (clt_comm_info *)(malloc(sizeof(clt_comm_info)));
        strcpy(clt_info->buf, buf);
        clt_info->ss_clt_socket = info.ss_clt_socket;
        clt_info->clt_socket = clt_socket;
        pthread_t *clt_comm = (pthread_t *)(malloc(sizeof(pthread_t)));
        pthread_create(clt_comm, NULL, clt_comm_func, (void *)clt_info);
    }

    return NULL;
}

int main()
{
    // ip of ss
    char *ip = "127.0.0.1";

    // initializing lock and linked lists
    sem_init(&read_ll_lock, 0, 1);
    sem_init(&write_ll_lock, 0, 1);
    char empty[5];
    bzero(empty, 5);
    read_ll = create_node(empty);
    write_ll = create_node(empty);

    // asking accessible paths
    printf("Enter no of paths\n");
    int n_path;
    scanf("%d", &n_path);
    printf("Enter the paths\n");
    char **paths = (char **)(malloc((n_path + 1) * sizeof(char *)));
    for (int i = 1; i <= n_path; ++i)
    {
        paths[i] = (char *)(malloc(1024 * sizeof(char))); // assuming path size < 1024
        scanf("%s", paths[i]);
    }

    int ss_nm_socket, ss_clt_socket, nm_ss_socket;
    struct sockaddr_in ss_nm_add, ss_clt_add, nm_ss_add;
    socklen_t ss_nm_add_size, ss_clt_add_size, nm_ss_add_size;

    if ((ss_nm_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("From nm socket");
        exit(1);
    }
    if ((ss_clt_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("From client socket");
        exit(1);
    }
    if ((nm_ss_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("To nm socket");
        exit(1);
    }

    memset(&ss_nm_add, '\0', sizeof(ss_nm_add));
    ss_nm_add.sin_family = AF_INET;
    ss_nm_add.sin_port = htons(0);
    ss_nm_add.sin_addr.s_addr = inet_addr(ip);
    ss_nm_add_size = sizeof(ss_nm_add);

    memset(&ss_clt_add, '\0', sizeof(ss_clt_add));
    ss_clt_add.sin_family = AF_INET;
    ss_clt_add.sin_port = htons(0);
    ss_clt_add.sin_addr.s_addr = inet_addr(ip);
    ss_clt_add_size = sizeof(ss_clt_add);

    memset(&nm_ss_add, '\0', sizeof(nm_ss_add));
    nm_ss_add.sin_family = AF_INET;
    nm_ss_add.sin_port = htons(1234);
    nm_ss_add.sin_addr.s_addr = inet_addr(ip);
    nm_ss_add_size = sizeof(nm_ss_add);

    ss_info *info = (ss_info *)(malloc(sizeof(ss_info)));

    info->ss_nm_socket = ss_nm_socket;
    info->ss_nm_add = ss_nm_add;
    info->ss_nm_add_size = ss_nm_add_size;
    info->nm_port = 0;

    info->ss_clt_socket = ss_clt_socket;
    info->ss_clt_add = ss_clt_add;
    info->ss_clt_add_size = ss_clt_add_size;
    info->clt_port = 0;

    info->nm_ss_socket = nm_ss_socket;
    info->nm_ss_add = nm_ss_add;
    info->nm_ss_add_size = nm_ss_add_size;
    info->ss_port = 1234;

    info->n_path = n_path;
    info->paths = paths;

    if (bind(ss_nm_socket, (struct sockaddr *)(&ss_nm_add), sizeof(ss_nm_add)) < 0)
    {
        perror("Binding for nm");
        exit(1);
    }
    if (getsockname(ss_nm_socket, (struct sockaddr *)(&ss_nm_add), &ss_nm_add_size) < 0)
    {
        perror("getsockname for nm");
        exit(1);
    }
    info->nm_port = ntohs(ss_nm_add.sin_port);
    if (listen(ss_nm_socket, 1000) < 0)
    {
        perror("Listen");
        exit(1);
    }
    printf("Listening for naming server on port %d\n", ntohs(ss_nm_add.sin_port));

    if (bind(ss_clt_socket, (struct sockaddr *)(&ss_clt_add), sizeof(ss_clt_add)) < 0)
    {
        perror("Binding for clt");
        exit(1);
    }
    if (getsockname(ss_clt_socket, (struct sockaddr *)(&ss_clt_add), &ss_clt_add_size) < 0)
    {
        perror("getsockname for clt");
        exit(1);
    }
    info->clt_port = ntohs(ss_clt_add.sin_port);
    if (listen(ss_clt_socket, 1000) < 0)
    {
        perror("Listen");
        exit(1);
    }
    printf("Listening for clients on port %d\n", ntohs(ss_clt_add.sin_port));

    pthread_t *ssnm_thrd = (pthread_t *)(malloc(sizeof(pthread_t)));
    pthread_t *nmss_thrd = (pthread_t *)(malloc(sizeof(pthread_t)));
    pthread_t *cltss_thrd = (pthread_t *)(malloc(sizeof(pthread_t)));

    pthread_create(ssnm_thrd, NULL, ss_to_nm, (void *)(info));
    pthread_create(nmss_thrd, NULL, nm_to_ss, (void *)(info));
    pthread_create(cltss_thrd, NULL, clt_to_ss, (void *)(info));

    pthread_join(*ssnm_thrd, NULL);
    pthread_join(*nmss_thrd, NULL);
    pthread_join(*cltss_thrd, NULL);
}