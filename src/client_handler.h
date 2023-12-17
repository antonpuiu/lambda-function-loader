#ifndef CLIENT_HANDLER_H_
#define CLIENT_HANDLER_H_

/* lambda function without parameter */
typedef void (*lambda_func_t)(void);

/* lambda function with parameter */
typedef void (*lambda_param_func_t)(const char*);

struct lib {
    char* outputfile;
    char* libname;
    char* funcname;
    char* filename;

    void* handle;

    lambda_func_t run;
    lambda_param_func_t p_run;
};

void thread_function(int connectfd);

#endif // CLIENT_HANDLER_H_
