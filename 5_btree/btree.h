#include <stdbool.h>

#define DEGREE 	2
#define MAX_KEYS_COUNT (2 * DEGREE - 1)

typedef struct btree_node
{
	uint64_t keys[MAX_KEYS_COUNT];
	uint64_t values[MAX_KEYS_COUNT];
	
	struct btree_node *children[MAX_KEYS_COUNT + 1];
	uint64_t keys_count;
	
	bool leaf_flag;
	bool pr_flag[MAX_KEYS_COUNT];
} bnode_t;

typedef struct btree
{
	bnode_t *root;
} btree_t;


int btree_create(btree_t *btree)
{
	btree->root = (bnode_t *)malloc(sizeof(bnode_t));
	if (!btree->root)
	{
		return -1;
	}
	
	btree->root->leaf_flag = true;
	btree->root->keys_count = 0;
	return 0;
}

static void bnode_destroy(bnode_t *bnode)
{
	if (!bnode->leaf_flag)
	{
		for (uint64_t i = 0; i < bnode->keys_count + 1; ++i)
		{
			bnode_destroy(bnode->children[i]);
		}
	}
	
	free(bnode);
}

void btree_destroy(btree_t *btree)
{
	bnode_t *bnode = btree->root;
	if (!bnode->leaf_flag)
	{
		for (uint64_t i = 0; i < bnode->keys_count + 1; ++i)
		{
			bnode_destroy(bnode->children[i]);
		}
	}
	
	free(bnode);
}

void btree_insert(btree_t *btree, uint64_t key, uint64_t val);
void btree_delete_key(btree_t *btree, uint64_t key);
void btree_print(btree_t *btree);
