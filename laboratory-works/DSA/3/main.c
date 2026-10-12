#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  char name[64];
  char language[32];
  double memoryMB;
  double executionTime;
} Program;

typedef struct {
  Program *buffer;
  int capacity;
  int front;
  int rear;
  int count;
} CircularQueue;

void queue_init(CircularQueue *q) {
  q->buffer = NULL;
  q->capacity = 0;
  q->front = 0;
  q->rear = -1;
  q->count = 0;
}

bool queue_is_allocated(const CircularQueue *q) { return q->buffer != NULL; }

bool queue_is_empty(const CircularQueue *q) { return q->count == 0; }

bool queue_is_full(const CircularQueue *q) { return q->count == q->capacity; }

bool queue_is_near_full(const CircularQueue *q) {
  return q->capacity > 1 && q->count == q->capacity - 1;
}

int queue_get_count(const CircularQueue *q) { return q->count; }

int queue_get_capacity(const CircularQueue *q) { return q->capacity; }

void queue_free(CircularQueue *q) {
  if (q->buffer != NULL) {
    free(q->buffer);
    q->buffer = NULL;
    printf("[INFO]: Queue dynamic memory successfully freed.\n");
  }
  q->capacity = 0;
  q->front = 0;
  q->rear = -1;
  q->count = 0;
}

bool queue_allocate(CircularQueue *q, int cap) {
  if (cap <= 0) {
    printf("[ERROR]: Queue capacity must be a positive integer (> 0)!\n");
    return false;
  }

  if (q->buffer != NULL) {
    queue_free(q);
  }

  q->buffer = (Program *)malloc((size_t)cap * sizeof(Program));
  if (q->buffer == NULL) {
    printf("[CRITICAL ERROR]: Failed to allocate memory using malloc!\n");
    q->capacity = 0;
    return false;
  }

  q->capacity = cap;
  q->front = 0;
  q->rear = -1;
  q->count = 0;
  printf("[SUCCESS]: Allocated dynamic memory for queue with capacity %d.\n",
         q->capacity);
  return true;
}

bool queue_enqueue(CircularQueue *q, Program prog) {
  if (!queue_is_allocated(q)) {
    printf("[ERROR]: Queue memory is not allocated! Please allocate memory "
           "first.\n");
    return false;
  }

  if (queue_is_full(q)) {
    printf("\n***************************************************************"
           "**\n");
    printf(" [QUEUE OVERFLOW ERROR]                                          "
           "\n");
    printf(" Cannot enqueue program \"%s\"!\n", prog.name);
    printf(" Queue is completely full: %d of %d slots used.\n", q->count,
           q->capacity);
    printf(" You must dequeue at least one element before inserting new "
           "ones.\n");
    printf("*****************************************************************"
           "\n");
    return false;
  }

  q->rear = (q->rear + 1) % q->capacity;
  q->buffer[q->rear] = prog;
  q->count++;

  printf("[SUCCESS]: Enqueued program \"%s\".\n", prog.name);
  printf("Queue state: %d of %d slots occupied.\n", q->count, q->capacity);

  if (queue_is_near_full(q)) {
    printf("\n---------------------------------------------------------------"
           "--\n");
    printf(" [WARNING: POTENTIAL QUEUE OVERFLOW]                             "
           "\n");
    printf(" Warning: Queue is almost full (%d of %d)!\n", q->count,
           q->capacity);
    printf(" Only 1 free slot remains. The next insertion will fill the "
           "queue.\n");
    printf("-----------------------------------------------------------------"
           "\n");
  }

  return true;
}

bool queue_dequeue(CircularQueue *q, Program *removedProg) {
  if (!queue_is_allocated(q)) {
    printf("[ERROR]: Queue memory is not allocated!\n");
    return false;
  }

  if (queue_is_empty(q)) {
    printf("[MESSAGE]: Queue is empty (Queue Underflow)! No elements to "
           "dequeue.\n");
    return false;
  }

  *removedProg = q->buffer[q->front];
  q->front = (q->front + 1) % q->capacity;
  q->count--;

  printf("[SUCCESS]: Dequeued program (FIFO): \"%s\"\n", removedProg->name);
  printf("         (Language: %s, Memory: %.2f MB, Execution Time: %.2f s)\n",
         removedProg->language, removedProg->memoryMB,
         removedProg->executionTime);
  printf("Remaining in queue: %d of %d slots.\n", q->count, q->capacity);
  return true;
}

double queue_get_total_time(const CircularQueue *q) {
  if (!queue_is_allocated(q) || queue_is_empty(q)) {
    return 0.0;
  }

  double total = 0.0;
  for (int i = 0; i < q->count; ++i) {
    int idx = (q->front + i) % q->capacity;
    total += q->buffer[idx].executionTime;
  }
  return total;
}

void queue_print(const CircularQueue *q) {
  if (!queue_is_allocated(q)) {
    printf("[INFO]: Queue is not allocated yet.\n");
    return;
  }

  if (queue_is_empty(q)) {
    printf("[INFO]: Queue is currently empty (0 of %d elements).\n",
           q->capacity);
    return;
  }

  printf("\n================================ QUEUE CONTENTS "
         "================================\n");
  printf("Buffer status: Capacity = %d | Elements = %d | Head (front) = %d | "
         "Tail (rear) = %d\n",
         q->capacity, q->count, q->front, q->rear);
  printf("-------------------------------------------------------------------"
         "-------------\n");
  printf("%-12s%-22s%-16s%-16s%-14s\n", "Position", "Program Name", "Language",
         "Memory (MB)", "Exec Time (s)");
  printf("-------------------------------------------------------------------"
         "-------------\n");

  for (int i = 0; i < q->count; ++i) {
    int idx = (q->front + i) % q->capacity;
    char orderTag[32];
    if (i == 0) {
      snprintf(orderTag, sizeof(orderTag), "%d (head)", i + 1);
    } else if (i == q->count - 1) {
      snprintf(orderTag, sizeof(orderTag), "%d (tail)", i + 1);
    } else {
      snprintf(orderTag, sizeof(orderTag), "%d", i + 1);
    }

    printf("%-12s%-22s%-16s%-16.1f%-14.2f\n", orderTag, q->buffer[idx].name,
           q->buffer[idx].language, q->buffer[idx].memoryMB,
           q->buffer[idx].executionTime);
  }

  printf("-------------------------------------------------------------------"
         "-------------\n");
  printf("Total execution time of all programs in queue: %.2f seconds.\n",
         queue_get_total_time(q));
  printf("==================================================================="
         "=============\n\n");
}

void clear_input_buffer(void) {
  int c;
  while ((c = getchar()) != '\n' && c != EOF) {
  }
}

void read_string(const char *prompt, char *buffer, size_t max_len) {
  while (true) {
    printf("%s", prompt);
    if (fgets(buffer, (int)max_len, stdin) != NULL) {
      size_t len = strlen(buffer);
      if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
        len--;
      }
      if (len > 0 && buffer[len - 1] == '\r') {
        buffer[len - 1] = '\0';
        len--;
      }
      if (len > 0) {
        return;
      }
    }
    printf(
        "[Error]: Value cannot be empty! Please enter a non-empty string.\n");
  }
}

double read_positive_double(const char *prompt) {
  double value;
  while (true) {
    printf("%s", prompt);
    if (scanf("%lf", &value) == 1 && value > 0) {
      clear_input_buffer();
      return value;
    }
    printf("[Error]: Invalid value! Please enter a positive number (e.g. "
           "15.5).\n");
    clear_input_buffer();
  }
}

int read_positive_int(const char *prompt) {
  int value;
  while (true) {
    printf("%s", prompt);
    if (scanf("%d", &value) == 1 && value > 0) {
      clear_input_buffer();
      return value;
    }
    printf(
        "[Error]: Invalid value! Please enter a positive integer (> 0).\n");
    clear_input_buffer();
  }
}

int main(void) {
  CircularQueue queue;
  queue_init(&queue);
  int choice = -1;

  while (true) {
    printf("\nMAIN MENU\n");
    printf(" 1. Allocate queue memory (set capacity)\n");
    printf(" 2. Enqueue program\n");
    printf(" 3. Dequeue program (FIFO)\n");
    printf(" 4. Print queue\n");
    printf(" 5. Calculate total execution time of all programs\n");
    printf(" 6. Free queue memory\n");
    printf(" 0. Exit\n\n");
    printf("Select action [0-6]: ");

    if (scanf("%d", &choice) != 1) {
      clear_input_buffer();
      printf(
          "[Error]: Invalid input! Please enter a number between 0 and 6.\n");
      continue;
    }
    clear_input_buffer();

    if (choice == 0) {
      queue_free(&queue);
      printf("\nExiting application. Queue memory freed. Goodbye!\n");
      break;
    }

    switch (choice) {
    case 1: {
      int cap = read_positive_int("Enter maximum queue capacity: ");
      queue_allocate(&queue, cap);
      break;
    }
    case 2: {
      if (!queue_is_allocated(&queue)) {
        printf("[ERROR]: Queue memory is not allocated yet! Please choose "
               "option 1 first.\n");
        break;
      }
      Program p;
      printf("\n--- Enter program details ---\n");
      read_string("Program name: ", p.name, sizeof(p.name));
      read_string("Programming language: ", p.language, sizeof(p.language));
      p.memoryMB = read_positive_double("Memory required (MB): ");
      p.executionTime = read_positive_double("Execution time (seconds): ");
      queue_enqueue(&queue, p);
      break;
    }
    case 3: {
      Program removed;
      queue_dequeue(&queue, &removed);
      break;
    }
    case 4: {
      queue_print(&queue);
      break;
    }
    case 5: {
      if (!queue_is_allocated(&queue) || queue_is_empty(&queue)) {
        printf(
            "[INFO]: Queue is empty. Total execution time = 0.00 seconds.\n");
      } else {
        printf("\n[RESULT]: Total execution time of all %d programs in queue: "
               "%.2f seconds.\n",
               queue_get_count(&queue), queue_get_total_time(&queue));
      }
      break;
    }
    case 6: {
      queue_free(&queue);
      break;
    }
    default:
      printf("[Error]: Unknown menu option. Please select 0 to 6.\n");
      break;
    }
  }

  return 0;
}
