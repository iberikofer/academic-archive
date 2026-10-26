#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(_WIN32)
#include <windows.h>
static double getTimeMs(void) {
  LARGE_INTEGER freq, counter;
  QueryPerformanceFrequency(&freq);
  QueryPerformanceCounter(&counter);
  return (double)counter.QuadPart * 1000.0 / (double)freq.QuadPart;
}
#else
static double getTimeMs(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1000000.0;
}
#endif

static void printSpan(const int *arr, int len, size_t maxSample) {
  printf("[");
  if ((size_t)len <= maxSample) {
    for (int i = 0; i < len; ++i) {
      printf("%d%s", arr[i], (i + 1 < len ? ", " : ""));
    }
  } else {
    int half = (int)(maxSample / 2);
    for (int i = 0; i < half; ++i) {
      printf("%d, ", arr[i]);
    }
    printf("... ");
    for (int i = len - half; i < len; ++i) {
      printf("%d%s", arr[i], (i + 1 < len ? ", " : ""));
    }
  }
  printf("]");
}

static void printSampleArray(const int *arr, size_t len, size_t maxSample) {
  printSpan(arr, (int)len, maxSample);
  printf(" (size = %zu)\n", len);
}

static void selectionSort(int *arr, size_t n, bool verbose) {
  if (n <= 1) {
    if (verbose) {
      printf("  Array size <= 1, already sorted.\n");
    }
    return;
  }

  for (size_t i = 0; i < n - 1; ++i) {
    size_t minIndex = i;
    for (size_t j = i + 1; j < n; ++j) {
      if (arr[j] < arr[minIndex]) {
        minIndex = j;
      }
    }
    if (verbose) {
      printf("  [Step %zu] Minimum in subarray [%zu..%zu] is %d at index %zu.\n",
             i + 1, i, n - 1, arr[minIndex], minIndex);
    }
    if (minIndex != i) {
      int temp = arr[i];
      arr[i] = arr[minIndex];
      arr[minIndex] = temp;
      if (verbose) {
        printf("          Swap arr[%zu] (%d) <-> arr[%zu] (%d)\n",
               i, arr[minIndex], minIndex, arr[i]);
        printf("          Current state: ");
        printSampleArray(arr, n, n);
      }
    } else {
      if (verbose) {
        printf("          Element arr[%zu] = %d is already in correct place.\n",
               i, arr[i]);
        printf("          Current state: ");
        printSampleArray(arr, n, n);
      }
    }
  }
}

#define MIN_MERGE 32

static int computeMinRun(int n) {
  int r = 0;
  while (n >= MIN_MERGE) {
    r |= (n & 1);
    n >>= 1;
  }
  return n + r;
}

static void binaryInsertionSort(int *a, int lo, int hi, int start, int totalLen,
                                bool verbose) {
  assert(lo <= start && start <= hi);
  if (start == lo) {
    start++;
  }
  for (; start < hi; ++start) {
    int pivot = a[start];
    int left = lo;
    int right = start;

    while (left < right) {
      int mid = left + (right - left) / 2;
      if (pivot < a[mid]) {
        right = mid;
      } else {
        left = mid + 1;
      }
    }

    if (verbose) {
      printf("  [Binary Insertion] Moving element %d from index %d into position %d\n",
             pivot, start, left);
    }

    for (int k = start; k > left; --k) {
      a[k] = a[k - 1];
    }
    a[left] = pivot;

    if (verbose && totalLen > 0) {
      printf("          Current state: ");
      printSpan(a, totalLen, (size_t)totalLen);
      printf("\n");
    }
  }
}

static int countRunAndMakeAscending(int *a, int lo, int hi, int totalLen,
                                    bool verbose) {
  assert(lo < hi);
  int runHi = lo + 1;
  if (runHi == hi) {
    return 1;
  }

  if (a[runHi] < a[lo]) {
    while (runHi < hi && a[runHi] < a[runHi - 1]) {
      runHi++;
    }

    int left = lo, right = runHi - 1;
    while (left < right) {
      int temp = a[left];
      a[left] = a[right];
      a[right] = temp;
      left++;
      right--;
    }
    if (verbose) {
      printf("  [Natural Run] Found strictly descending run in range [%d..%d]. Reversed to ascending.\n",
             lo, runHi - 1);
      if (totalLen > 0) {
        printf("          Current state: ");
        printSpan(a, totalLen, (size_t)totalLen);
        printf("\n");
      }
    }
  } else {
    while (runHi < hi && !(a[runHi] < a[runHi - 1])) {
      runHi++;
    }
    if (verbose) {
      printf("  [Natural Run] Found natural ascending run in range [%d..%d] of length %d.\n",
             lo, runHi - 1, runHi - lo);
    }
  }
  return runHi - lo;
}

typedef struct {
  int base;
  int len;
} Run;

typedef struct {
  int *arr;
  int length;
  bool verbose;
  Run runStack[128];
  int stackSize;
  int *tmpBuffer;
  int tmpCapacity;
} Sorter;

static void sorter_ensure_capacity(Sorter *s, int minCap) {
  if (s->tmpCapacity < minCap) {
    int newCap = s->tmpCapacity == 0 ? minCap : s->tmpCapacity;
    while (newCap < minCap) {
      newCap *= 2;
    }
    int *newBuf = (int *)realloc(s->tmpBuffer, sizeof(int) * newCap);
    if (!newBuf) {
      perror("realloc failed");
      exit(EXIT_FAILURE);
    }
    s->tmpBuffer = newBuf;
    s->tmpCapacity = newCap;
  }
}

static void sorter_mergeLo(Sorter *s, int base1, int len1, int base2, int len2) {
  assert(len1 > 0 && len2 > 0 && base1 + len1 == base2);
  sorter_ensure_capacity(s, len1);

  memcpy(s->tmpBuffer, &s->arr[base1], sizeof(int) * len1);

  int cursor1 = 0;
  int cursor2 = base2;
  int dest = base1;
  const int end2 = base2 + len2;

  while (cursor1 < len1 && cursor2 < end2) {
    if (s->arr[cursor2] < s->tmpBuffer[cursor1]) {
      s->arr[dest++] = s->arr[cursor2++];
    } else {
      s->arr[dest++] = s->tmpBuffer[cursor1++];
    }
  }

  while (cursor1 < len1) {
    s->arr[dest++] = s->tmpBuffer[cursor1++];
  }
}

static void sorter_mergeHi(Sorter *s, int base1, int len1, int base2, int len2) {
  assert(len1 > 0 && len2 > 0 && base1 + len1 == base2);
  sorter_ensure_capacity(s, len2);

  memcpy(s->tmpBuffer, &s->arr[base2], sizeof(int) * len2);

  int cursor1 = base1 + len1 - 1;
  int cursor2 = len2 - 1;
  int dest = base2 + len2 - 1;

  while (cursor1 >= base1 && cursor2 >= 0) {
    if (s->tmpBuffer[cursor2] < s->arr[cursor1]) {
      s->arr[dest--] = s->arr[cursor1--];
    } else {
      s->arr[dest--] = s->tmpBuffer[cursor2--];
    }
  }

  while (cursor2 >= 0) {
    s->arr[dest--] = s->tmpBuffer[cursor2--];
  }
}

static void sorter_mergeAt(Sorter *s, int i) {
  int base1 = s->runStack[i].base;
  int len1 = s->runStack[i].len;
  int base2 = s->runStack[i + 1].base;
  int len2 = s->runStack[i + 1].len;

  if (s->verbose) {
    printf("  [Merge] Merging Run 1 [%d..%d] (len %d) with Run 2 [%d..%d] (len %d)\n",
           base1, base1 + len1 - 1, len1, base2, base2 + len2 - 1, len2);
  }

  s->runStack[i].len = len1 + len2;
  if (i == s->stackSize - 3) {
    s->runStack[i + 1] = s->runStack[i + 2];
  }
  s->stackSize--;

  if (len1 <= len2) {
    sorter_mergeLo(s, base1, len1, base2, len2);
  } else {
    sorter_mergeHi(s, base1, len1, base2, len2);
  }

  if (s->verbose) {
    printf("          Current state after merge: ");
    printSpan(s->arr, s->length, (size_t)s->length);
    printf("\n");
  }
}

static void sorter_mergeCollapse(Sorter *s) {
  while (s->stackSize > 1) {
    int n = s->stackSize - 2;
    if ((n > 0 &&
         s->runStack[n - 1].len <= s->runStack[n].len + s->runStack[n + 1].len) ||
        (n > 1 &&
         s->runStack[n - 2].len <= s->runStack[n - 1].len + s->runStack[n].len)) {
      if (s->runStack[n - 1].len < s->runStack[n + 1].len) {
        n--;
      }
      sorter_mergeAt(s, n);
    } else if (s->runStack[n].len <= s->runStack[n + 1].len) {
      sorter_mergeAt(s, n);
    } else {
      break;
    }
  }
}

static void sorter_mergeForceCollapse(Sorter *s) {
  while (s->stackSize > 1) {
    int n = s->stackSize - 2;
    if (n > 0 && s->runStack[n - 1].len < s->runStack[n + 1].len) {
      n--;
    }
    sorter_mergeAt(s, n);
  }
}

static void timsort(int *arr, size_t n, bool verbose) {
  if (n <= 1) {
    return;
  }

  int length = (int)n;
  if (length < MIN_MERGE) {
    if (verbose) {
      printf("  [Timsort] Array length %d < MIN_MERGE (%d). Sorting via Binary Insertion Sort.\n",
             length, MIN_MERGE);
    }
    int initRunLen = countRunAndMakeAscending(arr, 0, length, length, verbose);
    binaryInsertionSort(arr, 0, length, initRunLen, length, verbose);
    return;
  }

  int minRun = computeMinRun(length);
  if (verbose) {
    printf("  [Timsort] Computed minRun = %d for length %d\n", minRun, length);
  }

  Sorter s;
  s.arr = arr;
  s.length = length;
  s.verbose = verbose;
  s.stackSize = 0;
  s.tmpBuffer = NULL;
  s.tmpCapacity = 0;

  int cursor = 0;
  do {
    int runLen = countRunAndMakeAscending(arr, cursor, length, length, verbose);
    if (runLen < minRun) {
      int remaining = length - cursor;
      int force = remaining < minRun ? remaining : minRun;
      if (verbose) {
        printf("  [Timsort] Extending run from length %d to %d via Binary Insertion Sort\n",
               runLen, force);
      }
      binaryInsertionSort(arr, cursor, cursor + force, cursor + runLen,
                          length, verbose);
      runLen = force;
    }

    s.runStack[s.stackSize].base = cursor;
    s.runStack[s.stackSize].len = runLen;
    s.stackSize++;

    if (verbose) {
      printf("  [Timsort] Run recorded on stack: base=%d, len=%d\n", cursor,
             runLen);
    }
    sorter_mergeCollapse(&s);

    cursor += runLen;
  } while (cursor < length);

  if (verbose && s.stackSize > 1) {
    printf("  [Timsort] Finalizing: collapsing remaining %d runs on stack.\n",
           s.stackSize);
  }
  sorter_mergeForceCollapse(&s);

  if (s.tmpBuffer) {
    free(s.tmpBuffer);
    s.tmpBuffer = NULL;
  }
}

static bool verifySorted(const int *arr, size_t n) {
  for (size_t i = 0; i + 1 < n; ++i) {
    if (arr[i] > arr[i + 1]) {
      return false;
    }
  }
  return true;
}

static void printFormattedTime(double ms) {
  if (ms < 1.0) {
    printf("%.3f ms (%lld us)", ms, (long long)(ms * 1000.0));
  } else if (ms < 1000.0) {
    printf("%.3f ms", ms);
  } else {
    printf("%.3f s (%.1f ms)", ms / 1000.0, ms);
  }
}

static int *generateRandomArray(size_t size) {
  int *data = (int *)malloc(size * sizeof(int));
  if (!data) {
    perror("malloc failed");
    exit(EXIT_FAILURE);
  }
  int maxVal = (int)(size * 5 > 50 ? size * 5 : 50);
  for (size_t i = 0; i < size; ++i) {
    data[i] = 1 + rand() % maxVal;
  }
  return data;
}

int main(void) {
  srand((unsigned int)time(NULL));

  while (1) {
    printf("1. Enter custom array manually\n");
    printf("2. Generate random array of custom size\n");
    printf("3. Exit\n");
    printf("Choose an option (1-3): ");

    int choice = 0;
    if (scanf("%d", &choice) != 1) {
      int c;
      while ((c = getchar()) != '\n' && c != EOF) {}
      if (c == EOF) {
        break;
      }
      continue;
    }

    if (choice == 3) {
      printf("Exiting program. Goodbye!\n");
      break;
    }

    int *arr = NULL;
    size_t n = 0;

    if (choice == 1) {
      printf("Enter number of elements: ");
      if (scanf("%zu", &n) != 1 || n == 0) {
        int c;
        while ((c = getchar()) != '\n' && c != EOF) {}
        if (c == EOF) {
          break;
        }
        continue;
      }
      arr = (int *)malloc(n * sizeof(int));
      if (!arr) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
      }
      printf("Enter %zu integers: ", n);
      bool readOk = true;
      for (size_t i = 0; i < n; ++i) {
        if (scanf("%d", &arr[i]) != 1) {
          readOk = false;
          break;
        }
      }
      if (!readOk) {
        int c;
        while ((c = getchar()) != '\n' && c != EOF) {}
        if (c == EOF) {
          free(arr);
          break;
        }
        printf("Invalid input. Please try again.\n\n");
        free(arr);
        continue;
      }
    } else if (choice == 2) {
      printf("Enter array size: ");
      if (scanf("%zu", &n) != 1 || n == 0) {
        int c;
        while ((c = getchar()) != '\n' && c != EOF) {}
        if (c == EOF) {
          break;
        }
        continue;
      }
      arr = generateRandomArray(n);
    } else {
      printf("Invalid choice. Please select 1, 2, or 3.\n\n");
      continue;
    }

    printf("\nOriginal array: ");
    printSampleArray(arr, n, 12);

    printf("\nChoose sorting algorithm:\n");
    printf("1. Selection Sort\n");
    printf("2. Timsort\n");
    printf("3. Compare Both\n");
    printf("Choice (1-3): ");

    int algoChoice = 0;
    if (scanf("%d", &algoChoice) != 1) {
      int c;
      while ((c = getchar()) != '\n' && c != EOF) {}
      free(arr);
      if (c == EOF) {
        break;
      }
      continue;
    }

    if (algoChoice == 1) {
      int *toSort = (int *)malloc(n * sizeof(int));
      int *benchCopy = (int *)malloc(n * sizeof(int));
      memcpy(toSort, arr, n * sizeof(int));
      memcpy(benchCopy, arr, n * sizeof(int));

      double t1 = getTimeMs();
      selectionSort(benchCopy, n, false);
      double t2 = getTimeMs();
      double ms = t2 - t1;

      bool showSteps = (n <= 30);
      if (n > 30) {
        printf("Array size is %zu (> 30). Show step-by-step trace? (y/n): ", n);
        char ans;
        if (scanf(" %c", &ans) == 1 && (ans == 'y' || ans == 'Y')) {
          showSteps = true;
        }
      }

      if (showSteps) {
        printf("\n--- Step-by-Step Selection Sort Execution ---\n");
        selectionSort(toSort, n, true);
      } else {
        memcpy(toSort, benchCopy, n * sizeof(int));
      }

      printf("\nSorted with Selection Sort:\n");
      printSampleArray(toSort, n, 12);
      printf("Calculation time: ");
      printFormattedTime(ms);
      printf("\n");
      printf("Validation:       %s\n\n",
             verifySorted(toSort, n) ? "SUCCESS" : "FAILED");

      free(toSort);
      free(benchCopy);
    } else if (algoChoice == 2) {
      int *toSort = (int *)malloc(n * sizeof(int));
      int *benchCopy = (int *)malloc(n * sizeof(int));
      memcpy(toSort, arr, n * sizeof(int));
      memcpy(benchCopy, arr, n * sizeof(int));

      double t1 = getTimeMs();
      timsort(benchCopy, n, false);
      double t2 = getTimeMs();
      double ms = t2 - t1;

      bool showSteps = (n <= 30);
      if (n > 30) {
        printf("Array size is %zu (> 30). Show step-by-step trace? (y/n): ", n);
        char ans;
        if (scanf(" %c", &ans) == 1 && (ans == 'y' || ans == 'Y')) {
          showSteps = true;
        }
      }

      if (showSteps) {
        printf("\n--- Step-by-Step Timsort Execution ---\n");
        timsort(toSort, n, true);
      } else {
        memcpy(toSort, benchCopy, n * sizeof(int));
      }

      printf("\nSorted with Timsort:\n");
      printSampleArray(toSort, n, 12);
      printf("Calculation time: ");
      printFormattedTime(ms);
      printf("\n");
      printf("Validation:       %s\n\n",
             verifySorted(toSort, n) ? "SUCCESS" : "FAILED");

      free(toSort);
      free(benchCopy);
    } else if (algoChoice == 3) {
      int *toSort1 = (int *)malloc(n * sizeof(int));
      int *toSort2 = (int *)malloc(n * sizeof(int));
      memcpy(toSort1, arr, n * sizeof(int));
      memcpy(toSort2, arr, n * sizeof(int));

      printf("\nBenchmarking both algorithms on %zu elements (silent mode)...\n",
             n);

      double t1 = getTimeMs();
      selectionSort(toSort1, n, false);
      double t2 = getTimeMs();
      double ms1 = t2 - t1;

      double t3 = getTimeMs();
      timsort(toSort2, n, false);
      double t4 = getTimeMs();
      double ms2 = t4 - t3;

      printf("\n--- Comparison Results ---\n");
      printf("Selection Sort: ");
      printFormattedTime(ms1);
      printf(" | Valid: %s\n", verifySorted(toSort1, n) ? "YES" : "NO");

      printf("Timsort:        ");
      printFormattedTime(ms2);
      printf(" | Valid: %s\n", verifySorted(toSort2, n) ? "YES" : "NO");

      if (ms1 > 0.0 && ms2 > 0.0) {
        printf("Speedup:        %.1fx faster (Timsort vs Selection Sort)\n\n",
               ms1 / ms2);
      } else {
        printf("\n");
      }

      free(toSort1);
      free(toSort2);
    } else {
      printf("Invalid algorithm choice.\n\n");
    }

    free(arr);
  }

  return 0;
}
