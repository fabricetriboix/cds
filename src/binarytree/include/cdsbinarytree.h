/* Copyright (c) 2016  Fabrice Triboix
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/** Plain binary tree
 *
 * \defgroup cdsbinarytree Plain binary tree
 * \addtogroup cdsbinarytree
 * @{
 *
 * Plain binary tree.
 */

#ifndef CDSBINARYTREE_h_
#define CDSBINARYTREE_h_

#include "cdscommon.h"



/*----------------+
 | Types & Macros |
 +----------------*/


/** Opaque type that represents a binary tree */
typedef struct CdsBinaryTree CdsBinaryTree;


/* Binary tree node
 *
 * You can "derive" from this structure, as long as it remains at the top of
 * your own structure definition. For example:
 *
 *     typedef struct {
 *         CdsBinaryTreeNode node;
 *         int x;
 *         float y;
 *         char* z;
 *     } MyItem;
 */
typedef struct CdsBinaryTreeNode
{
    CdsBinaryTree*            tree;
    struct CdsBinaryTreeNode* parent;
    struct CdsBinaryTreeNode* left;
    struct CdsBinaryTreeNode* right;
    uint8_t                   flags;
} CdsBinaryTreeNode;


/** Prototype of a function to add a reference to an item
 *
 * This function should increment the internal reference counter of the node by
 * one.
 */
typedef void (*CdsBinaryTreeNodeRef)(CdsBinaryTreeNode* node);


/** Prototype of a function to remove a reference to an item
 *
 * This function should decrement the internal reference counter of the node by
 * one. If the internal reference counter reaches zero, the node should be
 * freed.
 */
typedef void (*CdsBinaryTreeNodeUnref)(CdsBinaryTreeNode* node);


/** Prototype of a function to take action on a node
 *
 * You may modify the tree in this function, provided that it is on nodes that
 * have already been traversed, or this node.
 *
 * \param node   [in,out] Node to take action on
 * \param cookie [in]     Cookie for this function
 */
typedef void (*CdsBinaryTreeNodeAction)(CdsBinaryTreeNode* node, void* cookie);



/*------------------------------+
 | Public function declarations |
 +------------------------------*/


/** Create a binary tree
 *
 * \param name  [in] Name for this binary tree; may be NULL
 * \param ref   [in] Function to add a reference to a node; may be NULL if you
 *                   don't need it
 * \param unref [in] Function to remove a reference to a node; may be NULL if
 *                   you don't need it
 *
 * \return The newly-allocated binary tree, never NULL
 */
CdsBinaryTree* CdsBinaryTreeCreate(const char* name,
        CdsBinaryTreeNodeRef ref, CdsBinaryTreeNodeUnref unref);


/** Destroy a binary tree
 *
 * Any item in the binary tree will be unreferenced.
 *
 * \param tree [in,out] Binary tree to destroy; must not be NULL
 */
void CdsBinaryTreeDestroy(CdsBinaryTree* tree);


/** Get the tree's name
 *
 * \param tree [in] Binary tree to query; must not be NULL
 */
const char* CdsBinaryTreeName(const CdsBinaryTree* tree);


/** Get the number of items currently in the binary tree
 *
 * \param tree [in] Binary tree to query; must not be NULL
 */
int64_t CdsBinaryTreeSize(const CdsBinaryTree* tree);


/** Test if the binary tree is empty
 *
 * \param tree [in] Binary tree to query; must not be NULL
 *
 * \return `true` if the binary tree is empty, `false` otherwise
 */
bool CdsBinaryTreeIsEmpty(const CdsBinaryTree* tree);


/** Set the root of a binary tree
 *
 * This function should only be called on an empty tree.
 *
 * \param tree [in,out] Binary tree to manipulate; must not be NULL
 * \param root [in,out] Node to set as the root of the `tree`; must not be NULL
 *
 * \return `true` if OK, `false` if `tree` is not empty
 */
bool CdsBinaryTreeSetRoot(CdsBinaryTree* tree, CdsBinaryTreeNode* root);


/** Set the left of a parent node to the given child node
 *
 * You must not insert the same node more than once for a given tree, or random
 * crashes will occur.
 *
 * \param parent [in,out] Parent node; must not be NULL
 * \param child  [in,out] Child node to insert under `parent`; must not be NULL
 *
 * \return `true` if OK, `false` if the `parent` already has a left child
 */
bool CdsBinaryTreeInsertLeft(CdsBinaryTreeNode* parent,
        CdsBinaryTreeNode* child);


/** Set the right of a parent node to the given child node
 *
 * You must not insert the same node more than once for a given tree, or random
 * crashes will occur.
 *
 * \param parent [in,out] Parent node; must not be NULL
 * \param child  [in,out] Child node to insert under `parent`; must not be NULL
 *
 * \return `true` if OK, `false` if the `parent` already has a right child
 */
bool CdsBinaryTreeInsertRight(CdsBinaryTreeNode* parent,
        CdsBinaryTreeNode* child);


/** Recursively remove a node
 *
 * All nodes in the sub-tree under (and including) `node` will be de-referenced.
 *
 * \param node [in,out] Node to remove; must not be NULL
 */
void CdsBinaryTreeRemoveNode(CdsBinaryTreeNode* node);


/** Get the root of a binary tree
 *
 * \param tree [in] Binary tree to query; must not be NULL
 *
 * \return The root node of the binary tree, or NULL if `tree` is empty
 */
CdsBinaryTreeNode* CdsBinaryTreeRoot(const CdsBinaryTree* tree);


/** Get the left child of a node
 *
 * \param node [in] Node to query; must not be NULL
 *
 * \return Child node on left of `node`, or NULL if `node` has no left child
 */
CdsBinaryTreeNode* CdsBinaryTreeLeftNode(const CdsBinaryTreeNode* node);


/** Get the right child of a node
 *
 * \param node [in] Node to query; must not be NULL
 *
 * \return Child node on right of `node`, or NULL if `node` has no right child
 */
CdsBinaryTreeNode* CdsBinaryTreeRightNode(const CdsBinaryTreeNode* node);


/** Get the parent of a node
 *
 * \param node [in] Node to query; must not be NULL
 *
 * \return Parent node `node`, or NULL if `node` has no parent (i.e. it is the root node)
 */
CdsBinaryTreeNode* CdsBinaryTreeParentNode(const CdsBinaryTreeNode* node);


/** Check if a node is a leaf
 *
 * A node is a leaf if it has no children.
 *
 * \param node [in] Node to test; must not be NULL
 *
 * \return `true` if `node` is a leaf, `false` otherwise
 */
bool CdsBinaryTreeIsLeaf(const CdsBinaryTreeNode* node);


/** Merge two binary trees into one
 *
 * The two binary trees used as input will be destroyed in the process, like if
 * `CdsBinaryTreeDestroy()` would have been called on them.
 *
 * The node `ref` and `unref` functions of the `left` tree will be reused for
 * the merged tree.
 *
 * \param name  [in]     A name for the merged tree; may be NULL
 * \param root  [in,out] Node to use as the root of the new binary tree; must
 *                       not be NULL
 * \param left  [in,out] Binary tree to set left of `root`; must not be NULL;
 *                       the `left` binary tree will be destroyed in the process
 * \param right [in,out] Binary tree to set right of `root`; must not be NULL;
 *                       the `right` binary tree will be destroyed in the
 *                       process
 *
 * \return A new binary tree, which is a merge of the two; this function never
 *         returns NULL
 */
CdsBinaryTree* CdsBinaryTreeMerge(const char* name, CdsBinaryTreeNode* root,
        CdsBinaryTree* left, CdsBinaryTree* right);


/** Traverse a sub-tree in pre-order fashion
 *
 * This function will recursively traverse the sub-tree starting at `top`
 * following the pre-order method of traversal: root, left, right.
 *
 * The `action` function will be called on each traversed node.
 *
 * \param node   [in] Top node of sub-tree to traverse; must not be NULL
 * \param action [in] Action to apply to nodes; must not be NULL
 * \param cookie [in] Cookie for the previous action function
 */
void CdsBinaryTreeTraversePreOrder(CdsBinaryTreeNode* node,
        CdsBinaryTreeNodeAction action, void* cookie);


/** Traverse a sub-tree in in-order fashion
 *
 * This function will recursively traverse the sub-tree starting at `top`
 * following the in-order method of traversal: left, root, right.
 *
 * The `action` function will be called on each traversed node.
 *
 * \param node   [in] Top node of sub-tree to traverse; must not be NULL
 * \param action [in] Action to apply to nodes; must not be NULL
 * \param cookie [in] Cookie for the previous action function
 */
void CdsBinaryTreeTraverseInOrder(CdsBinaryTreeNode* node,
        CdsBinaryTreeNodeAction action, void* cookie);


/** Traverse a sub-tree in post-order fashion
 *
 * This function will recursively traverse the sub-tree starting at `top`
 * following the post-order method of traversal: left, root, right.
 *
 * The `action` function will be called on each traversed node.
 *
 * \param node   [in] Top node of sub-tree to traverse; must not be NULL
 * \param action [in] Action to apply to nodes; must not be NULL
 * \param cookie [in] Cookie for the previous action function
 */
void CdsBinaryTreeTraversePostOrder(CdsBinaryTreeNode* node,
        CdsBinaryTreeNodeAction action, void* cookie);



#endif /* CDSBINARYTREE_h_ */
/* @} */
