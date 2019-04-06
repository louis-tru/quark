/*
 * Copyright (C) 2017, Leo Ma <begeekmyfriend@gmail.com>
 */

#ifndef _BPLUS_TREE_H
#define _BPLUS_TREE_H

typedef int key_t;

typedef struct bplus_tree_s bplus_tree_t;

void bplus_tree_dump(bplus_tree_t *tree);
long bplus_tree_get(bplus_tree_t *tree, key_t key);
int  bplus_tree_put(bplus_tree_t *tree, key_t key, long data);
long bplus_tree_get_range(bplus_tree_t *tree, key_t key1, key_t key2);
bplus_tree_t *bplus_tree_init(char *filename, int block_size);
void bplus_tree_deinit(bplus_tree_t *tree);

#endif  /* _BPLUS_TREE_H */