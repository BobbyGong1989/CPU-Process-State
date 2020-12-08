#ifndef _RBTree
#define _RBTree
#define RED      0    // 红色节点
#define BLACK    1    // 黑色节点
#define rb_parent(r)   ((r)->parent)
#define rb_color(r) ((r)->color)
#define rb_is_red(r)   ((r)->color==RED)
#define rb_is_black(r)  ((r)->color==BLACK)
#define rb_set_black(r)  do { (r)->color = BLACK; } while (0)
#define rb_set_red(r)  do { (r)->color = RED; } while (0)
#define rb_set_parent(r,p)  do { (r)->parent = (p); } while (0)
#define rb_set_color(r,c)  do { (r)->color = (c); } while (0)

	//红黑树的节点
typedef struct RBTreeNode {
	unsigned long process_id;
	long long last_time_;
	long long last_system_time_;
    unsigned char color;        // 颜色(RED 或 BLACK)
    char* key;                  // 关键字(键值)
    struct RBTreeNode *left;    // 左孩子
    struct RBTreeNode *right;   // 右孩子
    struct RBTreeNode *parent;  // 父结点
} RBNode, *RBTree;

//红黑树的根
typedef struct rb_root {
    RBNode *node;
} RBRoot;

void rbtree_left_rotate(RBRoot *root, RBNode *x);

void rbtree_right_rotate(RBRoot *root, RBNode *y);

void rbtree_insert_fixup(RBRoot *root, RBNode *node);

void rbtree_insert(RBRoot *root, RBNode *node);

RBRoot* create_rbtree();

void destroy_rbtree(RBRoot *root);

int nodes(RBTree tree);

void preorder(RBTree tree);

void rbtree_preorder(RBRoot *root);

void inorder(RBTree tree);

void rbtree_inorder(RBRoot *root);

int rbtree_nodes(RBRoot *root);

RBNode* search(RBTree x, char* key);

RBNode* rbtree_search(RBRoot *root, char* key);

#endif
