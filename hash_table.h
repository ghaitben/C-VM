#ifndef COMPILER_HASH_TABLE_H
#define COMPILER_HASH_TABLE_H

#include "value.h"
#include <stdint.h>

#define MAX_LOAD_FACTOR 0.8
#define INITIAL_TABLE_CAPACITY 10

typedef struct HashTable HashTable;
typedef struct Entry Entry;
typedef struct EntryArray EntryArray;

struct Entry {
	  char *key;
		Value value;
};

uint32_t hash(char *key);

struct EntryArray {
		int count;
		int capacity;
		Entry *array;
};
void initEntryArray(EntryArray *entry_array);
void freeEntryArray(EntryArray *entry_array);
void writeEntryArray(EntryArray *entry_array, Entry entry);


struct HashTable {
		int count;
		int capacity;
		EntryArray *table;
};
void initHashTable(HashTable *hash_table);
void freeHashTable(HashTable *hash_table);
void resize(HashTable *hash_table, int new_capacity);


void insertHashTable(HashTable *hash_table, Entry entry);
bool deleteHashTable(HashTable *hash_table, char *key);
Entry *keyExists(HashTable *hash_table, char *key);

#endif
