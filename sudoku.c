#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

const int BOARD_LEN = 9*9;
const int N_THREADS = 9*3;
const char* filename = "puzzle.txt";

const int OFFSET_ROW[] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
const int OFFSET_COL[] = {0, 9, 18, 27, 36, 45, 54, 63, 72};
const int OFFSET_SUB[] = {0, 1, 2, 9, 10, 11, 18, 19, 20};
const int OFFSET_SUB_INDEX[] = {0, 3, 6, 27, 30, 33, 54, 57, 60};
const void* OFFSET_SELECTOR[] = {&OFFSET_ROW, &OFFSET_COL, &OFFSET_SUB};
struct check_args {
  const int *offset_set;
  int *cells;
  int offset;
};

void *check(void *args) {
  struct check_args *arguments = (struct check_args*)args;
  
  for (int a=0; a<8; a++)
    for (int b=a+1; b<9; b++) {
      if (arguments->cells[arguments->offset_set[a]+arguments->offset]
       == arguments->cells[arguments->offset_set[b]+arguments->offset] //A != B
       && arguments->cells[arguments->offset_set[a]+arguments->offset] != 0
       && arguments->cells[arguments->offset_set[b]+arguments->offset] != 0) //A and B are not 0
        return (void*)1;
    }
  return (void*)0;
}


int main() {
  int valid = 1;
  pthread_t tid[N_THREADS];
  int cells[BOARD_LEN];
  int cellp = 0;
  FILE *file = fopen(filename, "r");
  
  // Read entire file
  fseek(file, 0, SEEK_END);
  long fsize = ftell(file); // File length
  rewind(file); // Go back to beginning
  char *buffer = (char*)calloc(fsize, sizeof(char));
  fread(buffer, sizeof(char), fsize, file);
  fclose(file); // Close file

  char *cellch = strtok(buffer, " \t\n\r");
  while (cellch != NULL && cellp < BOARD_LEN) {
    cells[cellp++] = atoi(cellch);
    cellch = strtok(NULL, " \t\n\r");
  }
  free(buffer);

  void *status[9];
  int i;
  for (i=0; i<9; ++i) {
    struct check_args *args;
    args = malloc(sizeof(struct check_args));
    (*args).offset = i*9;
    (*args).cells = cells;
    (*args).offset_set = &OFFSET_ROW;
    pthread_create(&tid[i], NULL, check, (void *)args);
  }
  while (i--) {
    pthread_join(tid[i], &status[i]);
    if (status[i] != 0) {
      printf("Row %d is invalid!\n", i);
      valid = 0;
    }
  }

  for (i=0; i<9; ++i) {
    struct check_args *args;
    args = malloc(sizeof(struct check_args));
    (*args).offset = i;
    (*args).cells = cells;
    (*args).offset_set = &OFFSET_COL;
    pthread_create(&tid[i], NULL, check, (void *)args);
  }
  while (i--) {
    pthread_join(tid[i], &status[i]);
    if (status[i] != 0) {
      printf("Column %d is invalid!\n", i);
      valid = 0;
    }
  }

  for (i=0; i<9; ++i) {
    struct check_args *args;
    args = malloc(sizeof(struct check_args));
    (*args).offset = OFFSET_SUB_INDEX[i];
    (*args).cells = cells;
    (*args).offset_set = &OFFSET_SUB;
    pthread_create(&tid[i], NULL, check, (void *)args);
  }
  while (i--) {
    pthread_join(tid[i], &status[i]);
    if (status[i] != 0) {
      printf("Subgrid %d is invalid!\n", i);
      valid = 0;
    }
  }

  if (valid) printf("Board is valid.\n");
  return 0;
}