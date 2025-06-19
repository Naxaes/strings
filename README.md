# German String

A C implementation of a German String, that implements short string optimization, and fast comparisons.

## Features

- **Short String Optimization**: Strings up to 12 bytes are stored inline without any allocation.
- **Efficient Comparison**: Fast equality and ordering checks using inline prefix and size comparison for both short and long strings.

## API

### `German_String german_string_new(const char* str, size_t size);`
Creates a new `German_String` from a C string and its size.

### `void german_string_free(German_String* str);`
Frees memory used by a long string. No-op for short strings.

### `int german_string_cmp(German_String a, German_String b);`
Compares two `German_String` values lexicographically.

### `int german_string_eq(German_String a, German_String b);`
Checks if two `German_String` values are equal.

### `Str german_string_to_slice(const German_String* str);`
Converts a `German_String` to a `Str` view.

## Usage

To compile and run the test/bench suite:

```bash
cc -DSTRINGS_TEST -o string_test strings.c
cc -o string_bench -O2 test.c
./string_test
./string_bench
```
