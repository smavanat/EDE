#ifndef __FILE_UTILS_H__
#define __FILE_UTILS_H__

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif
int read_to_end(char const *path, char **buf, bool addNull);
#ifdef __cplusplus
}
#endif
#endif
