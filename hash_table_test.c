#include "hash_table.h"
#include "error.h"
#include <stdlib.h>
#include <stdio.h>


Entry createEntry(char *key, Value value) {
		Entry e;
		e.key = key;
		e.value = value;
		return e;
}

int main(int argc, char **argv) {
		HashTable hash_table;
		initHashTable(&hash_table);
		const int n = 100000;

		char *arr[n];

		for(int i = 0; i < n; ++i) {
				char alphabet[] = "abcdefghijklmnopqrstuvwxyz";
				int size = (rand() % 70) + 1;
				CHECK(size > 0, "size < 0");

				char *random_string = malloc(size + 1);
				for(int i = 0; i < size; ++i) {
						random_string[i] = alphabet[ rand() % 26 ];
				}
				random_string[size] = '\0';
				arr[i] = random_string;
				insertHashTable(&hash_table, createEntry(random_string, CREATE_NUMBER(size)));
		}

		for(int i = 0; i < n; ++i) {
				Entry *e = keyExists(&hash_table, arr[i]);
				CHECK(e != NULL, arr[i]);
		}

		for(int i = n - 10; i >= 0; --i) {
				deleteHashTable(&hash_table, arr[i]);
				CHECK(keyExists(&hash_table, arr[i]) == NULL, "Failed Test");
		}

		freeHashTable(&hash_table);
		for(int i = 0; i < n; ++i) free(arr[i]);

		printf("Tests Succeeded!\n");
}
