
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <getopt.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "utils.h"


struct SumArgs {
  int *array;
  int begin;
  int end;
};

int Sum(const struct SumArgs *args) {
  int sum = 0;
  for(int *i = args->array + args->begin; i < args->array + args->end; i++){
      sum += *i;
  }

  return sum;
}

void *ThreadSum(void *args) {
  struct SumArgs *sum_args = (struct SumArgs *)args;
  return (void *)(size_t)Sum(sum_args);
}

int main(int argc, char **argv) {

  uint32_t threads_num = 0;
  uint32_t array_size = 0;
  uint32_t seed = 0;
  pthread_t threads[threads_num];

  while (1) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"seed", required_argument, 0, 0},
                                      {"array_size", required_argument, 0, 0},
                                      {"threads_num", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            seed = atoi(optarg);
            if (seed <= 0) {
                 printf("seed is a positive number\n");
                 return 1;
            }
            break;
          case 1:
            array_size = atoi(optarg);
            if (array_size <= 0) {
                printf("array_size is a positive number\n");
                return 1;
            }
            break;
          case 2:
            threads_num = atoi(optarg);
            if (array_size <= 0) {
                printf("pnum is a positive number\n");
                return 1;
            }
            break;
          defalut:
            printf("Index %d is out of options\n", option_index);
        }
        break;
      case '?':
        break;

      default:
        printf("getopt returned character code 0%o?\n", c);
    }
  }

  if (optind < argc) {
    printf("Has at least one no option argument\n");
    return 1;
  }
  

  int *array = malloc(sizeof(int) * array_size);
  struct SumArgs *args = malloc(sizeof(struct SumArgs) * threads_num);
  GenerateArray(array, array_size, seed);
  
  struct timeval start_time;
  struct timeval fin_time;
  gettimeofday(&start_time, NULL);
  
  
  uint32_t step = array_size/threads_num;
  for (uint32_t i = 0; i < threads_num; i++) {
    args[i].array = array;
    args[i].begin = step * i;
    args[i].end   = (i == threads_num - 1) ? array_size : step * (i+1);
    if (pthread_create(&threads[i], NULL, ThreadSum, (void *)(args+i))) {
      printf("Error: pthread_create failed!\n");
      return 1;
    }
  }

  int total_sum = 0;
  for (uint32_t i = 0; i < threads_num; i++) {
    int sum = 0;
    pthread_join(threads[i], (void **)&sum);
    total_sum += sum;
  }
  gettimeofday(&fin_time, NULL);
  printf("Total: %d\n", total_sum);

  double elapsed_time = (fin_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (fin_time.tv_usec - start_time.tv_usec) / 1000.0;
  printf("Elapsed time: %fms\n", elapsed_time);

  free(array);
  
  return 0;
}