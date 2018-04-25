#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>


#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"

int time_is_out = 0;
int pnum = -1;
pid_t* children;

void alarm_handler(int a){
    printf("ALLLLAAAAAAAAAAAAARRRM!!!!!!1111\n");
    for(int i = 0; i < pnum; i++){
        kill(children[i], SIGKILL);
    }
    time_is_out = 1;
}
int main(int argc, char **argv) {
  int seed = -1;
  int array_size = -1;
  int timeout = -1;
  
  bool with_files = false;

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"seed", required_argument, 0, 0},
                                      {"array_size", required_argument, 0, 0},
                                      {"pnum", required_argument, 0, 0},
                                      {"by_files", no_argument, 0, 'f'},
                                      {"timeout", required_argument, 0, 0},
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
            pnum = atoi(optarg);
            if (array_size <= 0) {
                printf("pnum is a positive number\n");
                return 1;
            }
            break;
          case 3:
            with_files = true;
            break;
         case 4:
            if(atoi(optarg) <= 0){
            printf("timeout is a positive number\n");
            return 1;
            }
            else timeout = atoi(optarg);
          break;
          defalut:
            printf("Index %d is out of options\n", option_index);
        }
        break;
      case 'f':
        with_files = true;
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

  if (seed == -1 || array_size == -1 || pnum == -1) {
    printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" \n",
           argv[0]);
    return 1;
  }

  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);
  volatile int active_child_processes = 0;
  children = malloc(sizeof(pid_t)*pnum);
  struct timeval start_time;
  gettimeofday(&start_time, NULL);
  int segment_size = array_size / pnum;
  int* pipefd;
  if(!with_files){
      pipefd = malloc(sizeof(int) * pnum * 2);
      for(int i = 0; i < pnum; i++){
          if (pipe(pipefd + 2*i) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
          }
      }
  }
  
  if(timeout != -1){
      alarm(timeout);
  }
  signal(SIGALRM, alarm_handler);
  
  for (int i = 0; i < pnum; i++) {
    pid_t child_pid = fork();
    if (child_pid >= 0) {
      // successful fork
      active_child_processes += 1;
      if (child_pid == 0) {
      sleep(3);
      struct MinMax min_max;
        if(i<pnum-1) min_max = GetMinMax(array, segment_size*i, segment_size*(i+1));
        else   min_max = GetMinMax(array, segment_size*i, array_size);
        printf("min: %i\n", min_max.min);
        if (with_files) {

          FILE * f;
          if(!i)f = fopen("minmax","w");
          else f  = fopen("minmax","a");
          fprintf(f,"%i\n%i\n", min_max.min, min_max.max);
          fclose(f);
        } else {
          write(pipefd[2*i+1], &min_max, 8);
        }
        return 0;
      }else{
        children[i] = child_pid;
      }

    } else {
      printf("Fork failed!\n");
      return 1;
    }
  }

 
 while (active_child_processes > 0) {
    wait(NULL);
    if(errno == ECHILD) break;
    active_child_processes -= 1;
  }  
  if(!time_is_out){
  int min = INT_MAX;
  int max = INT_MIN;
  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;
  
     
      FILE * f;
      if(with_files)f = fopen("minmax","r");
      for (int i = 0; i < pnum; i++) {
        
        struct MinMax min_max; 
        
        if (with_files) {
          fscanf(f,"%i%i",&min_max.min,&min_max.max);
        } else {
          read(pipefd[2*i], &min_max, sizeof(min_max));
          
        }
         
        if (min_max.min < min) min = min_max.min;
        if (min_max.max > max) max = min_max.max;
      }
        
  printf("Min: %d\n", min);
  printf("Max: %d\n", max);
  
  }else{
      
  printf("timeout\n");
  
  }

  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  free(array);
  free(pipefd);
  
  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;
  printf("Elapsed time: %fms\n", elapsed_time);
  fflush(NULL);
  return 0;
}
