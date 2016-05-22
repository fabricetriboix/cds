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

#include "cdsbinarytree.h"
#include <stdlib.h>
#include <string.h>



/*----------------+
 | Macros & Types |
 +----------------*/


#define CDS_BT_FLAG_VISITED 0x01
#define CDS_BT_FLAG_LEFT    0x02
#define CDS_BT_FLAG_RIGHT   0x04


struct CdsBinaryTree
{
    char* name;
    int64_t size;
    int8_t flags;
    CdsListBinaryTreeNode* root;
    CdsListBinaryTreeNodeRef ref;
    CdsListBinaryTreeNodeUnref unref;
};



/*-------------------------------+
 | Private function declarations |
 +-------------------------------*/


/** Remove a leaf node from a binary tree
 *
 * \param node [in,out] Leaf node to remove; must not be NULL; must be a leaf
 */
static void cdsBinaryTreeRemoveLeafNode(CdsBinaryTreeNode* node)


/** Traversing callback to remove nodes
 *
 * This function will remove `node` if it is a leaf. If it is not a leaf, no
 * action will be taken.
 *
 * \param node   [in,out] Node to remove; must not be NULL; must be a leaf
 * \param cookie [in]     Unused
 */
static void cdsBinaryTreeRemoveNode(CdsBinaryTreeNode* node, void* cookie);


/*---------------------------------+
 | Public function implementations |
 +---------------------------------*/


CdsBinaryTree* CdsBinaryTreeCreate(const char* name,
        CdsBinaryTreeNodeRef ref, CdsBinaryTreeNodeUnref unref)
{
    CdsBinaryTree* tree = CdsMallocZ(sizeof(tree));

    if (name != NULL) {
        tree->name = strdup(name);
    }
    if (capacity > 0) {
        tree->capacity = capacity;
    }
    tree->ref = ref;
    tree->unref = unref;

    return tree;
}


void CdsBinaryTreeDestroy(CdsBinaryTree* tree)
{
    CDSASSERT(tree != NULL);

    CdsBinaryTreeRemove(tree->root);
    free(tree->name);
    free(tree);
}


const char* CdsBinaryTreeName(const CdsBinaryTree* tree)
{
    CDSASSERT(tree != NULL);
    return tree->name;
}


int64_t CdsBinaryTreeSize(const CdsBinaryTree* tree)
{
    CDSASSERT(tree != NULL);
    return tree->size;
}


bool CdsBinaryTreeIsEmpty(const CdsBinaryTree* tree)
{
    CDSASSERT(tree != NULL);
    return (tree->root != NULL);
}


bool CdsBinaryTreeSetRoot(CdsBinaryTree* tree, CdsBinaryTreeNode* root)
{
    CDSASSERT(tree != NULL);
    CDSASSERT(root != NULL);

    bool ret = false;
    if (tree->root == NULL) {
        root->tree = tree;
        root->parent = NULL;
        root->left = NULL;
        root->right = NULL;
        tree->root = root;
        tree->size++;
        ret = true;
    }
    return ret;
}


bool CdsBinaryTreeInsertLeft(CdsBinaryTreeNode* parent,
        CdsBinaryTreeNode* child)
{
    CDSASSERT(parent != NULL);
    CDSASSERT(parent->tree != NULL);
    CDSASSERT(child != NULL);

    bool ret = false;
    if (parent->left == NULL) {
        child->tree = parent->tree;
        child->parent = parent;
        child->left = NULL;
        child->right = NULL;
        parent->left = child;

        CdsBinaryTree* tree = child->tree;
        if (tree->ref != NULL) {
            tree->ref(child);
        }
        tree->size++;

        ret = true;
    }
    return ret;
}


bool CdsBinaryTreeInsertRight(CdsBinaryTreeNode* parent,
        CdsBinaryTreeNode* child)
{
    CDSASSERT(parent != NULL);
    CDSASSERT(parent->tree != NULL);
    CDSASSERT(child != NULL);

    bool ret = false;
    if (parent->right == NULL) {
        child->tree = parent->tree;
        child->parent = parent;
        child->left = NULL;
        child->right = NULL;
        parent->right = child;

        CdsBinaryTree* tree = child->tree;
        if (tree->ref != NULL) {
            tree->ref(child);
        }
        tree->size++;

        ret = true;
    }
    return ret;
}


void CdsBinaryTreeRemoveLeft(CdsBinaryTreeNode* node)
{
    CDSASSERT(node != NULL);
    CDSASSERT(node->tree != NULL);
    CDSASSERT(node->parent != NULL);

    CdsBinaryTree* tree = node->tree;

    if (node->left != NULL) {
        CdsBinaryTreeRemoveLeft(node->left);
    }
    if (node->right != NULL) {
    bool isleft = true;

    // Traverse the sub-tree in preorder method
    // NB: Recursion not necessary!
    for (CdsBinaryTreeNode* curr = node->left; curr != NULL; ) {
        if (curr->left != NULL) {
            // Go down the tree one level on the left
            curr = curr->left;
            isleft = true;

        } else if (curr->right != NULL) {
            // Go down the tree one level on the right
            curr = curr->right;
            isleft = false;

        } else {
            // This is a leaf node, remove it and go up the tree one level
            if (tree->unref != NULL) {
                tree->unref(curr);
            }
            if (isleft) {
                curr->parent->left = NULL;
            } else {
                curr->parent->right = NULL;
            }
            if (curr != node) {
                curr = curr->parent;
            } else {
                curr = node; // We remove all the nodes in the sub-tree
            }
        }
    }
}


void CdsBinaryTreeRemoveNode(CdsBinaryTreeNode* node)
{
    CDSASSERT(node != NULL);

    // TODO: too slow, needs inlining
    CdsBinaryTreeTraversePreOrder(node,
            cdsBinaryTreeActionRemoveNodeIfLeaf, NULL);
}


CdsBinaryTreeNode* CdsBinaryTreeRoot(const CdsBinaryTree* tree)
{
    CDSASSERT(tree != NULL);
    return tree->root;
}


CdsBinaryTreeNode* CdsBinaryTreeLeft(const CdsBinaryTreeNode* node)
{
    CDSASSERT(node != NULL);
    return node->left;
}


CdsBinaryTreeNode* CdsBinaryTreeRight(const CdsBinaryTreeNode* node)
{
    CDSASSERT(node != NULL);
    return node->right;
}


CdsBinaryTreeNode* CdsBinaryTreeIsLeaf(const CdsBinaryTreeNode* node)
{
    CDSASSERT(node != NULL);
    return (node->left == NULL) && (node->right == NULL);
}


CdsBinaryTree* CdsBinaryTreeMerge(const char* name, CdsBinaryTreeNode* root,
        CdsBinaryTree* left, CdsBinaryTree* right)
{
    CDSASSERT(node != NULL);
    CDSASSERT(left != NULL);
    CDSASSERT(right != NULL);

    CdsBinaryTree* tree = CdsBinaryTreeCreate(name, left->ref, left->unref);
    CdsBinaryTreeSetRoot(tree, root);

    tree->root->left = left->root;
    tree->root->right = right->root;
    tree->size += left->size + right->size;

    return tree;
}


void CdsBinaryTreeTraversePreOrder(CdsBinaryTreeNode* node,
        CdsBinaryTreeNodeAction action, void* cookie)
{
    CDSASSERT(node != NULL);
    CDSASSERT(action != NULL);

    // NB: No recursion necessary!
    node->flags = 0;
    for (CdsBinaryTreeNode* curr = node; curr != node->parent; ) {
        if (!(curr->flags & CDS_BT_FLAG_VISITED)) {
            action(curr, cookie);
            curr->flags |= CDS_BT_FLAG_VISITED;
        }

        if (!(curr->flags & CDS_BT_FLAG_LEFT) && (curr->left != NULL)) {
            curr->flags |= CDS_BT_FLAG_LEFT;
            curr = curr->left;
            curr->flags = 0;

        } else if (!(curr->flags & CDS_BT_FLAG_RIGHT) && (curr->right != NULL)){
            curr->flags |= CDS_BT_FLAG_RIGHT;
            curr = curr->right;
            curr->flags = 0;

        } else {
            curr = curr->parent;
        }
    }
}
// TODO: from here



/*----------------------------------+
 | Private function implementations |
 +----------------------------------*/


static void cdsBinaryTreeRemoveLeafNode(CdsBinaryTreeNode* node)
{
    CDSASSERT(node != NULL);
    CDSASSERT(node->tree != NULL);
    CDSASSERT(node->left == NULL);
    CDSASSERT(node->right == NULL);

    CdsBinaryTree* tree = node->tree;
    if (tree->unref != NULL) {
        tree->unref(node);
    }

    if (node->parent != NULL) {
        // NB: The root node has no parent
        if (node->parent->left == node) {
            node->parent->left = NULL;
        } else {
            CDSASSERT(node->parent->right != NULL);
            node->parent->right = NULL;
        }
    }

    tree->size--;
}


static void cdsBinaryTreeActionRemoveNodeIfLeaf(CdsBinaryTreeNode* node,
        void* cookie)
{
    (void)cookie; // unused argument
    CDSASSERT(node != NULL);

    if (CdsBinaryTreeIsLeaf(node)) {
        cdsBinaryTreeRemoveLeafNode(node);
    }
}
