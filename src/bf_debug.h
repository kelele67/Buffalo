#ifndef BF_DEBUG_H
#define BF_DEBUG_H

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#ifdef NDEBUG
#define bf_debug(M, ...)
#else
/*缺省号代表一个可以变化的参数表。使用保留名 __VA_ARGS__ 把参数传递给宏。当宏的调用展开时，实际的参数就传递给 printf()了*/
/*如果可变参数被忽略或为空，'##'操作将使预处理器(preprocessor)去除掉它前面的那个逗号*/
#define bf_debug(M, ...) fprintf(stderr, "DEBUG %s:%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#define clean_errno() (errno == 0 ? "None" : strerror(errno))

#define log_err(M, ...) fprintf(stderr, "[ERROR] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)
#define log_warn(M, ...) fprintf(stderr, "[WARN] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)
#define log_info(M, ...) fprintf(stderr, "[INFO] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define check(A, M, ...) if(!(A)) {log_err(M "\n", ##__VA_ARGS__); /*exit(1)*/}
#define check_exit(A, M, ...) if(!(A)) {log_err(M "\n", ##__VA_ARGS__); exit(1);}
#define check_debug(A, M, ...) if(!(A)) {debug(M "\n", ##__VA_ARGS__); /*exit(1)*/}

#endif

