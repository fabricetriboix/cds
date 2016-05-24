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
    char*                  name;
    int64_t                capacity;
    int64_t                size;
    CdsBinaryTreeNode*     root;
    CdsBinaryTreeNodeUnref unref;
};



/*---------------------------------+
 | Public function implementations |
 +---------------------------------*/


CdsBinaryTree* CdsBinaryTreeCreate(const char* name, int64_t capacity,
        CdsBinaryTreeNodeUnref unref)
{
    CdsBinaryTree* tree = CdsMallocZ(sizeof(*tree));

    if (name != NULL) {
        tree->name = strdup(name);
        CDSASSERT(tree->name != NULL);
    }
    if (capacity > 0) {
        tree->capacity = capacity;
    }
    tree->unref = unref;

    return tree;
}


void CdsBinaryTreeDestroy(CdsBinaryTree* tree)
{
    CDSASSERT(tree != NULL);

    if (tree->root != NULL) {
        CdsBinaryTreeRemoveNode(tree->root);
    }
    free(tree->name);
    free(tree);
}


const char* CdsBinaryTreeName(const CdsBinaryTree* tree)
{
    CDSASSERT(tree != NULL);
    return tree->name;
}


int64_t CdsBinaryTreeCapacity(const CdsBinaryTree* tree)
{
    CDSASSERT(tree != NULL);
    return tree->capacity;
}


int64_t CdsBinaryTreeSize(const CdsBinaryTree* tree)
{
    CDSASSERT(tree != NULL);
    return tree->size;
}


bool CdsBinaryTreeIsEmpty(const CdsBinaryTree* tree)
{
    CDSASSERT(tree != NULL);
    return (tree->size <= 0);
}


bool CdsBinaryTreeIsFull(const CdsBinaryTree* tree)
{
    CDSASSERT(tree != NULL);
    bool isFull = false;
    if ((tree->capacity > 0) && (tree->size >= tree->capacity)) {
        isFull = true;
    }
    return isFull;
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

    CdsBinaryTree* tree = parent->tree;
    bool ret = false;
    if ((parent->left == NULL) && !CdsBinaryTreeIsFull(tree)) {
        child->tree = tree;
        child->parent = parent;
        child->left = NULL;
        child->right = NULL;
        parent->left = child;
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

    CdsBinaryTree* tree = parent->tree;
    bool ret = false;
    if ((parent->right == NULL) && !CdsBinaryTreeIsFull(tree)) {
        child->tree = tree;
        child->parent = parent;
        child->left = NULL;
        child->right = NULL;
        parent->right = child;
        tree->size++;
        ret = true;
    }
    return ret;
}


void CdsBinaryTreeRemoveNode(CdsBinaryTreeNode* node)
{
    CDSASSERT(node != NULL);
    CDSASSERT(node->tree != NULL);

    CdsBinaryTree* tree = node->tree;
    CdsBinaryTreeNode* parent = node->parent;

    // NB: No recursion necessary!
    node->flags = 0;
    for (CdsBinaryTreeNode* curr = node; curr != node->parent; ) {
        if (!(curr->flags & CDS_BT_FLAG_LEFT) && (curr->left != NULL)) {
            curr->flags |= CDS_BT_FLAG_LEFT;
            curr = curr->left;
            curr->flags = 0;

        } else if (!(curr->flags & CDS_BT_FLAG_RIGHT) && (curr->right != NULL)){
            curr->flags |= CDS_BT_FLAG_RIGHT;
            curr = curr->right;
            curr->flags = 0;

        } else {
            CdsBinaryTreeNode* tmp = curr->parent;
            if (tree->unref != NULL) {
                tree->unref(curr);
            }
            tree->size--;
            curr = tmp;
        }
    }

    if (parent == NULL) {
        // We removed the root node
        CDSASSERT(tree->size == 0);
        CDSASSERT(tree->root == node);
        tree->root = NULL;

    } else {
        CDSASSERT(tree->size > 0);
        if (parent->left == node) {
            parent->left = NULL;
        } else {
            CDSASSERT(parent->right == NULL);
            parent->right = NULL;
        }
    }
}


CdsBinaryTreeNode* CdsBinaryTreeRoot(const CdsBinaryTree* tree)
{
    CDSASSERT(tree != NULL);
    return tree->root;
}


CdsBinaryTreeNode* CdsBinaryTreeLeftNode(const CdsBinaryTreeNode* node)
{
    CDSASSERT(node != NULL);
    return node->left;
}


CdsBinaryTreeNode* CdsBinaryTreeRightNode(const CdsBinaryTreeNode* node)
{
    CDSASSERT(node != NULL);
    return node->right;
}


CdsBinaryTreeNode* CdsBinaryTreeParentNode(const CdsBinaryTreeNode* node)
{
	CDSASSERT(node !=NULL);
	return node->parent;
}


bool CdsBinaryTreeIsLeaf(const CdsBinaryTreeNode* node)
{
    CDSASSERT(node != NULL);
    return (node->left == NULL) && (node->right == NULL);
}


CdsBinaryTree* CdsBinaryTreeMerge(const char* name, CdsBinaryTreeNode* root,
        CdsBinaryTree* left, CdsBinaryTree* right)
{
    CDSASSERT(root != NULL);
    CDSASSERT(left != NULL);
    CDSASSERT(right != NULL);

    int64_t capacity = 0;
    if ((left->capacity > 0) && (right->capacity > 0)) {
        CDSASSERT(left->size <= left->capacity);
        CDSASSERT(right->size <= right->capacity);
        capacity = left->capacity + right->capacity;
    }

    CdsBinaryTree* tree = CdsBinaryTreeCreate(name, capacity, left->unref);
    CdsBinaryTreeSetRoot(tree, root);

    tree->root->left = left->root;
    tree->root->right = right->root;
    tree->size += left->size + right->size;

    free(left->name);
    free(left);
    free(right->name);
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
            if (!action(curr, cookie)) {
                break;
            }
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


void CdsBinaryTreeTraverseInOrder(CdsBinaryTreeNode* node,
        CdsBinaryTreeNodeAction action, void* cookie)
{
    CDSASSERT(node != NULL);
    CDSASSERT(action != NULL);

    // NB: No recursion necessary!
    node->flags = 0;
    for (CdsBinaryTreeNode* curr = node; curr != node->parent; ) {
        if (!(curr->flags & CDS_BT_FLAG_LEFT) && (curr->left != NULL)) {
            curr->flags |= CDS_BT_FLAG_LEFT;
            curr = curr->left;
            curr->flags = 0;

        } else if (!(curr->flags & CDS_BT_FLAG_RIGHT) && (curr->right != NULL)){
            if (!(curr->flags & CDS_BT_FLAG_VISITED)) {
                if (!action(curr, cookie)) {
                    break;
                }
                curr->flags |= CDS_BT_FLAG_VISITED;
            }
            curr->flags |= CDS_BT_FLAG_RIGHT;
            curr = curr->right;
            curr->flags = 0;

        } else {
            if (!(curr->flags & CDS_BT_FLAG_VISITED)) {
                if (!action(curr, cookie)) {
                    break;
                }
                curr->flags |= CDS_BT_FLAG_VISITED;
            }

            curr = curr->parent;
        }
    }
}


void CdsBinaryTreeTraversePostOrder(CdsBinaryTreeNode* node,
        CdsBinaryTreeNodeAction action, void* cookie)
{
    CDSASSERT(node != NULL);
    CDSASSERT(action != NULL);

    // NB: No recursion necessary!
    node->flags = 0;
    for (CdsBinaryTreeNode* curr = node; curr != node->parent; ) {
        if (!(curr->flags & CDS_BT_FLAG_LEFT) && (curr->left != NULL)) {
            curr->flags |= CDS_BT_FLAG_LEFT;
            curr = curr->left;
            curr->flags = 0;

        } else if (!(curr->flags & CDS_BT_FLAG_RIGHT) && (curr->right != NULL)){
            curr->flags |= CDS_BT_FLAG_RIGHT;
            curr = curr->right;
            curr->flags = 0;

        } else {
            if (!action(curr, cookie)) {
                break;
            }
            curr = curr->parent;
        }
    }
}
