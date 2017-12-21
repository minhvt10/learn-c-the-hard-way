#ifndef LCTHW_BSTREE_H
#define LCTHW_BSTREE_H

typedef int (*BSTree_compare) (void *a, void *b);

typedef struct BSTreeNode
{
	void *key;
	void *value;

	struct BSTreeNode *left;
	struct BSTreeNode *right;
	struct BSTreeNode *parent;
} BSTreeNode;

typedef struct BSTree
{
	int count;
	BSTree_compare compare_cb;
	BSTreeNode *root;
}

typedef int (*BSTree_traverse_cb) (BSTreeNode *node);

BSTree *BSTree_create(BSTree_compare compare_cb);
void BSTree_destroy(BSTree *map);

int BSTree_set(BSTree *map, void *key, void *value);
void *BSTree_get(BSTree *map, void *key);

int BSTree_traverse(BSTree *map, BSTree_traverse_cb traverse_cb);

void *BSTree_delete(BSTree *map, void *key);

#endif // LCTHW_BSTREE_H