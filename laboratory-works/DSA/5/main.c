#define _CRT_SECURE_NO_WARNINGS
#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(_WIN32)
#include <windows.h>
#endif

/* -------------------------------------------------------------------------
 * Timing Utilities
 * ------------------------------------------------------------------------- */
static double getTimeMs(void) {
#if defined(_WIN32)
  LARGE_INTEGER freq, counter;
  QueryPerformanceFrequency(&freq);
  QueryPerformanceCounter(&counter);
  return (double)counter.QuadPart * 1000.0 / (double)freq.QuadPart;
#else
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1000000.0;
#endif
}

/* -------------------------------------------------------------------------
 * DES Permutation Tables and Constants for LM-Hash
 * ------------------------------------------------------------------------- */
static const uint8_t IP_TABLE[64] = {
    58, 50, 42, 34, 26, 18, 10, 2, 60, 52, 44, 36, 28, 20, 12, 4,
    62, 54, 46, 38, 30, 22, 14, 6, 64, 56, 48, 40, 32, 24, 16, 8,
    57, 49, 41, 33, 25, 17, 9,  1, 59, 51, 43, 35, 27, 19, 11, 3,
    61, 53, 45, 37, 29, 21, 13, 5, 63, 55, 47, 39, 31, 23, 15, 7};

static const uint8_t FP_TABLE[64] = {
    40, 8, 48, 16, 56, 24, 64, 32, 39, 7, 47, 15, 55, 23, 63, 31,
    38, 6, 46, 14, 54, 22, 62, 30, 37, 5, 45, 13, 53, 21, 61, 29,
    36, 4, 44, 12, 52, 20, 60, 28, 35, 3, 43, 11, 51, 19, 59, 27,
    34, 2, 42, 10, 50, 18, 58, 26, 33, 1, 41, 9,  49, 17, 57, 25};

static const uint8_t PC1_TABLE[56] = {
    57, 49, 41, 33, 25, 17, 9,  1,  58, 50, 42, 34, 26, 18, 10, 2,  59, 51, 43,
    35, 27, 19, 11, 3,  60, 52, 44, 36, 63, 55, 47, 39, 31, 23, 15, 7,  62, 54,
    46, 38, 30, 22, 14, 6,  61, 53, 45, 37, 29, 21, 13, 5,  28, 20, 12, 4};

static const uint8_t PC2_TABLE[48] = {
    14, 17, 11, 24, 1,  5,  3,  28, 15, 6,  21, 10, 23, 19, 12, 4,
    26, 8,  16, 7,  27, 20, 13, 2,  41, 52, 31, 37, 47, 55, 30, 40,
    51, 45, 33, 48, 44, 49, 39, 56, 34, 53, 46, 42, 50, 36, 29, 32};

static const uint8_t SHIFT_SCHEDULE[16] = {1, 1, 2, 2, 2, 2, 2, 2,
                                           1, 2, 2, 2, 2, 2, 2, 1};

static const uint8_t E_BIT_TABLE[48] = {
    32, 1,  2,  3,  4,  5,  4,  5,  6,  7,  8,  9,  8,  9,  10, 11,
    12, 13, 12, 13, 14, 15, 16, 17, 16, 17, 18, 19, 20, 21, 20, 21,
    22, 23, 24, 25, 24, 25, 26, 27, 28, 29, 28, 29, 30, 31, 32, 1};

static const uint8_t S_BOX_TABLE[8][4][16] = {
    {{14, 4, 13, 1, 2, 15, 11, 8, 3, 10, 6, 12, 5, 9, 0, 7},
     {0, 15, 7, 4, 14, 2, 13, 1, 10, 6, 12, 11, 9, 5, 3, 8},
     {4, 1, 14, 8, 13, 6, 2, 11, 15, 12, 9, 7, 3, 10, 5, 0},
     {15, 12, 8, 2, 4, 9, 1, 7, 5, 11, 3, 14, 10, 0, 6, 13}},
    {{15, 1, 8, 14, 6, 11, 3, 4, 9, 7, 2, 13, 12, 0, 5, 10},
     {3, 13, 4, 7, 15, 2, 8, 14, 12, 0, 1, 10, 6, 9, 11, 5},
     {0, 14, 7, 11, 10, 4, 13, 1, 5, 8, 12, 6, 9, 3, 2, 15},
     {13, 8, 10, 1, 3, 15, 4, 2, 11, 6, 7, 12, 0, 5, 14, 9}},
    {{10, 0, 9, 14, 6, 3, 15, 5, 1, 13, 12, 7, 11, 4, 2, 8},
     {13, 7, 0, 9, 3, 4, 6, 10, 2, 8, 5, 14, 12, 11, 15, 1},
     {13, 6, 4, 9, 8, 15, 3, 0, 11, 1, 2, 12, 5, 10, 14, 7},
     {1, 10, 13, 0, 6, 9, 8, 7, 4, 15, 14, 3, 11, 5, 2, 12}},
    {{7, 13, 14, 3, 0, 6, 9, 10, 1, 2, 8, 5, 11, 12, 4, 15},
     {13, 8, 11, 5, 6, 15, 0, 3, 4, 7, 2, 12, 1, 10, 14, 9},
     {10, 6, 9, 0, 12, 11, 7, 13, 15, 1, 3, 14, 5, 2, 8, 4},
     {3, 15, 0, 6, 10, 1, 13, 8, 9, 4, 5, 11, 12, 7, 2, 14}},
    {{2, 12, 4, 1, 7, 10, 11, 6, 8, 5, 3, 15, 13, 0, 14, 9},
     {14, 11, 2, 12, 4, 7, 13, 1, 5, 0, 15, 10, 3, 9, 8, 6},
     {4, 2, 1, 11, 10, 13, 7, 8, 15, 9, 12, 5, 6, 3, 0, 14},
     {11, 8, 12, 7, 1, 14, 2, 13, 6, 15, 0, 9, 10, 4, 5, 3}},
    {{12, 1, 10, 15, 9, 2, 6, 8, 0, 13, 3, 4, 14, 7, 5, 11},
     {10, 15, 4, 2, 7, 12, 9, 5, 6, 1, 13, 14, 0, 11, 3, 8},
     {9, 14, 15, 5, 2, 8, 12, 3, 7, 0, 4, 10, 1, 13, 11, 6},
     {4, 3, 2, 12, 9, 5, 15, 10, 11, 14, 1, 7, 6, 0, 8, 13}},
    {{4, 11, 2, 14, 15, 0, 8, 13, 3, 12, 9, 7, 5, 10, 6, 1},
     {13, 0, 11, 7, 4, 9, 1, 10, 14, 3, 5, 12, 2, 15, 8, 6},
     {1, 4, 11, 13, 12, 3, 7, 14, 10, 15, 6, 8, 0, 5, 9, 2},
     {6, 11, 13, 8, 1, 4, 10, 7, 9, 5, 0, 15, 14, 2, 3, 12}},
    {{13, 2, 8, 4, 6, 15, 11, 1, 10, 9, 3, 14, 5, 0, 12, 7},
     {1, 15, 13, 8, 10, 3, 7, 4, 12, 5, 6, 11, 0, 14, 9, 2},
     {7, 11, 4, 1, 9, 12, 14, 2, 0, 6, 10, 13, 15, 3, 5, 8},
     {2, 1, 14, 7, 4, 10, 8, 13, 15, 12, 9, 0, 3, 5, 6, 11}}};

static const uint8_t P_PERM_TABLE[32] = {
    16, 7, 20, 21, 29, 12, 28, 17, 1,  15, 23, 26, 5,  18, 31, 10,
    2,  8, 24, 14, 32, 27, 3,  9,  19, 13, 30, 6,  22, 11, 4,  25};

static const uint8_t FIXED_MAGIC_BLOCK[8] = {0x4B, 0x47, 0x53, 0x21,
                                             0x40, 0x23, 0x24, 0x25};

/* -------------------------------------------------------------------------
 * Low-level DES Bit Operations
 * ------------------------------------------------------------------------- */
static void applyPermutation(const uint8_t *in, uint8_t *out,
                             const uint8_t *table, size_t outBits) {
  for (size_t i = 0; i < outBits; ++i) {
    size_t bitPos = table[i] - 1;
    uint8_t bit = (in[bitPos / 8] >> (7 - (bitPos % 8))) & 1;
    if (bit) {
      out[i / 8] |= (uint8_t)(1 << (7 - (i % 8)));
    } else {
      out[i / 8] &= (uint8_t)~(1 << (7 - (i % 8)));
    }
  }
}

static void shiftLeftBits(uint8_t *block, size_t bitCount, size_t shift) {
  uint32_t val = 0;
  for (size_t i = 0; i < 4; ++i) {
    val = (val << 8) | block[i];
  }
  val = (val >> (32 - bitCount));
  uint32_t mask = (1U << bitCount) - 1U;
  val = ((val << shift) | (val >> (bitCount - shift))) & mask;
  val = (val << (32 - bitCount));
  for (int i = 3; i >= 0; --i) {
    block[i] = (uint8_t)((val >> (8 * (3 - i))) & 0xFF);
  }
}

static void deriveSubkeys(const uint8_t rawKey[8], uint8_t subkeys[16][6]) {
  uint8_t cd[7] = {0};
  applyPermutation(rawKey, cd, PC1_TABLE, 56);

  uint8_t c[4] = {cd[0], cd[1], cd[2], (uint8_t)(cd[3] & 0xF0)};
  uint8_t d[4] = {(uint8_t)((cd[3] << 4) | (cd[4] >> 4)),
                  (uint8_t)((cd[4] << 4) | (cd[5] >> 4)),
                  (uint8_t)((cd[5] << 4) | (cd[6] >> 4)),
                  (uint8_t)(cd[6] << 4)};

  for (size_t r = 0; r < 16; ++r) {
    shiftLeftBits(c, 28, SHIFT_SCHEDULE[r]);
    shiftLeftBits(d, 28, SHIFT_SCHEDULE[r]);

    uint8_t merged[7] = {c[0],
                         c[1],
                         c[2],
                         (uint8_t)((c[3] & 0xF0) | ((d[0] >> 4) & 0x0F)),
                         (uint8_t)((d[0] << 4) | (d[1] >> 4)),
                         (uint8_t)((d[1] << 4) | (d[2] >> 4)),
                         (uint8_t)((d[2] << 4) | (d[3] >> 4))};

    for (size_t j = 0; j < 6; ++j) {
      subkeys[r][j] = 0;
    }
    applyPermutation(merged, subkeys[r], PC2_TABLE, 48);
  }
}

static void feistelRound(const uint8_t rIn[4], const uint8_t subkey[6],
                         uint8_t fOut[4]) {
  uint8_t eBits[6] = {0};
  applyPermutation(rIn, eBits, E_BIT_TABLE, 48);

  for (size_t i = 0; i < 6; ++i) {
    eBits[i] ^= subkey[i];
  }

  uint8_t sOutput[4] = {0};
  for (size_t i = 0; i < 8; ++i) {
    size_t bitIdx = i * 6;
    uint8_t b1 = (eBits[bitIdx / 8] >> (7 - (bitIdx % 8))) & 1;
    uint8_t b2 = (eBits[(bitIdx + 1) / 8] >> (7 - ((bitIdx + 1) % 8))) & 1;
    uint8_t b3 = (eBits[(bitIdx + 2) / 8] >> (7 - ((bitIdx + 2) % 8))) & 1;
    uint8_t b4 = (eBits[(bitIdx + 3) / 8] >> (7 - ((bitIdx + 3) % 8))) & 1;
    uint8_t b5 = (eBits[(bitIdx + 4) / 8] >> (7 - ((bitIdx + 4) % 8))) & 1;
    uint8_t b6 = (eBits[(bitIdx + 5) / 8] >> (7 - ((bitIdx + 5) % 8))) & 1;

    uint8_t row = (uint8_t)((b1 << 1) | b6);
    uint8_t col = (uint8_t)((b2 << 3) | (b3 << 2) | (b4 << 1) | b5);
    uint8_t val = S_BOX_TABLE[i][row][col];

    if (i % 2 == 0) {
      sOutput[i / 2] |= (uint8_t)(val << 4);
    } else {
      sOutput[i / 2] |= (uint8_t)(val & 0x0F);
    }
  }

  for (size_t i = 0; i < 4; ++i) {
    fOut[i] = 0;
  }
  applyPermutation(sOutput, fOut, P_PERM_TABLE, 32);
}

static void encryptCoreBlock(const uint8_t inBlock[8], const uint8_t rawKey[8],
                             uint8_t outBlock[8]) {
  uint8_t initial[8] = {0};
  applyPermutation(inBlock, initial, IP_TABLE, 64);

  uint8_t subkeys[16][6];
  deriveSubkeys(rawKey, subkeys);

  uint8_t left[4] = {initial[0], initial[1], initial[2], initial[3]};
  uint8_t right[4] = {initial[4], initial[5], initial[6], initial[7]};

  for (size_t r = 0; r < 16; ++r) {
    uint8_t fResult[4];
    feistelRound(right, subkeys[r], fResult);

    uint8_t nextRight[4];
    for (size_t i = 0; i < 4; ++i) {
      nextRight[i] = left[i] ^ fResult[i];
      left[i] = right[i];
      right[i] = nextRight[i];
    }
  }

  uint8_t preOutput[8] = {right[0], right[1], right[2], right[3],
                          left[0],  left[1],  left[2],  left[3]};

  for (size_t i = 0; i < 8; ++i) {
    outBlock[i] = 0;
  }
  applyPermutation(preOutput, outBlock, FP_TABLE, 64);
}

static void expandSevenToEight(const uint8_t in7[7], uint8_t out8[8]) {
  out8[0] = (uint8_t)(in7[0] >> 1);
  out8[1] = (uint8_t)(((in7[0] & 0x01) << 6) | (in7[1] >> 2));
  out8[2] = (uint8_t)(((in7[1] & 0x03) << 5) | (in7[2] >> 3));
  out8[3] = (uint8_t)(((in7[2] & 0x07) << 4) | (in7[3] >> 4));
  out8[4] = (uint8_t)(((in7[3] & 0x0F) << 3) | (in7[4] >> 5));
  out8[5] = (uint8_t)(((in7[4] & 0x1F) << 2) | (in7[5] >> 6));
  out8[6] = (uint8_t)(((in7[5] & 0x3F) << 1) | (in7[6] >> 7));
  out8[7] = (uint8_t)(in7[6] & 0x7F);

  for (size_t i = 0; i < 8; ++i) {
    out8[i] = (uint8_t)((out8[i] << 1) & 0xFF);
    uint8_t bits = 0;
    for (size_t b = 0; b < 8; ++b) {
      if ((out8[i] >> b) & 1) {
        bits++;
      }
    }
    if (bits % 2 == 0) {
      out8[i] |= 1;
    }
  }
}

/* -------------------------------------------------------------------------
 * Output Formatting Helpers
 * ------------------------------------------------------------------------- */
static size_t getUtf8VisibleLength(const char *str) {
  size_t len = 0;
  for (size_t i = 0; str[i] != '\0'; ++i) {
    unsigned char c = (unsigned char)str[i];
    if ((c & 0xC0) != 0x80) {
      len++;
    }
  }
  return len;
}

static void printCell(const char *str, size_t colWidth, bool alignRight) {
  size_t visLen = getUtf8VisibleLength(str);
  size_t pad = (visLen < colWidth) ? (colWidth - visLen) : 0;
  if (alignRight) {
    for (size_t i = 0; i < pad; ++i) {
      putchar(' ');
    }
    fputs(str, stdout);
  } else {
    fputs(str, stdout);
    for (size_t i = 0; i < pad; ++i) {
      putchar(' ');
    }
  }
}

/* -------------------------------------------------------------------------
 * Method 1: LM-Hash (LAN Manager Hash)
 * ------------------------------------------------------------------------- */
void computeMethodAlphaDigest(const char *input, char *outHex) {
  uint8_t buffer[14] = {0};
  size_t inLen = strlen(input);
  size_t copyLen = (inLen < 14) ? inLen : 14;
  for (size_t i = 0; i < copyLen; ++i) {
    buffer[i] = (uint8_t)toupper((unsigned char)input[i]);
  }

  uint8_t key1[8], key2[8];
  expandSevenToEight(buffer, key1);
  expandSevenToEight(buffer + 7, key2);

  uint8_t cipher1[8], cipher2[8];
  encryptCoreBlock(FIXED_MAGIC_BLOCK, key1, cipher1);
  encryptCoreBlock(FIXED_MAGIC_BLOCK, key2, cipher2);

  for (size_t i = 0; i < 8; ++i) {
    snprintf(outHex + i * 2, 3, "%02X", (unsigned int)cipher1[i]);
  }
  for (size_t i = 0; i < 8; ++i) {
    snprintf(outHex + 16 + i * 2, 3, "%02X", (unsigned int)cipher2[i]);
  }
  outHex[32] = '\0';
}

size_t computeMethodAlphaIndex(const char *input, size_t tableCapacity) {
  if (tableCapacity == 0) {
    return 0;
  }
  uint8_t buffer[14] = {0};
  size_t inLen = strlen(input);
  size_t copyLen = (inLen < 14) ? inLen : 14;
  for (size_t i = 0; i < copyLen; ++i) {
    buffer[i] = (uint8_t)toupper((unsigned char)input[i]);
  }

  uint8_t key1[8], key2[8];
  expandSevenToEight(buffer, key1);
  expandSevenToEight(buffer + 7, key2);

  uint8_t c1[8], c2[8];
  encryptCoreBlock(FIXED_MAGIC_BLOCK, key1, c1);
  encryptCoreBlock(FIXED_MAGIC_BLOCK, key2, c2);

  uint64_t val = 0;
  for (size_t i = 0; i < 8; ++i) {
    val = (val << 8) | (c1[i] ^ c2[i]);
  }
  return (size_t)(val % tableCapacity);
}

/* -------------------------------------------------------------------------
 * Method 2: Division-based Hash Function
 * ------------------------------------------------------------------------- */
void computeDivisionDigest(const char *input, char *outHex) {
  const uint64_t prime1 = 1000000000000037ULL;
  const uint64_t prime2 = 1000000000000091ULL;
  const uint64_t mult1 = 131ULL;
  const uint64_t mult2 = 137ULL;

  uint64_t h1 = 0;
  uint64_t h2 = 0;
  for (size_t i = 0; input[i] != '\0'; ++i) {
    unsigned char ch = (unsigned char)input[i];
    h1 = (h1 * mult1 + (uint64_t)ch) % prime1;
    h2 = (h2 * mult2 + (uint64_t)ch) % prime2;
  }

  snprintf(outHex, 33, "%016llX%016llX", (unsigned long long)h1,
           (unsigned long long)h2);
  outHex[32] = '\0';
}

size_t computeDivisionIndex(const char *input, size_t tableCapacity) {
  if (tableCapacity == 0) {
    return 0;
  }
  const uint64_t primeVal = 1000000000000037ULL;
  const uint64_t multiplier = 131ULL;
  uint64_t h = 0;
  for (size_t i = 0; input[i] != '\0'; ++i) {
    unsigned char ch = (unsigned char)input[i];
    h = (h * multiplier + (uint64_t)ch) % primeVal;
  }
  return (size_t)(h % tableCapacity);
}

/* -------------------------------------------------------------------------
 * UserVault: Secure Password Storage System
 * ------------------------------------------------------------------------- */
typedef struct {
  char username[64];
  char passwordHash[65];
  char algorithm[32];
  char createdAt[32];
} UserRecord;

typedef struct {
  UserRecord *records;
  size_t count;
  size_t capacity;
} UserVault;

static void getCurrentTimestamp(char *buf, size_t bufSize) {
  time_t t = time(NULL);
  struct tm *lt = localtime(&t);
  if (lt != NULL) {
    strftime(buf, bufSize, "%Y-%m-%d %H:%M:%S", lt);
  } else {
    strncpy(buf, "2026-11-09 12:00:00", bufSize);
    buf[bufSize - 1] = '\0';
  }
}

void userVaultInit(UserVault *vault) {
  vault->count = 0;
  vault->capacity = 16;
  vault->records = (UserRecord *)malloc(vault->capacity * sizeof(UserRecord));
}

void userVaultFree(UserVault *vault) {
  if (vault->records != NULL) {
    free(vault->records);
    vault->records = NULL;
  }
  vault->count = 0;
  vault->capacity = 0;
}

bool userVaultAddUser(UserVault *vault, const char *username,
                      const char *password, int algoChoice) {
  for (size_t i = 0; i < vault->count; ++i) {
    if (strcmp(vault->records[i].username, username) == 0) {
      return false;
    }
  }

  if (vault->count >= vault->capacity) {
    size_t newCap = vault->capacity * 2;
    UserRecord *newRecs =
        (UserRecord *)realloc(vault->records, newCap * sizeof(UserRecord));
    if (newRecs == NULL) {
      return false;
    }
    vault->records = newRecs;
    vault->capacity = newCap;
  }

  UserRecord *rec = &vault->records[vault->count];
  strncpy(rec->username, username, sizeof(rec->username) - 1);
  rec->username[sizeof(rec->username) - 1] = '\0';

  if (algoChoice == 1) {
    computeMethodAlphaDigest(password, rec->passwordHash);
    strncpy(rec->algorithm, "Method 1", sizeof(rec->algorithm) - 1);
  } else {
    computeDivisionDigest(password, rec->passwordHash);
    strncpy(rec->algorithm, "Method 2 (Division)", sizeof(rec->algorithm) - 1);
  }
  rec->algorithm[sizeof(rec->algorithm) - 1] = '\0';

  getCurrentTimestamp(rec->createdAt, sizeof(rec->createdAt));
  vault->count++;
  return true;
}

bool userVaultAuthenticate(const UserVault *vault, const char *username,
                           const char *password) {
  for (size_t i = 0; i < vault->count; ++i) {
    if (strcmp(vault->records[i].username, username) == 0) {
      char checkHash[65];
      if (strcmp(vault->records[i].algorithm, "Method 1") == 0) {
        computeMethodAlphaDigest(password, checkHash);
      } else {
        computeDivisionDigest(password, checkHash);
      }
      return (strcmp(checkHash, vault->records[i].passwordHash) == 0);
    }
  }
  return false;
}

/* -------------------------------------------------------------------------
 * Benchmark Data Structures & Chaining Hash Table
 * ------------------------------------------------------------------------- */
typedef struct HashNode {
  char *key;
  struct HashNode *next;
} HashNode;

typedef struct {
  size_t capacity;
  size_t totalKeys;
  size_t collisions;
  size_t maxChain;
  size_t emptyBuckets;
  double loadFactor;
  double standardDeviation;
  double elapsedMs;
} BenchmarkMetrics;

BenchmarkMetrics evaluateAlgorithm(int algoChoice, char **dataset, size_t count,
                                   size_t capacity) {
  HashNode **buckets = (HashNode **)calloc(capacity, sizeof(HashNode *));
  size_t collisions = 0;

  double t0 = getTimeMs();
  for (size_t i = 0; i < count; ++i) {
    size_t idx = (algoChoice == 1)
                     ? computeMethodAlphaIndex(dataset[i], capacity)
                     : computeDivisionIndex(dataset[i], capacity);
    if (buckets[idx] != NULL) {
      collisions++;
    }
    HashNode *newNode = (HashNode *)malloc(sizeof(HashNode));
    newNode->key = dataset[i];
    newNode->next = buckets[idx];
    buckets[idx] = newNode;
  }
  double t1 = getTimeMs();
  double elapsed = t1 - t0;

  size_t maxChain = 0;
  size_t emptyBuckets = 0;
  double sum = 0.0;
  size_t *chainLengths = (size_t *)malloc(capacity * sizeof(size_t));

  for (size_t i = 0; i < capacity; ++i) {
    size_t sz = 0;
    HashNode *cur = buckets[i];
    while (cur != NULL) {
      sz++;
      cur = cur->next;
    }
    chainLengths[i] = sz;
    if (sz == 0) {
      emptyBuckets++;
    }
    if (sz > maxChain) {
      maxChain = sz;
    }
    sum += (double)sz;
  }

  double mean = sum / (double)capacity;
  double varianceSum = 0.0;
  for (size_t i = 0; i < capacity; ++i) {
    double diff = (double)chainLengths[i] - mean;
    varianceSum += diff * diff;
  }
  double stdDev = sqrt(varianceSum / (double)capacity);
  free(chainLengths);

  /* Free hash table chain nodes */
  for (size_t i = 0; i < capacity; ++i) {
    HashNode *cur = buckets[i];
    while (cur != NULL) {
      HashNode *temp = cur;
      cur = cur->next;
      free(temp);
    }
  }
  free(buckets);

  BenchmarkMetrics m;
  m.capacity = capacity;
  m.totalKeys = count;
  m.collisions = collisions;
  m.maxChain = maxChain;
  m.emptyBuckets = emptyBuckets;
  m.loadFactor = (double)count / (double)capacity;
  m.standardDeviation = stdDev;
  m.elapsedMs = elapsed;
  return m;
}

/* -------------------------------------------------------------------------
 * Pseudo-random Dataset Generator
 * ------------------------------------------------------------------------- */
static uint64_t g_rngState = 42;

static uint32_t fastRandom32(void) {
  g_rngState = g_rngState * 6364136223846793005ULL + 1442695040888963407ULL;
  return (uint32_t)(g_rngState >> 32);
}

static uint32_t randomRange(uint32_t minVal, uint32_t maxVal) {
  return minVal + (fastRandom32() % (maxVal - minVal + 1));
}

char **generateDataset(int type, size_t count) {
  char **dataset = (char **)malloc(count * sizeof(char *));
  if (type == 1) {
    for (size_t i = 0; i < count; ++i) {
      char buf[64];
      snprintf(buf, sizeof(buf), "record_%zu", 100000 + i);
      dataset[i] = (char *)malloc(strlen(buf) + 1);
      strcpy(dataset[i], buf);
    }
  } else if (type == 2) {
    static const char alphabet[] =
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    size_t alphaLen = sizeof(alphabet) - 1;
    g_rngState = 42;
    for (size_t i = 0; i < count; ++i) {
      size_t len = randomRange(8, 14);
      char *s = (char *)malloc(len + 1);
      for (size_t j = 0; j < len; ++j) {
        s[j] = alphabet[randomRange(0, (uint32_t)(alphaLen - 1))];
      }
      s[len] = '\0';
      dataset[i] = s;
    }
  } else {
    static const char digits[] = "0123456789";
    g_rngState = 1337;
    for (size_t i = 0; i < count; ++i) {
      size_t len = randomRange(4, 8);
      char *s = (char *)malloc(len + 1);
      for (size_t j = 0; j < len; ++j) {
        s[j] = digits[randomRange(0, 9)];
      }
      s[len] = '\0';
      dataset[i] = s;
    }
  }
  return dataset;
}

void freeDataset(char **dataset, size_t count) {
  for (size_t i = 0; i < count; ++i) {
    free(dataset[i]);
  }
  free(dataset);
}

/* -------------------------------------------------------------------------
 * Metric Row Printing
 * ------------------------------------------------------------------------- */
void printMetricsRow(const char *name, const BenchmarkMetrics *m) {
  char bufKeys[32], bufCol[32], bufMax[32], bufEmp[32];
  char bufLf[32], bufSd[32], bufTime[32];

  snprintf(bufKeys, sizeof(bufKeys), "%zu", m->totalKeys);
  snprintf(bufCol, sizeof(bufCol), "%zu", m->collisions);
  snprintf(bufMax, sizeof(bufMax), "%zu", m->maxChain);
  snprintf(bufEmp, sizeof(bufEmp), "%zu", m->emptyBuckets);
  snprintf(bufLf, sizeof(bufLf), "%.3f", m->loadFactor);
  snprintf(bufSd, sizeof(bufSd), "%.3f", m->standardDeviation);
  snprintf(bufTime, sizeof(bufTime), "%.2f", m->elapsedMs);

  printCell(name, 22, false);
  printCell(bufKeys, 10, true);
  printCell(bufCol, 12, true);
  printCell(bufMax, 12, true);
  printCell(bufEmp, 12, true);
  printCell(bufLf, 14, true);
  printCell(bufSd, 14, true);
  printCell(bufTime, 12, true);
  putchar('\n');
}

/* -------------------------------------------------------------------------
 * Automated Benchmark Execution
 * ------------------------------------------------------------------------- */
void runAutomatedBenchmark(void) {
  puts("\n==================================================================="
       "======================");
  puts("                 COMPARATIVE ANALYSIS OF HASH FUNCTIONS AND COLLISIONS "
       "                  ");
  puts("====================================================================="
       "====================");

  size_t capacities[] = {1000, 2000, 5000};
  int dataTypes[] = {1, 2, 3};
  const char *dataNames[] = {"Sequential Identifiers",
                             "Random Passwords (8-14)", "Short Numeric Keys"};

  for (size_t d = 0; d < 3; ++d) {
    printf("\n>>> Dataset: %s\n", dataNames[d]);
    for (size_t c = 0; c < 3; ++c) {
      size_t cap = capacities[c];
      size_t fillCount = (size_t)(cap * 0.65);
      char **data = generateDataset(dataTypes[d], fillCount);

      BenchmarkMetrics m1 = evaluateAlgorithm(1, data, fillCount, cap);
      BenchmarkMetrics m2 = evaluateAlgorithm(2, data, fillCount, cap);

      printf("\n--- Table size N = %zu, elements K = %zu (fill factor ~65%%) "
             "---\n",
             cap, fillCount);
      printCell("Algorithm", 22, false);
      printCell("Keys", 10, true);
      printCell("Collisions", 12, true);
      printCell("Max Chain", 12, true);
      printCell("Empty", 12, true);
      printCell("LoadFactor", 14, true);
      printCell("StdDev", 14, true);
      printCell("Time (ms)", 12, true);
      putchar('\n');

      for (int i = 0; i < 108; ++i) {
        putchar('-');
      }
      putchar('\n');

      printMetricsRow("Method 1", &m1);
      printMetricsRow("Method 2 (Division)", &m2);

      freeDataset(data, fillCount);
    }
  }
}

/* -------------------------------------------------------------------------
 * Automated Self-Test
 * ------------------------------------------------------------------------- */
void runSelfTest(UserVault *vault) {
  puts("\n=========================================================");
  puts("                 AUTOMATED FUNCTIONAL TEST               ");
  puts("=========================================================");

  puts("\n1. Verification of hash calculation for test vectors:");
  const char *testStrs[] = {"", "password", "welcome", "Admin2026!",
                            "very_long_input_string_exceeding_14_chars"};
  for (size_t i = 0; i < 5; ++i) {
    char h1[65], h2[65];
    computeMethodAlphaDigest(testStrs[i], h1);
    computeDivisionDigest(testStrs[i], h2);
    printf("Input: \"%s\"\n", testStrs[i]);
    printf("  Method 1:           %s\n", h1);
    printf("  Method 2 (Division): %s\n", h2);
  }

  puts("\n2. Verification of secure password storage system:");
  userVaultAddUser(vault, "admin", "AdminSecret123", 1);
  userVaultAddUser(vault, "yaroslav", "StudPassword!2026", 2);
  userVaultAddUser(vault, "guest", "12345678", 1);
  puts("Three test users added successfully.");

  puts("\n3. Authentication checks:");
  printf("  admin with correct password:    %s\n",
         userVaultAuthenticate(vault, "admin", "AdminSecret123")
             ? "SUCCESS (access granted)"
             : "ERROR");
  printf("  admin with incorrect password:  %s\n",
         userVaultAuthenticate(vault, "admin", "WrongPass")
             ? "ERROR"
             : "REJECTED (access denied)");
  printf("  yaroslav with correct password: %s\n",
         userVaultAuthenticate(vault, "yaroslav", "StudPassword!2026")
             ? "SUCCESS (access granted)"
             : "ERROR");
  printf("  yaroslav with wrong password:   %s\n",
         userVaultAuthenticate(vault, "yaroslav", "studpassword!2026")
             ? "ERROR"
             : "REJECTED (case-sensitive)");

  puts("\n4. Stored credentials table (protected view):");
  printCell("User", 15, false);
  printCell("Algorithm", 22, false);
  printCell("Password Hash", 36, false);
  printCell("Registration Time", 22, false);
  putchar('\n');
  for (int i = 0; i < 95; ++i) {
    putchar('-');
  }
  putchar('\n');

  for (size_t i = 0; i < vault->count; ++i) {
    printCell(vault->records[i].username, 15, false);
    printCell(vault->records[i].algorithm, 22, false);
    printCell(vault->records[i].passwordHash, 36, false);
    printCell(vault->records[i].createdAt, 22, false);
    putchar('\n');
  }

  puts("\nAutomated test completed successfully!");
}

/* -------------------------------------------------------------------------
 * Safe Line Input Helper
 * ------------------------------------------------------------------------- */
static void readLineSafe(char *buf, size_t size) {
  if (fgets(buf, (int)size, stdin) != NULL) {
    size_t len = strlen(buf);
    if (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r')) {
      buf[len - 1] = '\0';
    }
    len = strlen(buf);
    if (len > 0 && (buf[len - 1] == '\r')) {
      buf[len - 1] = '\0';
    }
  } else {
    buf[0] = '\0';
  }
}

/* -------------------------------------------------------------------------
 * Main Function & Menu Loop
 * ------------------------------------------------------------------------- */
int main(void) {
#if defined(_WIN32)
  SetConsoleOutputCP(65001);
  SetConsoleCP(65001);
#endif

  UserVault vault;
  userVaultInit(&vault);

  int choice = -1;
  char lineBuf[256];

  while (choice != 0) {
    puts("\n=========================================================");
    puts("                        MAIN MENU                        ");
    puts("=========================================================");
    puts("1. Compute string hash");
    puts("2. Register new user (store password hash)");
    puts("3. Authenticate user (verify password by hash)");
    puts("4. View password table in protected view");
    puts("5. Comparative analysis of distribution and collisions (experiment)");
    puts("6. Run complete automated demonstration test");
    puts("0. Exit");
    printf("Your choice: ");
    fflush(stdout);

    if (fgets(lineBuf, sizeof(lineBuf), stdin) == NULL) {
      break;
    }
    if (sscanf(lineBuf, "%d", &choice) != 1) {
      choice = -1;
      continue;
    }

    if (choice == 1) {
      char input[256];
      printf("Enter string to hash: ");
      fflush(stdout);
      readLineSafe(input, sizeof(input));

      char h1[65], h2[65];
      computeMethodAlphaDigest(input, h1);
      computeDivisionDigest(input, h2);

      printf("\nInput string: \"%s\"\n", input);
      printf("Result Method 1:           %s\n", h1);
      printf("Result Method 2 (Division): %s\n", h2);
    } else if (choice == 2) {
      char uname[64], pwd[64], algoStr[32];
      int algo = 1;

      printf("Enter username: ");
      fflush(stdout);
      readLineSafe(uname, sizeof(uname));

      printf("Enter password: ");
      fflush(stdout);
      readLineSafe(pwd, sizeof(pwd));

      printf("Select hashing algorithm (1 - Method 1, 2 - Method 2): ");
      fflush(stdout);
      readLineSafe(algoStr, sizeof(algoStr));
      if (sscanf(algoStr, "%d", &algo) != 1 || (algo != 1 && algo != 2)) {
        algo = 1;
      }

      if (userVaultAddUser(&vault, uname, pwd, algo)) {
        printf("User \"%s\" successfully registered (password protected with "
               "hash)!\n",
               uname);
      } else {
        puts("Error: user with this username already exists!");
      }
    } else if (choice == 3) {
      char uname[64], pwd[64];
      printf("Enter username: ");
      fflush(stdout);
      readLineSafe(uname, sizeof(uname));

      printf("Enter password: ");
      fflush(stdout);
      readLineSafe(pwd, sizeof(pwd));

      if (userVaultAuthenticate(&vault, uname, pwd)) {
        puts(">>> Authentication successful! Password correct. Access granted. "
             "<<<");
      } else {
        puts(">>> Authentication failed! Invalid username or password. Access "
             "denied. <<<");
      }
    } else if (choice == 4) {
      if (vault.count == 0) {
        puts("Password vault is empty. Register users or run automated test.");
      } else {
        puts("\nStored user accounts (plaintext passwords absent):");
        printCell("User", 15, false);
        printCell("Algorithm", 22, false);
        printCell("Password Hash", 36, false);
        printCell("Registration Time", 22, false);
        putchar('\n');
        for (int i = 0; i < 95; ++i) {
          putchar('-');
        }
        putchar('\n');

        for (size_t i = 0; i < vault.count; ++i) {
          printCell(vault.records[i].username, 15, false);
          printCell(vault.records[i].algorithm, 22, false);
          printCell(vault.records[i].passwordHash, 36, false);
          printCell(vault.records[i].createdAt, 22, false);
          putchar('\n');
        }
      }
    } else if (choice == 5) {
      runAutomatedBenchmark();
    } else if (choice == 6) {
      runSelfTest(&vault);
    } else if (choice == 0) {
      puts("Program terminated.");
    } else {
      puts("Invalid menu option. Please try again.");
    }
  }

  userVaultFree(&vault);
  return 0;
}
