#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define sizeof_field(type, field) (sizeof(((type*)0)->field))

#define SHORT_STRING_CAPACITY    sizeof_field(struct Short_String, data)
#define GERMAN_STRING_PREFIX_LEN sizeof_field(union German_String, prefix)


typedef union German_String {
    // Combination of size and prefix
    uint64_t eq_hash;

    struct {
        uint32_t size;
        uint32_t prefix;
    };

    struct Short_String {
        uint32_t size;
        char     data[16 - sizeof(size)];
    } s_str;

    struct Long_String {
        uint32_t size;
        char     prefix_[16 - sizeof(char*) - sizeof(size)];
        char*    data;
    } l_str;
} German_String;


typedef struct Str {
    size_t      size;
    const char* data;
} Str;


int str_cmp(Str a, Str b) {
    int size = a.size < b.size ? a.size : b.size;
    int ordering = memcmp(a.data, b.data, size);
    return ordering != 0 ? ordering : (int)(a.size - b.size);
}


int str_eq(Str a, Str b) {
    return a.size != b.size || memcmp(a.data, b.data, a.size) == 0;
}


German_String german_string_new(const char* str, size_t size) {
    assert(size <= 1ULL << 32ULL);
    if (size <= SHORT_STRING_CAPACITY) {
        German_String short_string = { 0 };
        short_string.s_str.size = size;
        memcpy(short_string.s_str.data, str, size);
        return short_string;
    } else {
        German_String long_string = { 0 };
        void* data = malloc(size);
        if (data == NULL) {
            return long_string;
        }
        long_string.l_str.size = size;
        long_string.l_str.data = data;
        memcpy(long_string.l_str.data, str, size);
        memcpy(long_string.l_str.prefix_, str, GERMAN_STRING_PREFIX_LEN);
        return long_string;
    }
}


void german_string_free(German_String* str) {
    if (str->size > SHORT_STRING_CAPACITY) {
        free(str->l_str.data);
    }
    *str = (German_String) { 0 };
}


int german_string_cmp(German_String a, German_String b) {
    if (a.prefix != b.prefix) {
        return (int)(a.prefix - b.prefix);
    } else {
        const char* a_str = a.size <= SHORT_STRING_CAPACITY ? a.s_str.data : a.l_str.data;
        const char* b_str = b.size <= SHORT_STRING_CAPACITY ? b.s_str.data : b.l_str.data;
        int size = a.size < b.size ? a.size : b.size;
        int ordering = memcmp(a_str, b_str, size);
        return ordering != 0 ? ordering : (int)(a.size - b.size);
    }
}


int german_string_eq(German_String a, German_String b) {
    if (a.eq_hash != b.eq_hash) {
        return 0;
    } else if (a.size <= SHORT_STRING_CAPACITY) {
        return memcmp(a.s_str.data, b.s_str.data, a.size) == 0;
    } else {
        return memcmp(a.l_str.data, b.l_str.data, a.size) == 0;
    }
}


Str german_string_to_slice(const German_String* str) {
    const char* data = str->size <= SHORT_STRING_CAPACITY ? str->s_str.data : str->l_str.data;
    return (Str) { (size_t) str->size, data };
}


#ifdef STRINGS_TEST
#include <assert.h>

int main(void) {
    German_String a0 = german_string_new("Hello World", 11);        // Short
    German_String b0 = german_string_new("Hello World 123", 15);    // Long
    German_String a1 = german_string_new("Hello Earth", 11);        // Short
    German_String b1 = german_string_new("Hello Earth 123", 15);    // Long

    Str sv_a0 = german_string_to_slice(&a0);
    Str sv_b0 = german_string_to_slice(&b0);
    Str sv_a1 = german_string_to_slice(&a1);
    Str sv_b1 = german_string_to_slice(&b1);

    // Equality tests
    assert(german_string_eq(a0, a0) == 1);
    assert(german_string_eq(a0, b0) == 0);
    assert(german_string_eq(a0, a1) == 0);
    assert(german_string_eq(a0, b1) == 0);

    assert(german_string_eq(b0, b0) == 1);
    assert(german_string_eq(b0, a1) == 0);
    assert(german_string_eq(b0, b1) == 0);

    assert(german_string_eq(a1, a1) == 1);
    assert(german_string_eq(a1, b1) == 0);

    assert(german_string_eq(b1, b1) == 1);

    // Comparison tests
    assert(german_string_cmp(a0, a0) == 0);
    assert(german_string_cmp(a0, b0) < 0);
    assert(german_string_cmp(a0, a1) > 0);
    assert(german_string_cmp(a0, b1) > 0);

    assert(german_string_cmp(b0, a0) > 0);
    assert(german_string_cmp(b0, b0) == 0);
    assert(german_string_cmp(b0, a1) > 0);
    assert(german_string_cmp(b0, b1) > 0);

    assert(german_string_cmp(a1, a0) < 0);
    assert(german_string_cmp(a1, b0) < 0);
    assert(german_string_cmp(a1, a1) == 0);
    assert(german_string_cmp(a1, b1) < 0);

    assert(german_string_cmp(b1, a0) < 0);
    assert(german_string_cmp(b1, b0) < 0);
    assert(german_string_cmp(b1, a1) > 0);
    assert(german_string_cmp(b1, b1) == 0);

    // Clean up
    german_string_free(&a0);
    german_string_free(&b0);
    german_string_free(&a1);
    german_string_free(&b1);
}
#endif
