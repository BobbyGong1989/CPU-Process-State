#include<stdio.h>
#include<malloc.h>
#include<string.h>
#include"RBTree.h"
#include<windef.h>

/*
 * 对红黑树的节点(x)进行左旋转
 *
 * 左旋示意图(对节点x进行左旋)：
 *      px                              px
 *     /                               /
 *    x                               y
 *   /  \      --(左旋)-->           / \                #
 *  lx   y                          x  ry
 *     /   \                       /  \
 *    ly   ry                     lx  ly
 *
 *
 */
void rbtree_left_rotate(RBRoot *root, RBNode *x) {
	// 设置x的右孩子为y
	RBNode *y = x->right;

	// 将 “y的左孩子” 设为 “x的右孩子”；
	// 如果y的左孩子非空，将 “x” 设为 “y的左孩子的父亲”
	x->right = y->left;
	if (y->left != NULL)
		y->left->parent = x;

	// 将 “x的父亲” 设为 “y的父亲”
	y->parent = x->parent;

	if (x->parent == NULL) {
		//tree = y;            // 如果 “x的父亲” 是空节点，则将y设为根节点
		root->node = y;            // 如果 “x的父亲” 是空节点，则将y设为根节点
	} else {
		if (x->parent->left == x)
			x->parent->left = y;    // 如果 x是它父节点的左孩子，则将y设为“x的父节点的左孩子”
		else
			x->parent->right = y;    // 如果 x是它父节点的左孩子，则将y设为“x的父节点的左孩子”
	}

	// 将 “x” 设为 “y的左孩子”
	y->left = x;
	// 将 “x的父节点” 设为 “y”
	x->parent = y;
}

/*
 * 对红黑树的节点(y)进行右旋转
 *
 * 右旋示意图(对节点y进行左旋)：
 *            py                               py
 *           /                                /
 *          y                                x
 *         /  \      --(右旋)-->            /  \                     #
 *        x   ry                           lx   y
 *       / \                                   / \                   #
 *      lx  rx                                rx  ry
 *
 */
void rbtree_right_rotate(RBRoot *root, RBNode *y) {
	// 设置x是当前节点的左孩子。
	RBNode *x = y->left;

	// 将 “x的右孩子” 设为 “y的左孩子”；
	// 如果"x的右孩子"不为空的话，将 “y” 设为 “x的右孩子的父亲”
	y->left = x->right;
	if (x->right != NULL)
		x->right->parent = y;

	// 将 “y的父亲” 设为 “x的父亲”
	x->parent = y->parent;

	if (y->parent == NULL) {
		//tree = x;            // 如果 “y的父亲” 是空节点，则将x设为根节点
		root->node = x;            // 如果 “y的父亲” 是空节点，则将x设为根节点
	} else {
		if (y == y->parent->right)
			y->parent->right = x;    // 如果 y是它父节点的右孩子，则将x设为“y的父节点的右孩子”
		else
			y->parent->left = x;    // (y是它父节点的左孩子) 将x设为“x的父节点的左孩子”
	}

	// 将 “y” 设为 “x的右孩子”
	x->right = y;

	// 将 “y的父节点” 设为 “x”
	y->parent = x;
}

/*
 * 红黑树插入修正函数
 *
 * 在向红黑树中插入节点之后(失去平衡)，再调用该函数；
 * 目的是将它重新塑造成一颗红黑树。
 *
 * 参数说明：
 *     root 红黑树的根
 *     node 插入的结点        // 对应《算法导论》中的z
 */
void rbtree_insert_fixup(RBRoot *root, RBNode *node) {
	RBNode *parent, *gparent;

	// 若“父节点存在，并且父节点的颜色是红色”
	while ((parent = rb_parent(node)) && rb_is_red(parent)) {
		gparent = rb_parent(parent);

		//若“父节点”是“祖父节点的左孩子”
		if (parent == gparent->left) {
			// Case 1条件：叔叔节点是红色
			{
				RBNode *uncle = gparent->right;
				if (uncle && rb_is_red(uncle)) {
					rb_set_black(uncle);
					rb_set_black(parent);
					rb_set_red(gparent);
					node = gparent;
					continue;
				}
			}

			// Case 2条件：叔叔是黑色，且当前节点是右孩子
			if (parent->right == node) {
				RBNode *tmp;
				rbtree_left_rotate(root, parent);
				tmp = parent;
				parent = node;
				node = tmp;
			}

			// Case 3条件：叔叔是黑色，且当前节点是左孩子。
			rb_set_black(parent);
			rb_set_red(gparent);
			rbtree_right_rotate(root, gparent);
		} else { //若“z的父节点”是“z的祖父节点的右孩子”
			// Case 1条件：叔叔节点是红色
			{
				RBNode *uncle = gparent->left;
				if (uncle && rb_is_red(uncle)) {
					rb_set_black(uncle);
					rb_set_black(parent);
					rb_set_red(gparent);
					node = gparent;
					continue;
				}
			}

			// Case 2条件：叔叔是黑色，且当前节点是左孩子
			if (parent->left == node) {
				RBNode *tmp;
				rbtree_right_rotate(root, parent);
				tmp = parent;
				parent = node;
				node = tmp;
			}

			// Case 3条件：叔叔是黑色，且当前节点是右孩子。
			rb_set_black(parent);
			rb_set_red(gparent);
			rbtree_left_rotate(root, gparent);
		}
	}

	// 将根节点设为黑色
	rb_set_black(root->node);
}

/*
 * 添加节点：将节点(node)插入到红黑树中
 *
 * 参数说明：
 *     root 红黑树的根
 *     node 插入的结点        // 对应《算法导论》中的z
 */
void rbtree_insert(RBRoot *root, RBNode *node) {
	RBNode *y = NULL;
	RBNode *x = root->node;

	// 1. 将红黑树当作一颗二叉查找树，将节点添加到二叉查找树中。
	while (x != NULL) {
		y = x;
		if (strcmp(node->key, x->key) < 0)
			x = x->left;
		else
			x = x->right;
	}
	rb_parent(node) = y;

	if (y != NULL) {
		if (strcmp(node->key, y->key) < 0)
			y->left = node;                // 情况2：若“node所包含的值” < “y所包含的值”，则将node设为“y的左孩子”
		else
			y->right = node;            // 情况3：(“node所包含的值” >= “y所包含的值”)将node设为“y的右孩子”
	} else {
		root->node = node;                // 情况1：若y是空节点，则将node设为根
	}

	// 2. 设置节点的颜色为红色
	node->color = RED;

	// 3. 将它重新修正为一颗二叉查找树
	rbtree_insert_fixup(root, node);
}

/*
 * 创建红黑树，返回"红黑树的根"！
 */
RBRoot* create_rbtree() {
	RBRoot *root = (RBRoot*) malloc(sizeof(RBRoot));
	root->node = NULL;
	return root;
}

/*
 712  * 销毁红黑树
 713  */
void rbtree_destroy(RBTree tree) {
	if (tree == NULL)
		return;
	if (tree->left != NULL)
		rbtree_destroy(tree->left);
	if (tree->right != NULL)
		rbtree_destroy(tree->right);
	char *key = tree->key;
	free(key);
	key = NULL;
	free(tree);
	tree = NULL;
}

void destroy_rbtree(RBRoot *root) {
	if (root != NULL)
		rbtree_destroy(root->node);

	free(root);
}

/*
 * 遍历树的节点数
 */
int nodes(RBTree tree) {
	if (tree == NULL)
		return 0;
	else
		return nodes(tree->left) + nodes(tree->right) + 1;
}

/*
 * 前序遍历"红黑树"
 */
void preorder(RBTree tree) {
	if (tree != NULL) {
		printf("%s ", tree->key);
		preorder(tree->left);
		preorder(tree->right);
	}
}

void rbtree_preorder(RBRoot *root) {
	if (root)
		preorder(root->node);
}

void inorder(RBTree tree) {
	if (tree != NULL) {
		inorder(tree->left);
		printf("%s \n", tree->key);
		//void *root = tree->data;
		//printf("%s ", root->type);
		//if (root != NULL) {
		//	inorder(root->node);
		//}
		inorder(tree->right);
	}
}

void rbtree_inorder(RBRoot *root) {
	if (root)
		inorder(root->node);
}

int rbtree_nodes(RBRoot *root) {
	if (root)
		return nodes(root->node);
	else
		return 0;
}

/*
 * (递归实现)查找"红黑树x"中键值为key的节点
 */
RBNode* search(RBTree x, char *key) {
	if (x == NULL || strcmp(x->key, key) == 0)
		return x;

	if (strcmp(key, x->key) < 0)
		return search(x->left, key);
	else
		return search(x->right, key);
}

RBNode* rbtree_search(RBRoot *root, char *key) {
	if (root)
		return search(root->node, key);
	else
		return NULL;
}

