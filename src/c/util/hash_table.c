#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hash_table.h"

#define INITIAL_HASH_TABLE_SIZE	    113

struct ht_entry {
    char *key;
    const void *data;
    struct ht_entry *next;
};

struct hash_table {
    int (*hash) (const char *);
    size_t size;
    struct ht_entry **buf;
    int entry_count;
};

static const unsigned char T[256] = {
    // 256 values 0-255 in any (random) order suffices
    98, 6, 85, 150, 36, 23, 112, 164, 135, 207, 169, 5, 26, 64, 165, 219,
    61, 20, 68, 89, 130, 63, 52, 102, 24, 229, 132, 245, 80, 216, 195, 115,
    90, 168, 156, 203, 177, 120, 2, 190, 188, 7, 100, 185, 174, 243, 162, 10,
    237, 18, 253, 225, 8, 208, 172, 244, 255, 126, 101, 79, 145, 235, 228, 121,
    123, 251, 67, 250, 161, 0, 107, 97, 241, 111, 181, 82, 249, 33, 69, 55,
    59, 153, 29, 9, 213, 167, 84, 93, 30, 46, 94, 75, 151, 114, 73, 222,
    197, 96, 210, 45, 16, 227, 248, 202, 51, 152, 252, 125, 81, 206, 215, 186,
    39, 158, 178, 187, 131, 136, 1, 49, 50, 17, 141, 91, 47, 129, 60, 99,
    154, 35, 86, 171, 105, 34, 38, 200, 147, 58, 77, 118, 173, 246, 76, 254,
    133, 232, 196, 144, 198, 124, 53, 4, 108, 74, 223, 234, 134, 230, 157, 139,
    189, 205, 199, 128, 176, 19, 211, 236, 127, 192, 231, 70, 233, 88, 146, 44,
    183, 201, 22, 83, 13, 214, 116, 109, 159, 32, 95, 226, 140, 220, 57, 12,
    221, 31, 209, 182, 143, 92, 149, 184, 148, 62, 113, 65, 37, 27, 106, 166,
    3, 14, 204, 72, 21, 41, 56, 66, 28, 193, 40, 217, 25, 54, 179, 117,
    238, 87, 240, 155, 180, 170, 242, 212, 191, 163, 78, 218, 137, 194, 175, 110,
    43, 119, 224, 71, 122, 142, 42, 160, 104, 48, 247, 103, 15, 11, 138, 239
};

static int default_hash(const char *x)
{
    // Pearson's hash:
    // http://cs.mwsu.edu/~griffin/courses/2133/downloads/Spring11/p677-pearson.pdf
    unsigned char hh[4];
    for (int j = 0; j < 4; j++) {
	unsigned char h = T[(x[0] + j) % 256];
	for (int i = 0; x[i]; i++)
	    h = T[h ^ x[i]];
	hh[j] = h;
    }

    return *(int *) hh;
}

static struct ht_entry *new_entry(const char *key, const void *data)
{
    struct ht_entry *he = (struct ht_entry *) malloc(sizeof(*he));
    he->key = strdup(key);
    he->data = data;
    he->next = NULL;
    return he;
}

static void free_entry(struct ht_entry *he)
{
    if (he) {
	free(he->key);
	free(he);
    }
}

struct hash_table *ht_create(size_t size, int (*hash) (const char *))
{
    struct hash_table *ht = (struct hash_table *) malloc(sizeof(*ht));
    if (hash)
	ht->hash = hash;
    else
	ht->hash = &default_hash;
    if (0 == size)
	ht->size = INITIAL_HASH_TABLE_SIZE;
    else
	ht->size = size;
    ht->buf = calloc(sizeof(*ht->buf), ht->size);
    return ht;
}

int ht_add_entry(struct hash_table *ht, const char *key, const void *data)
{
    int hash = ht->hash(key);
    int pos = hash % ht->size;

    struct ht_entry *he = new_entry(key, data);
    he->next = ht->buf[pos];
    ht->buf[pos] = he;

    return 0;
}

int ht_remove_entry(struct hash_table *ht, const char *key)
{
    int hash = ht->hash(key);
    int pos = hash % ht->size;

    struct ht_entry *he = ht->buf[pos], *prev = NULL;

    while (he) {
	if (!strcmp(he->key, key)) {
	    if (prev)
		prev->next = he->next;
	    else
		ht->buf[pos] = he->next;
	    free_entry(he);
	    return 0;
	}
	prev = he;
	he = he->next;
    }
    return -1;
}

int ht_has_entry(struct hash_table *ht, const char *key)
{
    int hash = ht->hash(key);
    int pos = hash % ht->size;

    struct ht_entry *he = ht->buf[pos];

    while (he) {
	if (!strcmp(he->key, key))
	    return 1;
	he = he->next;
    }
    return 0;
}

int ht_get_entry(struct hash_table *ht, const char *key, void *ret)
{
    int hash = ht->hash(key);
    int pos = hash % ht->size;

    struct ht_entry *he = ht->buf[pos];

    while (he) {
	if (!strcmp(he->key, key)) {
	    *(const void **) ret = he->data;
	    return 0;
	}
	he = he->next;
    }
    return -1;
}

int ht_hash(struct hash_table *ht, const char *key)
{
    return ht->hash(key);
}

void ht_free(struct hash_table *ht)
{
    if (ht) {
	for (unsigned int i = 0; i < ht->size; ++i) {
	    struct ht_entry *he = ht->buf[i];
	    while (he) {
		struct ht_entry *tmp = he;
		he = he->next;
		free_entry(tmp);
	    }
	}
	free(ht->buf);
	free(ht);
    }
}

struct list* ht_to_list(const struct hash_table *ht)
{
    struct list *l = list_new(0);
    if (ht) {
	for (unsigned int i = 0; i < ht->size; ++i) {
	    struct ht_entry *he = ht->buf[i];
	    while (he) {
		list_append(l, he->data);
		he = he->next;
	    }
	}
    }
    return l;
}
