# Gunslinger Containers
This document shows off all available containers and data structures in gunslinger.

## Dynamic Array

`gs_dyn_array` is a generic, dynamic array of type T, which is defined by the user and is inspired GREATLY from Shawn Barret's [stretchy buffer](https://github.com/nothings/stb/blob/master/stretchy_buffer.h) implementation:

```c
gs_dyn_array(float) arr = NULL;  // Dyanmic array of type float
```

Since c99 does not provide templates, generics must be achieved using macros. `gs_dyn_array(T)` is a macro that evalutes to `T*`. The user must initialize it to `NULL` before using it, that way the underlying implementation knows to internally initialize it.

`gs_dyn_array` stores header information right before the actual array in memory for a table to describe properties of the array: 

```txt
array => [header_info][actual array data]
```

Where the header is a define as:
```c
typedef struct gs_array_header_t {
  uint32_t size;
  uint32_t capacity;
} gs_array_header_t;
```
The user maintains a pointer to the beginning of the data array in memory at all times.

Since `gs_dyn_array` evaluates to a simple pointer of type `T`, the array can be randomly accessed using the `[]` operator as you would with any stadnard c array. 

```c
float v = arr[v];
```

There are also provided functions for accessing information using this provided table. `gs_dyn_array` is the baseline for all other containers provided in gunslinger.

## Dynamic Array API:
* Creating/Deleting: 
```c
gs_dyn_array(T) arr = NULL;   // Create dynamic array of type T.
gs_dyn_array_free(arr);       // Frees array data calling `gs_free()` internally.
```
* Inserting/Accessing data: 
```c
gs_dyn_array_push(array, T);    // Push data of type `T` into array
T val = array[i];               // Access data of type `T` at index `i`
T* valp = &array[i];            // Access pointer of data type `T` at index `i`
```
* Size/Capacity/Empty/Reserve/Clear:
```c
uint32_t sz = gs_dyn_array_size(arr);       // Gets size of array. Return 0 if arr is NULL.
uint32_t cap = gs_dyn_array_capacity(arr);  // Gets capacity of array. Return 0 if arr is NULL.
bool is_empty = gs_dyn_array_empty(arr);    // Returns whether array is empty. Return true if arr is NULL.
gs_dyn_array_reserve(arr, N);               // Reserves internal space in the array for N (uint32_t), non-initialized elements.
gs_dyn_array_clear(arr);                    // Clears all elements. Simply sets array size to 0.
```
* Iterating data: 
```c
for (uint32_t i = 0; i < gs_dyn_array_size(arr); ++i) {     // Iterate size of array, access elements via index `i`
  T* vp = &arr[i];
}
```
## Hash Table
`gs_hash_table` is a generic hash table of key `K` and value `V`, and is inspired by Shawn Barret's [ds](https://github.com/nothings/stb/blob/master/stb_ds.h) library:
```c
gs_hash_table(uint32_t, float) ht = NULL;   // Declares a hash table with K = uint32_t, V = float
```
A hash table is a collection of unamed struct data that is defined when you declare the structure with the `gs_hash_table(K, V)` macro. 
```c
#define gs_hash_table(__HMK, __HMV)\
    struct {\
        __gs_hash_table_entry(__HMK, __HMV)* data;\
        __HMK tmp_key;\
        __HMV tmp_val;\
        size_t stride;\
        size_t klpvl;\
    }*
```

Where the data is a dynamic array of `__gs_hash_table_entry(K, V)`. These entries store key, value pairs as well as whether or not the entry is `active` or `inactive`. Since the data is a contiguous array, `gs_hash_table` uses open addressing via quadratic probing to search for keys. 

Internally, the hash table uses a 64-bit siphash to hash generic byte data to an unsigned 64-bit key. This means it's possible to pass up arbitrary data to the hash table and it will hash accordingly, such as structs:
```c
typedef struct key_t {
  uint32_t id0;
  uint64_t id1;
} key_t;

gs_hash_table(key_t, float) ht = NULL;    // Create hash table with K = key_t, V = float
```

Inserting into the hash table with these "complex" types is as simple as: 
```c
key_t k = {.ido0 = 5, .id1 = 32};     // Create structure for "key"
gs_hash_table_insert(ht, k, 5.f);     // Insert into hash table using key
float v = gs_hash_table_get(ht, k);   // Get data at key
```

Note: It is possible to return a reference to the data using `gs_hash_table_getp()`. However, keep in mind that this comes with the danger that the reference could be lost *IF* the internal data array grows or shrinks in between you caching the pointer and using it.
```c                
float* val = gs_hash_table_getp(ht, k);    // Cache pointer to internal data. Dangerous game.
gs_hash_table_insert(ht, new_key);         // At this point, your pointer could be invalidated due to growing internal array.
```
## Hash Table API: 

* Creating/Deleting
```c
gs_hash_table(K, V) ht = NULL;    // Create hash table with key = K, val = V
gs_hash_table_free(ht);           // Frees hash table internal data calling `gs_free()` internally.
```
* Inserting/Accessing data: 
```c
// Insert key/val pair {K, V} into hash table. Will dynamically grow/init on demand. 
gs_hash_table_insert(ht, K, V);

// Use to query whether or not a key exists in the table. Returns true if exists, false if doesn't.
bool exists = gs_hash_table_key_exists(ht, K); 

// Get value V at key = K. NOTE: Will crash due to access exemption if key not available. 
V val = gs_hash_table_get(ht, K);               

// Get pointer reference to data at key = K. NOTE: Will crash due to access exemption if key not available.
V* valp = gs_hash_table_get(ht, K);             
```
* Size/Capacity/Empty/Reserve/Clear:
```c
uint32_t sz = gs_hash_table_size(ht);           // Get size of hash table. Returns 0 if ht is NULL.               
uint32_t cap = gs_hash_table_capacity(ht);      // Get capacity of hash table. Returns 0 if ht is NULL.
bool is_empty = gs_hash_table_empty(ht);        // Returns whether hash table is empty. Returns true if ht NULL.
gs_hash_table_clear(ht);                        // Clears all elements. Sets size to 0.
```
* Iterating data:
Hash table provides an `stl-style` iterator api using `gs_hash_table_iter`. You can use this iterator in for/while loops to iterate cleanly over valid data.
```c
// Using for loop
for (
  gs_hash_table_iter it = gs_hash_table_iter_new(ht);   // Creates new iterator
  gs_hash_table_iter_valid(ht, it);                     // Checks whether the iterator is valid each loop iteration
  gs_hash_table_iter_advance(ht, it)                    // Advances the iterator position internally
) 
{
  V val = gs_hash_table_iter_get(ht, it);         // Get value using iterator
  V* valp = gs_hash_table_iter_getp(ht, it);      // Get value pointer using iterator
  K key = gs_hash_table_iter_get_key(ht, it);     // Get key using iterator
  K* keyp = gs_hash_table_iter_get_keyp(ht, it);  // Get key pointer using iterator
}

// Using while loop
gs_hash_table_iter it = gs_hash_table_iter_new(ht);
while (gs_hash_table_iter_valid(ht, it))
{
   // Do stuff with iterator like in for loop example
   gs_hash_table_iter_advance(ht, it);
}
```
## Slot Array

## Slot Map

## Byte Buffer

## Command Buffer
