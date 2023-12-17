#include <dlfcn.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "ipc.h"
#include "utils.h"
#include "client_handler.h"

#ifndef OUTPUT_TEMPLATE
#define OUTPUT_TEMPLATE "../checker/output/out-XXXXXX"
#endif

#ifndef DEFAULT_FUNCNAME
#define DEFAULT_FUNCNAME "run"
#endif

char getRandomChar()
{
    return (char)('A' + rand() % 26);
}

void replaceXWithRandom(char* str)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    srand(ts.tv_nsec);

    for (int i = 0; str[i] != '\0'; ++i) {
        if (str[i] == 'X') {
            str[i] = getRandomChar();
        }
    }
}

char* getAbsolutePath(int fd)
{
    char fdPath[BUFSIZE];
    char procPath[BUFSIZE];

    // Create the path to the symbolic link in /proc
    snprintf(procPath, sizeof(procPath), "/proc/self/fd/%d", fd);

    // Read the symbolic link
    ssize_t bytesRead = readlink(procPath, fdPath, sizeof(fdPath) - 1);

    if (bytesRead == -1) {
        perror("readlink failed");
        return NULL;
    }

    // Null-terminate the string
    fdPath[bytesRead] = '\0';

    // Allocate memory for the absolute path
    char* absolutePath = realpath(fdPath, NULL);

    if (absolutePath == NULL) {
        perror("realpath failed");
        return NULL;
    }

    return absolutePath;
}

static void print_err(struct lib* lib)
{
    char output[BUFSIZE];

    if (lib->filename)
        sprintf(output, "Error: %s %s %s could not be executed.\n", lib->libname, lib->funcname,
                lib->filename);
    else
        sprintf(output, "Error: %s %s could not be executed.\n", lib->libname, lib->funcname);

    write(1, output, strlen(output));
}

static int lib_load(struct lib* lib)
{
    if (lib->libname == NULL || lib->funcname == NULL) {
        print_err(lib);
        return 1;
    }

    lib->handle = dlopen(lib->libname, RTLD_LAZY);
    if (lib->handle == NULL) {
        print_err(lib);
        return 1;
    }

    // Load the function from the shared library
    if (lib->filename == NULL) {
        lib->run = (lambda_func_t)dlsym(lib->handle, lib->funcname);

        // Check if the function was loaded successfully
        if (lib->run == NULL) {
            print_err(lib);
            dlclose(lib->handle);
            return 1;
        }
    } else {
        lib->p_run = (lambda_param_func_t)dlsym(lib->handle, lib->funcname);

        // Check if the function was loaded successfully
        if (lib->p_run == NULL) {
            print_err(lib);
            dlclose(lib->handle);
            return 1;
        }
    }

    return 0;
}

static int lib_execute(struct lib* lib)
{
    if (lib->filename == NULL)
        lib->run();
    else
        lib->p_run(lib->filename);

    return 0;
}

static int lib_close(struct lib* lib)
{
    int rc = dlclose(lib->handle);
    DIE(rc < 0, "dlclose");

    return 0;
}

static int lib_posthooks(struct lib* lib)
{
    free(lib->libname);
    free(lib->funcname);

    if (lib->filename != NULL)
        free(lib->filename);

    return 0;
}

static int lib_run(struct lib* lib)
{
    int err;

    err = lib_load(lib);
    if (err != 0)
        return err;

    err = lib_execute(lib);
    if (err != 0)
        return err;

    err = lib_close(lib);
    if (err != 0)
        return err;

    return lib_posthooks(lib);
}

static int parse_command(const char* buf, char* name, char* func, char* params)
{
    int ret;

    ret = sscanf(buf, "%s %s %s", name, func, params);
    if (ret < 0)
        return -1;

    return ret;
}

void thread_function(int connectfd)
{
    char buffer[BUFSIZE];
    struct lib lib;

    memset(&lib, 0, sizeof(struct lib));

    /* TODO - get message from client */
    memset(buffer, 0, BUFSIZE);
    int rc = recv_socket(connectfd, buffer, BUFSIZE);
    DIE(rc < 0, "read");

    /* TODO - parse message with parse_command and populate lib */
    char lib_name[BUFSIZE / 3];
    char function_name[BUFSIZE / 3];
    char filename[BUFSIZE / 3];

    memset(lib_name, 0, BUFSIZE / 3);
    memset(function_name, 0, BUFSIZE / 3);
    memset(filename, 0, BUFSIZE / 3);

    int args = parse_command(buffer, lib_name, function_name, filename);

    if (args == 1) {
        lib.funcname = (char*)malloc(strlen(DEFAULT_FUNCNAME) * sizeof(char) + 1);
        memset(lib.funcname, 0, strlen(DEFAULT_FUNCNAME) * sizeof(char));
        strcpy(lib.funcname, DEFAULT_FUNCNAME);
    }
    if (args >= 1) {
        lib.libname = (char*)malloc(BUFSIZE / 3 * sizeof(char));
        DIE(lib.libname == NULL, "malloc");

        memset(lib.libname, 0, BUFSIZE / 3 * sizeof(char));
        lib.libname = strncpy(lib.libname, lib_name, strlen(lib_name));
    }
    if (args >= 2) {
        lib.funcname = (char*)malloc(BUFSIZE / 3 * sizeof(char));
        memset(lib.funcname, 0, BUFSIZE / 3 * sizeof(char));
        strncpy(lib.funcname, function_name, strlen(function_name));
    }
    if (args >= 3) {
        lib.filename = (char*)malloc(BUFSIZE / 3 * sizeof(char));
        strncpy(lib.filename, filename, strlen(filename));
    }

    /* TODO - handle request from client */
    int originalStdout = dup(STDOUT_FILENO);
    DIE(originalStdout < 0, "dup");
    close(STDOUT_FILENO);

    char output[] = OUTPUT_TEMPLATE;
    replaceXWithRandom(output);
    int newFd =
        open(output, O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    DIE(newFd < 0, "open");

    rc = dup2(newFd, STDOUT_FILENO);
    DIE(rc < 0, "dup2");

    lib_run(&lib);
    fflush(stdout);

    lib.outputfile = getAbsolutePath(newFd);

    memset(buffer, 0, BUFSIZE);
    memcpy(buffer, lib.outputfile, strlen(lib.outputfile));
    free(lib.outputfile);

    rc = write(connectfd, buffer, strlen(buffer));
    DIE(rc < 0, "write");

    close(connectfd);
    exit(0);
}
