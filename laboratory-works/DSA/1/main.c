#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE 20

int main(void) {
    int sizeInput = 0;
    double vector[MAX_SIZE];
    double *ptr = NULL;
    double sum = 0.0;
    double average = 0.0;
    int countAboveAverage = 0;

    printf("Enter the number of vector elements (from 1 to %d): ", MAX_SIZE);
    if (scanf("%d", &sizeInput) != 1 || sizeInput <= 0 || sizeInput > MAX_SIZE) {
        printf("\n[Error]: Invalid vector size! Expected an integer 1-%d.\n", MAX_SIZE);
        system("pause");
        return 1;
    }

    printf("\nEnter %d real numbers (e.g. 2.5):\n", sizeInput);
    for (ptr = vector; ptr < vector + sizeInput; ptr++) {
        printf("vector[%d] = ", (int)(ptr - vector));
        if (scanf("%lf", ptr) != 1) {
            printf("\n[Error]: Invalid element value! Enter numbers using dot as decimal separator (e.g. 2.5).\n");
            system("pause");
            return 1;
        }
    }

    for (ptr = vector; ptr < vector + sizeInput; ptr++) {
        sum += *ptr;
    }

    average = sum / sizeInput;

    for (ptr = vector; ptr < vector + sizeInput; ptr++) {
        if (*ptr > average) {
            countAboveAverage++;
        }
    }

    printf("\n----------------------- RESULTS -----------------------\n");
    printf("Vector: [ ");
    for (ptr = vector; ptr < vector + sizeInput; ptr++) {
        printf("%.2f ", *ptr);
    }
    printf("]\n\n");

    printf("Sum of elements:                   %.4f\n", sum);
    printf("Arithmetic mean (average):         %.4f\n", average);
    printf("Number of elements > average:      %d\n", countAboveAverage);

    if (countAboveAverage > 0) {
        printf("Elements greater than average:     ");
        for (ptr = vector; ptr < vector + sizeInput; ptr++) {
            if (*ptr > average) {
                printf("%.2f (at index %d)  ", *ptr, (int)(ptr - vector));
            }
        }
        printf("\n");
    } else {
        printf("All elements are equal, none exceeds the average.\n");
    }
    printf("-------------------------------------------------------\n\n");

    system("pause");
    return 0;
}
