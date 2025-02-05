#ifndef LOGGER_H
#define LOGGER_H
#include <stdio.h>
#define LOG(fmt, ...) fprintf(stderr, " [LOG] : " fmt "\n", ##__VA_ARGS__)
#endif