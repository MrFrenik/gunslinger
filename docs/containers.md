# Gunslinger Containers
This document shows off all available containers and data structures in gunslinger.

## Contents: 
* Dynamic Array:  [Overview](https://github.com/MrFrenik/gunslinger/blob/master/docs/containers.md#dynamic-array) | [API](https://github.com/MrFrenik/gunslinger/blob/master/docs/containers.md#dynamic-array-api)
* Hash Table:     [Overview](https://github.com/MrFrenik/gunslinger/blob/master/docs/containers.md#hash-table) | [API](https://github.com/MrFrenik/gunslinger/blob/master/docs/containers.md#hash-table-api) 
* Slot Array:     [Overview](https://github.com/MrFrenik/gunslinger/blob/master/docs/containers.md#slot-array) | [API](https://github.com/MrFrenik/gunslinger/blob/master/docs/containers.md#slot-array-api)
* Slot Map:       [Overview](https://github.com/MrFrenik/gunslinger/blob/master/docs/containers.md#slot-map) | [API](https://github.com/MrFrenik/gunslinger/blob/master/docs/containers.md#slot-map-api)
* Byte Buffer:    [Overview](https://github.com/MrFrenik/gunslinger/blob/master/docs/containers.md#byte-buffer) | [API](https://github.com/MrFrenik/gunslinger/blob/master/docs/containers.md#byte-buffer-api)
* Command Buffer: [Overview](https://github.com/MrFrenik/gunslinger/blob/master/docs/containers.md#command-buffer) | [API](https://github.com/MrFrenik/gunslinger/blob/master/docs/containers.md#command-buffer-api)

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

Where the header is defined as:
```c
typedef struct gs_array_header_t {
  uint32_t size;
  uint32_t capacity;
} gs_array_header_t;
```
The user maintains a pointer to the beginning of the data array in memory at all times.

Since `gs_dyn_array` evaluates to a simple pointer of type `T`, the array can be randomly accessed using the `[]` operator as you would with any standard c array. 

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
V* valp = gs_hash_table_getp(ht, K);             
```
* Size/Capacity/Empty/Reserve/Clear:
```c
uint32_t sz = gs_hash_table_size(ht);           // Get size of hash table. Returns 0 if ht is NULL.               
uint32_t cap = gs_hash_table_capacity(ht);      // Get capacity of hash table. Returns 0 if ht is NULL.
bool is_empty = gs_hash_table_empty(ht);        // Returns whether hash table is empty. Returns true if ht NULL.
gs_hash_table_reserve(ht, N);                   // Reserves internal space in the hash table for N (uint32_t), non-initialized elements.
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
`gs_slot_array` is a double indirection array. Internally they are just dynamic arrays but alleviate the issue with losing references to internal data when the arrays grow. Slot arrays therefore hold two internal arrays: 
```c
gs_dyn_array(T)        your_data;
gs_dyn_array(uint32_t) indirection_array;
```
The indirection array takes an opaque `uint32_t` handle and then dereferences it to find the actual index for the data you're interested in. Just like dynamic arrays, they are `NULL` initialized and then allocated/initialized internally upon use:
```c
gs_slot_array(float) arr = NULL;                    // Slot array with internal 'float' data
uint32_t hndl = gs_slot_array_insert(arr, 3.145f);  // Inserts your data into the slot array, returns handle to you
float val = gs_slot_array_get(arr, hndl);           // Returns copy of data to you using handle as lookup
```
It is possible to return a mutable pointer reference to the data using `gs_slot_array_getp()`. However, keep in mind that this comes with the
danger that the reference could be lost IF the internal data array grows or shrinks in between you caching the pointer 
and using it. Take for example:
```c
float* val = gs_slot_array_getp(arr, hndl);     // Cache pointer to internal data. Dangerous game.
gs_slot_array_insert(arr, 5.f);                 // At this point, your pointer could be invalidated due to growing internal array.
```
## Slot Array API:
* Creating/Deleting
```c
gs_slot_array(T) sa = NULL;    // Create slot array with data type `T`
gs_slot_array_free(sa);        // Frees slot array internal data calling `gs_free()` internally.
```
* Inserting/Accessing data: 
```c
// Insert data type `T` into slot array. Returns a `uint32_t` handle to user to use for lookups.
uint32_t hndl = gs_slot_array_insert(sa, T);

// Use to query whether or not a handle is valid. Returns true if valid, false if not.
bool valid = gs_slot_array_handle_valid(sa, hndl);

// Get value of type T using handle as lookup. NOTE: Will crash due to access exemption if hndl not valid. 
T val = gs_slot_array_get(sa, hndl);               

// Get pointer reference to data at hndl. NOTE: Will crash due to access exemption if hndl not valid.
T* valp = gs_slot_array_getp(sa, hndl);             
```
* Size/Capacity/Empty/Reserve/Clear:
```c
uint32_t sz = gs_slot_array_size(sa);           // Get size of slot array. Returns 0 if ht is NULL.               
uint32_t cap = gs_slot_array_capacity(sa);      // Get capacity of slot array. Returns 0 if ht is NULL.
bool is_empty = gs_slot_array_empty(sa);        // Returns whether slot array is empty. Returns true if ht NULL.
gs_slot_array_reserve(sa, N);                   // Reserves internal space in the slot array for N (uint32_t), non-initialized elements.
gs_slot_array_clear(sa);                        // Clears all elements. Sets size to 0.
```
* Iterating data:
`gs_slot_array` provides an `stl-style` iterator api using `gs_slot_array_iter`. You can use this iterator in for/while loops to iterate cleanly over valid data.
```c
// Using for loop
for (
  gs_slot_array_iter it = gs_slot_array_iter_new(sa);   // Creates new iterator
  gs_slot_array_iter_valid(sa, it);                     // Checks whether the iterator is valid each loop iteration
  gs_slot_array_iter_advance(sa, it)                    // Advances the iterator position internally
) 
{
  T val = gs_slot_array_iter_get(sa, it);         // Get value using iterator
  T* valp = gs_slot_array_iter_getp(sa, it);      // Get value pointer using iterator
}

// Using while loop
gs_slot_array_iter it = gs_slot_array_iter_new(sa);
while (gs_slot_array_iter_valid(sa, it))
{
   // Do stuff with iterator like in for loop example
   gs_slot_array_iter_advance(sa, it);
}
```

## Slot Map
`gs_slot_map` functionally works exactly the same as `gs_slot_array`, however it allows the user to use one more layer of indirection by 
hashing any data as a key type `K`.

```c
gs_slot_map(float, uint32_t) sm = NULL;         // Slot map with key type 'float' and value type 'uint32_t'
gs_slot_map_insert(sm, 3.145f, 10);             // Inserts your data into the slot map
uint32_t val = gs_slot_map_get(sm, 3.145f);     // Returns copy of data to you using handle as lookup
```

Like the slot array, it is possible to return a reference via pointer using `gs_slot_map_getp()`. Again, this comes with the same 
danger of losing references if not careful about growing the internal data. 

```c
uint32_t* v = gs_slot_map_getp(sm, 3.145.f);    // Cache pointer to data
gs_slot_map_insert(sm, 2.f, 10);                // Possibly have just invalidated your previous pointer
```
## Slot Map API: 

* Creating/Deleting
```c
gs_slot_map(K, V) sm = NULL;        // Create slot map with key K, value V
gs_slot_map_free(sm);               // Frees map memory. Calls `gs_free` internally.
```
* Inserting/Accessing data: 
```c
gs_slot_map_insert(sm, K, V);                // Insert data into slot map. Init/Grow on demand.
uint64_t v = gs_slot_map_get(sm, K);         // Get data at key.
uint64_t* vp = gs_slot_map_getp(sm, K);      // Get pointer reference at hndl. Dangerous.
```
* Size/Capacity/Empty/Reserve/Clear:
```c
uint32_t sz = gs_slot_map_size(sm);          // Size of slot map. Returns 0 if NULL.
uint32_t cap = gs_slot_map_capacity(sm);     // Capacity of slot map. Returns 0 if NULL.
gs_slot_map_empty(sm);                       // Returns whether slot map is empty. Returns true if NULL.
gs_slot_map_reserve(sm, N);                  // Reserves internal space in the slot map for N (uint32_t), non-initialized elements.
gs_slot_map_clear(sm);                       // Clears map. Sets size to 0.
```
* Iterating data:
`gs_slot_map` provides an `stl-style` iterator api using `gs_slot_map_iter`. You can use this iterator in for/while loops to iterate cleanly over valid data.
```c
// Using for loop
for (
  gs_slot_map_iter it = gs_slot_map_iter_new(sm);   // Creates new iterator
  gs_slot_map_iter_valid(sm, it);                   // Checks whether the iterator is valid each loop iteration
  gs_slot_map_iter_advance(sm, it)                  // Advances the iterator position internally
) 
{
  K key= gs_slot_map_iter_getk(sm, it);         // Get key using iterator
  K* keyp = gs_slot_map_iter_getkp(sm, it);     // Get key pointer using iterator
  V val = gs_slot_map_iter_get(sa, it);         // Get value using iterator
  V* valp = gs_slot_map_iter_getp(sa, it);      // Get value pointer using iterator
}

// Using while loop
gs_slot_map_iter it = gs_slot_array_iter_new(sm);
while (gs_slot_map_iter_valid(sm, it))
{
   // Do stuff with iterator like in for loop example
   gs_slot_map_iter_advance(sm, it);
}
```

## Byte Buffer
`gs_byte_buffer_t` is a convenient data structure for being able to read/write structed byte information, which makes it perfect for binary serialization. Internally, it just consists of a dynamic array of `uint8_t` data. 

## Byte Buffer API:

* New/Free
```c
gs_byte_buffer_t bb = gs_byte_buffer_new();     // Create new byte buffer and initialize.
gs_byte_buffer_free(&bb);                       // Free byte buffer memory by calling `gs_free()` internally.
```
* Read/Write
```c
gs_byte_buffer_write(&bb, T, val);          // Macro for writing data of type `T` into buffer.

T val;
gs_byte_buffer_read(&bb, T, &val);          // Macro for reading data of type `T` into val. Pass in val by pointer.

gs_byte_buffer_readc(&bb, T, NAME);         // Macro for reading data type `T` from buffer and constructing variable `NAME`. 
```
* Read/Write Bulk
```c
gs_byte_buffer_read_bulk(gs_byte_buffer_t* buffer, void** dst, size_t sz);        // Reads 'sz' number of bytes from the buffer into 'dst'. 'dst' must be allocated.
gs_byte_buffer_read_bulkc(BUFFER, T, NAME, SZ);                                   // Macro for bulk reading data type `T` from buffer and constructing variable `NAME`. 
void gs_byte_buffer_write_bulk(gs_byte_buffer_t* buffer, void* src, size_t sz);   // Writes 'src' into buffer 'sz' amount of bytes.
```

* Seek Commands
```c
gs_byte_buffer_seek_to_beg(gs_byte_buffer_t* buffer);                   // Sets read/write position to beginning of buffer.
gs_byte_buffer_seek_to_end(gs_byte_buffer_t* buffer);                   // Sets read/write position to end of buffer.
gs_byte_buffer_advance_position(gs_byte_buffer_t* buffer, size_t sz);   // Advances byte buffer ahead in 'sz' number of bytes.
```

# Example
```c
gs_byte_buffer_t bb = gs_byte_buffer_new();   // Construct new byte buffer.
gs_byte_buffer_write(&bb, uint32_t, 16);      // Write in a uint32_t value of 16.
gs_byte_buffer_seek_to_beg(&bb);              // Set read position back to beginning of buffer to prepare for read.
gs_byte_buffer_readc(&bb, uint32_t, v);       // Read the uint32_t value back into a variable 'v'.
```

## Command Buffer
`gs_command_buffer_t` is used for buffering up data in "command packets" that can be used for various tasks, including rendering. For example, the graphics subsystem uses command buffers explicitly for the purpose of being able to buffer up as many commands as possible throughout the application that can then be passed along to the graphics subsystem to parse and push out to the graphics hardware. 

## Command Buffer API:

* New/Free
```c
gs_command_buffer_t cb = gs_command_buffer_new();     // Create new command buffer and initialize.
gs_command_buffer_free(gs_command_buffer_t* cb);      // Free byte buffer memory by calling `gs_free()` internally.

* Read/Write
gs_command_buffer_write(CB, CT, C, T, VAL);     // Macro for writing command 'C' of command type 'CT' into buffer. Then value 'VAL' of type 'T' is written as the packet data.
gs_command_buffer_readc(CB, C, NAME);           // Macro for reading command type 'C' and constructing a variable of type `NAME`.
```




