#ifndef COMMON_H
#define COMMON_H

#include "dispatch_table.h"

#include <polyp_logs.h>
#include <constants.h>

#include <vector>
#include <algorithm>
#include <string>
#include <iostream>
#include <functional>
#include <memory>
#include <tuple>
#include <iterator>
#include <type_traits>

#define CHECKRET(expr)                             \
{                                                  \
    auto temp = expr;                              \
    if (temp != VK_SUCCESS) {                      \
    printf("Expression %s\n", #expr);              \
    printf("Failed with result = %lu\n", temp);    \
    exit(1);}                                      \
}

#endif // COMMON_H
