/* SimAK65 debug
 * Copyright A.K. 2018, 2023
 */

#ifndef SIMAK65_ERROR_H_
#define SIMAK65_ERROR_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#ifndef LOG_LEVEL
#define LOG_LEVEL 3
#endif

#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define RESET "\x1B[0m"

#if LOG_LEVEL >= 1
#define FATAL(msg, ...) do { \
		fprintf(stderr, RED "[FATAL] [%s:%d] (%s): ", __FILE__, __LINE__, __func__); \
		fprintf(stderr, msg, ##__VA_ARGS__); \
		fprintf(stderr, "\nPress Any Key to Exit...\n" RESET); \
		getchar(); \
		exit(1); \
	} while (0)
#else
#define FATAL(msg, ...)
#endif

#if LOG_LEVEL >= 2
#define WARN(msg, ...) do { \
		fprintf(stderr, YEL "[WARNING] [%s:%d] (%s): ", __FILE__, __LINE__, __func__); \
		fprintf(stderr, msg, ##__VA_ARGS__); \
		fprintf(stderr, "\n" RESET); \
	} while (0)
#else
#define WARN(msg, ...)
#endif

#if LOG_LEVEL >= 3
#define INFO(msg, ...) do { \
		printf("[INFO] "); \
		printf(msg, ##__VA_ARGS__); \
		printf("\n"); \
	} while (0)
#else
#define INFO(msg, ...)
#endif

#if LOG_LEVEL >= 4
#define DEBUG(msg, ...) do { \
		fprintf(stderr, GRN "[DEBUG] [%s:%d] (%s): ", __FILE__, __LINE__, __func__); \
		fprintf(stderr, msg, ##__VA_ARGS__); \
		fprintf(stderr, "\n" RESET); \
	} while (0)
#else
#define DEBUG(msg, ...)
#endif

#endif /* SIMAK65_ERROR_H_ */
