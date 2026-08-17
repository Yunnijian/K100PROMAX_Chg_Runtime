/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __LINUX_UNION_FIND_H
#define __LINUX_UNION_FIND_H


struct uf_node {
	struct uf_node *parent;
	unsigned int rank;
};


#define UF_INIT_NODE(node)	{.parent = &node, .rank = 0}


static inline void uf_node_init(struct uf_node *node)
{
	node->parent = node;
	node->rank = 0;
}


struct uf_node *uf_find(struct uf_node *node);


void uf_union(struct uf_node *node1, struct uf_node *node2);

#endif 
