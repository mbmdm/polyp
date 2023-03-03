#ifndef COMMON_H
#define COMMON_H

#include "dispatch_table.h"

#include <vector>
#include <algorithm>
#include <string>
#include <iostream>
#include <functional>
#include <memory>
#include <tuple>

#define CHECKRET(expr)                         \
{                                              \
auto temp = expr;                              \
if (temp != VK_SUCCESS) {                      \
printf("Expression %s\n", #expr);              \
printf("Failed with result = %lu\n", temp);    \
exit(1);}                                      \
}

#endif // COMMON_H
