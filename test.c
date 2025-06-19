#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <stdint.h>
#include <math.h>


#define MAX_STRING_LENGTH 64
#define TEST_CASES 1000
#define TEST_ITERATIONS 10000

#include "strings.c"

// Random string generator
void random_string(char* buffer, int length) {
    static const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 ";
    for (int i = 0; i < length; ++i) {
        buffer[i] = charset[rand() % (sizeof(charset) - 1)];
    }
}


int biased_random(int max, double exponent) {
    double r = (double)rand() / RAND_MAX;
    double biased = pow(r, exponent);
    return (int)(biased * max) + 1;
}


// Benchmark timer
double now_seconds() {
    struct timespec start;
    if (clock_gettime(CLOCK_REALTIME, &start) == -1) {
        perror( "clock gettime" );
        return EXIT_FAILURE;
    }

    return (double)start.tv_sec + (double)start.tv_nsec / 1000000000.0;
}


int compare_doubles(const void* a, const void* b) {
    double diff = *(double*)a - *(double*)b;
    return (diff > 0) - (diff < 0);
}


__attribute__((noinline))
void test_german_string(char* buf_a, char* buf_b, int len_a, int len_b) {
    German_String a = german_string_new(buf_a, len_a);
    German_String b = german_string_new(buf_b, len_b);

    int cmp_ab = german_string_cmp(a, b);
    int cmp_ba = german_string_cmp(b, a);
    int eq_ab  = german_string_eq(a, b);
    int eq_ba  = german_string_eq(b, a);

    // Reflexivity
    assert(german_string_cmp(a, a) == 0);
    assert(german_string_cmp(b, b) == 0);
    assert(german_string_eq(a, a) == 1);
    assert(german_string_eq(b, b) == 1);

    // Symmetry
    if (cmp_ab != 0) {
        assert(cmp_ab == -cmp_ba);
    } else {
        assert(eq_ab == 1 && eq_ba == 1);
    }

    // Optional: print mismatches
    if (eq_ab != eq_ba) {
        printf("Mismatch in equality symmetry:\n  A: %.*s\n  B: %.*s\n", len_a, buf_a, len_b, buf_b);
    }

    german_string_free(&a);
    german_string_free(&b);
}


__attribute__((noinline))
void test_standard_string(char* buf_a, char* buf_b, int len_a, int len_b) {
    void* a_data = malloc(len_a);
    memcpy(a_data, buf_a, len_a);

    void* b_data = malloc(len_b);
    memcpy(b_data, buf_b, len_b);

    Str a = { .size = len_a, .data = a_data};
    Str b = { .size = len_b, .data = b_data };

    int cmp_ab = str_cmp(a, b);
    int cmp_ba = str_cmp(b, a);
    int eq_ab  = str_eq(a, b);
    int eq_ba  = str_eq(b, a);

    // Reflexivity
    assert(str_cmp(a, a) == 0);
    assert(str_cmp(b, b) == 0);
    assert(str_eq(a, a) == 1);
    assert(str_eq(b, b) == 1);

    // Symmetry
    if (cmp_ab != 0) {
        assert(cmp_ab == -cmp_ba);
    } else {
        assert(eq_ab == 1 && eq_ba == 1);
    }

    // Optional: print mismatches
    if (eq_ab != eq_ba) {
        printf("Mismatch in equality symmetry:\n  A: %.*s\n  B: %.*s\n", len_a, buf_a, len_b, buf_b);
    }

    free((void*) a.data);
    free((void*) b.data);
}


void report_timing(double * timings) {
    qsort(timings, TEST_CASES, sizeof(double), compare_doubles);

    double min = timings[0];
    double max = timings[TEST_CASES - 1];
    double sum = 0.0;
    for (int i = 0; i < TEST_CASES; ++i) {
        sum += timings[i];
    }
    double avg = sum / TEST_CASES;

    double q10 = timings[(int)(0.10 * TEST_CASES)];
    double q25 = timings[(int)(0.25 * TEST_CASES)];
    double q50 = timings[(int)(0.50 * TEST_CASES)];
    double q75 = timings[(int)(0.75 * TEST_CASES)];
    double q90 = timings[(int)(0.90 * TEST_CASES)];

    printf("\nCompleted %d test cases in %.3f seconds.\n", TEST_CASES, sum);
    printf("Timing Statistics (in seconds):\n");
    printf("Min:    %.6f\n", min);
    printf("Max:    %.6f\n", max);
    printf("Avg:    %.6f\n", avg);
    printf("10%%:    %.6f\n", q10);
    printf("25%%:    %.6f\n", q25);
    printf("50%%:    %.6f (Median)\n", q50);
    printf("75%%:    %.6f\n", q75);
    printf("90%%:    %.6f\n", q90);
}

int main(void) {
    srand((unsigned int)time(NULL));

    double* timings1 = malloc(TEST_CASES * sizeof(double));
    double* timings2 = malloc(TEST_CASES * sizeof(double));
    int timing_counter = 0;

    for (int i = 0; i < TEST_CASES; ++i) {
        int len_a = biased_random(MAX_STRING_LENGTH, 2.0);
        int len_b = biased_random(MAX_STRING_LENGTH, 2.0);

        char buf_a[MAX_STRING_LENGTH + 1] = {0};
        char buf_b[MAX_STRING_LENGTH + 1] = {0};

        random_string(buf_a, len_a);
        random_string(buf_b, len_b);

        {
            __compiler_barrier();
            double t0 = now_seconds();
            for (int j = 0; j < TEST_ITERATIONS; ++j)
                test_german_string(buf_a, buf_b, len_a, len_b);
            double t1 = now_seconds();

            timings1[timing_counter] = t1 - t0;
        }

        {
            __compiler_barrier();
            double t0 = now_seconds();
            for (int j = 0; j < TEST_ITERATIONS; ++j)
                test_standard_string(buf_a, buf_b, len_a, len_b);
            double t1 = now_seconds();

            timings2[timing_counter] = t1 - t0;
        }

        timing_counter += 1;
    }

    printf("\n---- German string ----\n");
    report_timing(timings1);
    printf("\n---- Regular string ----\n");
    report_timing(timings2);

    return 0;
}
