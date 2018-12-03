#include <stdio.h>
#include <stdlib.h>
#include "btree.h"
#include <string.h>
#include <time.h>

#define INSERTION_COUNT 1000000
#define DELETION_COUNT 10

int main(void)
{
	printf("Insertion time measurement test:\n");
	btree_t btree;
	if (btree_create(&btree))
	{
		printf("Error occured\n");
		exit(-1);
	}

	clock_t clk = clock();
	for (int i = 0; i < INSERTION_COUNT; ++i)
	{
		uint64_t key = random() % INSERTION_COUNT;
		btree_insert(&btree, key, i);
	}
	printf("Insertion Count:\t%lu\nTime:\t\t\t%lums\n", INSERTION_COUNT, (clock() - clk) * 1000 / CLOCKS_PER_SEC);
	btree_destroy(&btree);
	
	fprintf(stdout, "\nDeletion test:");
	if (btree_create(&btree))
	{
		printf("Error occured\n");
		exit(-1);
	}
	for (int i = 0; i < DELETION_COUNT; ++i)
	{
		uint64_t key = random() % DELETION_COUNT;
		btree_insert(&btree, key, i);
	}
	btree_print(&btree);
	printf("\nDelete: ");
	for (int i = 0; i < DELETION_COUNT; ++i)
	{
		uint64_t key = random() % DELETION_COUNT;
		printf("%lu ", key);
		btree_delete_key(&btree, key);
	}
	btree_print(&btree);
	printf("\nInsert: ");
	for (int i = 0; i < DELETION_COUNT; ++i)
	{
		uint64_t key = random() % DELETION_COUNT;
		fprintf(stdout, "%lu ", key);
		btree_insert(&btree, key, i);
	}
	btree_print(&btree);
	btree_destroy(&btree);
}

static void bnode_inner_split(bnode_t *bnode, bnode_t *child, uint64_t i)
{
	for (uint64_t j = bnode->keys_count; j > i; --j)
	{
		bnode->keys[j] = bnode->keys[j - 1];
		bnode->values[j] = bnode->values[j - 1];
		bnode->pr_flag[j] = bnode->pr_flag[j - 1];
	}
	
	bnode->keys[i] = child->keys[DEGREE - 1];
	bnode->values[i] = child->values[DEGREE - 1];
	bnode->pr_flag[i] = child->pr_flag[DEGREE - 1];
}

static void bnode_split_child(bnode_t *bnode, uint64_t i)
{
	bnode_t *child = bnode->children[i];
	
	bnode_t *new_child = (bnode_t *)malloc(sizeof(bnode_t));
	new_child->leaf_flag = child->leaf_flag;
	new_child->keys_count = DEGREE - 1;
	
	memcpy(new_child->keys, child->keys + DEGREE, (DEGREE - 1) * sizeof(*child->keys));
	memcpy(new_child->values, child->values + DEGREE, (DEGREE - 1) * sizeof(*child->values));
	memcpy(new_child->pr_flag, child->pr_flag + DEGREE, (DEGREE - 1) * sizeof(*child->pr_flag));
	if (!child->leaf_flag)
	{
		memcpy(new_child->children, child->children + DEGREE, DEGREE * sizeof(bnode_t *));
	}
	
	child->keys_count = DEGREE - 1;
	for (uint64_t j = bnode->keys_count + 1; j > i + 1; --j)
	{
		bnode->children[j] = bnode->children[j - 1];
	}
	bnode->children[i + 1] = new_child;
	bnode_inner_split(bnode, child, i);
	
	++bnode->keys_count;
}

static void bnode_insert_key(bnode_t *bnode, uint64_t key, uint64_t val)
{
	uint64_t i = bnode->keys_count - 1;
	if (bnode->leaf_flag)
	{
		while (key < bnode->keys[i])
		{
			bnode->keys[i + 1] = bnode->keys[i];
			bnode->values[i + 1] = bnode->values[i];
			bnode->pr_flag[i + 1] = bnode->pr_flag[i];
			if (i-- == 0) break;
		}
		
		bnode->keys[i + 1] = key;
		bnode->values[i + 1] = val;
		bnode->pr_flag[i + 1] = true;
		++bnode->keys_count;
	}
	else
	{
		while (key < bnode->keys[i])
		{
			if (i-- == 0) break;
		}
		++i;
		
		if (bnode->children[i]->keys_count == MAX_KEYS_COUNT)
		{
			bnode_split_child(bnode, i);
			if (key > bnode->keys[i])
			{
				++i;
			}
		}
		
		bnode_insert_key(bnode->children[i], key, val);
	}
}

static void bnode_print(bnode_t *bnode)
{
	printf("(");
	for (uint64_t i = 0; i < bnode->keys_count; ++i)
	{
		if (!bnode->leaf_flag)
		{
			bnode_print(bnode->children[i]);
		}
		printf(" %lu%c%lu ", bnode->keys[i], bnode->pr_flag[i] ? ':' : '*', bnode->values[i]);
	}
	if (!bnode->leaf_flag)
	{
		bnode_print(bnode->children[bnode->keys_count]);
	}
	printf(")");
}

void btree_print(btree_t *btree)
{
	printf("\nBtree\t");
	bnode_print(btree->root);
}

static inline bnode_t *bnode_search(bnode_t *bnode, uint64_t key, uint64_t *index)
{
	uint64_t i = 0;
	while ((i < bnode->keys_count) && (key > bnode->keys[i]))
	{
		++i;
	}
	
	if ((i <= bnode->keys_count) && (key == bnode->keys[i]))
	{
		if (index)
		{
			*index = i;
		}
		return bnode;
	}
	else
	{
		if (!bnode->leaf_flag)
		{
			return bnode_search(bnode->children[i], key, index);
		}
		else
		{
			return NULL;
		}
	}
}

void btree_delete_key(btree_t *btree, uint64_t key)
{
	uint64_t index;
	bnode_t *bnode = bnode_search(btree->root, key, &index);
	if (bnode)
	{
		bnode->pr_flag[index] = false;
	}
}

void btree_insert(btree_t *btree, uint64_t key, uint64_t val)
{
	bnode_t *root = btree->root;
	uint64_t index;
	bnode_t *bnode = bnode_search(root, key, &index);
	
	if (bnode)
	{
		bnode->values[index] = val;
		bnode->pr_flag[index] = true;
		return;
	}
	
	if (root->keys_count == MAX_KEYS_COUNT)
	{
		bnode_t *s = (bnode_t *)malloc(sizeof(bnode_t));
		s->leaf_flag = false;
		s->keys_count = 0;
		s->children[0] = root;
		
		btree->root = s;
		bnode_split_child(s, 0);
		bnode_insert_key(s, key, val);
	}
	else
	{
		bnode_insert_key(root, key, val);
	}
}
