#ifndef COMMON_H
#define COMMON_H

#include <Windows.h>
#define VK_USE_PLATFORM_WIN32_KHR
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#include <vector>
#include <algorithm>
#include <string>
#include <iostream>
#include <functional>
#include <memory>

#define CHECKRET(expr)                         \
{                                              \
auto temp = expr;                              \
if (temp != VK_SUCCESS) {                      \
printf("Expression %s\n", #expr);              \
printf("Failed with result = %lu\n", temp);    \
exit(1);}                                      \
}

#endif // COMMON_H
