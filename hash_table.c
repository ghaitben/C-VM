#include "hash_table.h"
#include "error.h"
#include <string.h>
#include <stddef.h>
#include <stdlib.h>

static char *dynamicStrCpy(char *s) {
		char *ret = malloc(strlen(s) + 1);
		strcpy(ret, s);
		return ret;
}

void initEntryArray(EntryArray *entry_array) {
		entry_array->count = 0;
		entry_array->capacity = 0;
		entry_array->array = NULL;
}

void freeEntryArray(EntryArray *entry_array) {
		for(int i = 0; i < entry_array->count; ++i) {
				freeValue(&entry_array->array[i].value);
		}
		free(entry_array->array);
		initEntryArray(entry_array);
}

// Grow the array when the number of elements reaches the max capacity of the array.
void writeEntryArray(EntryArray *entry_array, Entry entry) {
		if(entry_array->count + 1 > entry_array->capacity) {
				entry_array->capacity = entry_array->capacity > 0 ? 2 * entry_array->capacity : 8;
				entry_array->array = realloc(entry_array->array, sizeof(Entry) * entry_array->capacity);
		}
		entry_array->array[entry_array->count] = entry;
		entry_array->count++;
}

void initHashTable(HashTable *hash_table) {
		hash_table->count = 0;
		hash_table->capacity = 0;
		hash_table->table = NULL;
}

void freeHashTable(HashTable *hash_table) {
		for(int c = 0; c < hash_table->capacity; ++c) {
				EntryArray *entry_array = &hash_table->table[c];
				
				for(int r = 0; r < entry_array->count; ++r) {
						free(entry_array->array[r].key);
				}
				freeEntryArray(entry_array);
		}
		free(hash_table->table);
		initHashTable(hash_table);
}

void resize(HashTable *hash_table, int new_capacity) {
		EntryArray *new_table = malloc(new_capacity * sizeof(EntryArray));
		for(int c = 0; c < new_capacity; ++c) {
				initEntryArray(&new_table[c]);
		}

		for(int c = 0; c < hash_table->capacity; ++c) {
				EntryArray *row = &hash_table->table[c];
				for(int r = 0; r < row->count; ++r) {
						int index = hash(row->array[r].key) % new_capacity;
						writeEntryArray(&new_table[index], row->array[r]);
				}
		}
		
		for(int c = 0; c < hash_table->capacity; ++c) {
				freeEntryArray(&hash_table->table[c]);
		}

		free(hash_table->table);
		hash_table->capacity = new_capacity;
		hash_table->table = new_table;
}

Entry *keyExists(HashTable *hash_table, char *key) {
		int index = hash(key) % hash_table->capacity;
		EntryArray *entry_array = &hash_table->table[index];

		for(int r = 0; r < entry_array->count; ++r) {
				if(strcmp(entry_array->array[r].key, key) == 0) return &entry_array->array[r];
		}
		return NULL;
}

void insertHashTable(HashTable *hash_table, Entry entry) {
		if(hash_table->count + 1 > hash_table->capacity * MAX_LOAD_FACTOR) {
				int new_capacity = hash_table->capacity > 0 ? 2 * hash_table->capacity :
						INITIAL_TABLE_CAPACITY;

				resize(hash_table, new_capacity);
		}

		Entry *entry_found = keyExists(hash_table, entry.key);
		if(entry_found) {
				entry_found->value = entry.value;
		}
		else {
				int index = hash(entry.key) % hash_table->capacity;
				entry.key = dynamicStrCpy(entry.key);
				writeEntryArray(&hash_table->table[index], entry);
		}
		hash_table->count++;
}

bool deleteHashTable(HashTable *hash_table, char *key) {
		if(!keyExists(hash_table, key)) return false;

		int index = hash(key) % hash_table->capacity;
		EntryArray *entry_array = &hash_table->table[index];

		int r = 0;
		while(strcmp(entry_array->array[r].key, key)) r++;
		
		free(entry_array->array[r].key);
		freeValue(&entry_array->array[r].value);
		entry_array->array[r].key = entry_array->array[entry_array->count - 1].key;
		entry_array->array[r].value = entry_array->array[entry_array->count -1].value;

		entry_array->count--;
		hash_table->count--;
		return true;
}

uint32_t hash(char *key) {
		uint32_t h = 2166136261u;
		for(int i = 0; i < strlen(key); ++i) {
				h ^= (uint8_t) key[i];
				h *= 16777619;
		}
		return h;
}
