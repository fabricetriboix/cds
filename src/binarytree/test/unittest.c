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
#include "rttest.h"
#include <string.h>


typedef struct {
    CdsBinaryTreeNode node;
    int ref;
    int level;
    int rank;
} TestNode;

static int gNumberOfNodesInExistence = 0;

static void testNodeUnref(CdsBinaryTreeNode* tnode)
{
    TestNode* node = (TestNode*)tnode;
    node->ref--;
    if (node->ref <= 0) {
        free(node);
        gNumberOfNodesInExistence--;
    }
}

static TestNode* testNodeAlloc(int level, int rank)
{
    TestNode* node = malloc(sizeof(*node));
    memset(node, 0, sizeof(*node));
    node->ref = 1;
    node->level = level;
    node->rank = rank;
    gNumberOfNodesInExistence++;
    return node;
}

CdsBinaryTree* gTree = NULL;


/*

The test binary tree looks like this, with the (level, rank) for each node:

                              (0,0)
                                |
                     +----------+----------+
                     |                     |
                   (1,0)                 (1,1)
                     |                     |
              +------+----+         +------+------+
              |           |         |             |
            (2,0)       (2,1)     (2,2)          NULL
                                    |
                              +-----+----+
                              |          |
                             NULL      (3,5)
 */

typedef struct {
    int nextLevel;
    int nextRank;
    bool ok;
} TraverseData;

#define MAGIC_LEVEL_DONE 0xcafedeca
#define MAGIC_RANK_DONE  0xdeadbeef

static void testNodeActionPreOrder(CdsBinaryTreeNode* tnode, void* cookie)
{
    TestNode* node = (TestNode*)tnode;
    TraverseData* d = (TraverseData*)cookie;
    if ((node->level != d->nextLevel) || (node->rank != d->nextRank)) {
        d->ok = false;
    }

    if ((node->level == 0) && (node->rank == 0)) {
        d->nextLevel = 1;
        d->nextRank = 0;

    } else if ((node->level == 1) && (node->rank == 0)) {
        d->nextLevel = 2;
        d->nextRank = 0;

    } else if ((node->level == 2) && (node->rank == 0)) {
        d->nextLevel = 2;
        d->nextRank = 1;

    } else if ((node->level == 2) && (node->rank == 1)) {
        d->nextLevel = 1;
        d->nextRank = 1;

    } else if ((node->level == 1) && (node->rank == 1)) {
        d->nextLevel = 2;
        d->nextRank = 2;

    } else if ((node->level == 2) && (node->rank == 2)) {
        d->nextLevel = 3;
        d->nextRank = 5;

    } else if ((node->level == 3) && (node->rank == 5)) {
        d->nextLevel = MAGIC_LEVEL_DONE;
        d->nextRank = MAGIC_RANK_DONE;
    }
}


RTT_GROUP_START(TestCdsBinaryTree, 0x00040001u, NULL, NULL)

RTT_TEST_START(cds_should_create_binary_tree)
{
    gTree = CdsBinaryTreeCreate("MyBinaryTree", 7, testNodeUnref);
    RTT_ASSERT(gTree != NULL);
}
RTT_TEST_END

RTT_TEST_START(cds_should_get_binary_tree_name)
{
    const char* name = CdsBinaryTreeName(gTree);
    RTT_EXPECT(strcmp(name, "MyBinaryTree") == 0);
}
RTT_TEST_END

RTT_TEST_START(cds_binary_tree_size_should_be_0_after_creation)
{
    RTT_ASSERT(CdsBinaryTreeSize(gTree) == 0);
    RTT_ASSERT(gNumberOfNodesInExistence == 0);
}
RTT_TEST_END

RTT_TEST_START(cds_binary_tree_should_be_empty_after_creation)
{
    RTT_ASSERT(CdsBinaryTreeIsEmpty(gTree));
}
RTT_TEST_END

RTT_TEST_START(cds_binary_tree_set_root)
{
    TestNode* root = testNodeAlloc(0, 0);
    RTT_ASSERT(CdsBinaryTreeSetRoot(gTree, (CdsBinaryTreeNode*)root));
    RTT_ASSERT(CdsBinaryTreeRoot(gTree) == (CdsBinaryTreeNode*)root);
}
RTT_TEST_END

RTT_TEST_START(cds_binary_tree_should_failed_to_set_root_twice)
{
    TestNode* root = testNodeAlloc(1, 1);
    RTT_ASSERT(!CdsBinaryTreeSetRoot(gTree, (CdsBinaryTreeNode*)root));
    testNodeUnref((CdsBinaryTreeNode*)root);
}
RTT_TEST_END

RTT_TEST_START(cds_binary_tree_root_node_should_be_leaf_after_set_root)
{
    CdsBinaryTreeNode* root = CdsBinaryTreeRoot(gTree);
    RTT_ASSERT(CdsBinaryTreeIsLeaf(root));
}
RTT_TEST_END

RTT_TEST_START(cds_binary_tree_size_should_be_1_after_set_root)
{
    RTT_ASSERT(CdsBinaryTreeSize(gTree) == 1);
    RTT_ASSERT(gNumberOfNodesInExistence == 1);
}
RTT_TEST_END

RTT_TEST_START(cds_binary_tree_capacity_should_be_7_after_set_root)
{
    RTT_ASSERT(CdsBinaryTreeCapacity(gTree) == 7);
}
RTT_TEST_END

RTT_TEST_START(cds_binary_tree_should_not_be_empty_after_set_root)
{
    RTT_ASSERT(!CdsBinaryTreeIsEmpty(gTree));
}
RTT_TEST_END

RTT_TEST_START(cds_binary_tree_should_not_be_full_after_set_root)
{
    RTT_ASSERT(!CdsBinaryTreeIsFull(gTree));
}
RTT_TEST_END

RTT_TEST_START(cds_binary_tree_should_insert_left_of_root)
{
    CdsBinaryTreeNode* root = CdsBinaryTreeRoot(gTree);
    RTT_ASSERT(root != NULL);
    TestNode* node = testNodeAlloc(1, 0);
    RTT_ASSERT(CdsBinaryTreeInsertLeft(root, (CdsBinaryTreeNode*)node));
    RTT_ASSERT(CdsBinaryTreeLeftNode(root) == (CdsBinaryTreeNode*)node);
}
RTT_TEST_END

RTT_TEST_START(cds_binary_tree_should_fail_to_insert_left_twice)
{
    CdsBinaryTreeNode* root = CdsBinaryTreeRoot(gTree);
    RTT_ASSERT(root != NULL);
    TestNode* node = testNodeAlloc(99, 99);
    RTT_ASSERT(!CdsBinaryTreeInsertLeft(root, (CdsBinaryTreeNode*)node));
    testNodeUnref((CdsBinaryTreeNode*)node);
}
RTT_TEST_END

RTT_TEST_START(cds_binary_tree_should_insert_right_of_root)
{
    CdsBinaryTreeNode* root = CdsBinaryTreeRoot(gTree);
    RTT_ASSERT(root != NULL);
    TestNode* node = testNodeAlloc(1, 1);
    RTT_ASSERT(CdsBinaryTreeInsertRight(root, (CdsBinaryTreeNode*)node));
    RTT_ASSERT(CdsBinaryTreeRightNode(root) == (CdsBinaryTreeNode*)node);
}
RTT_TEST_END

RTT_TEST_START(cds_binary_tree_should_fail_to_insert_right_twice)
{
    CdsBinaryTreeNode* root = CdsBinaryTreeRoot(gTree);
    RTT_ASSERT(root != NULL);
    TestNode* node = testNodeAlloc(99, 99);
    RTT_ASSERT(!CdsBinaryTreeInsertRight(root, (CdsBinaryTreeNode*)node));
    testNodeUnref((CdsBinaryTreeNode*)node);
}
RTT_TEST_END

RTT_TEST_START(cds_binary_tree_size_should_be_3_after_inserting_root_children)
{
    RTT_ASSERT(CdsBinaryTreeSize(gTree) == 3);
    RTT_ASSERT(gNumberOfNodesInExistence == 3);
}
RTT_TEST_END

RTT_TEST_START(cds_binary_tree_capacity_should_be_7_after_inserting_root_children)
{
    RTT_ASSERT(CdsBinaryTreeCapacity(gTree) == 7);
}
RTT_TEST_END

RTT_TEST_START(cds_binary_tree_should_insert_2_nodes_under_left)
{
    CdsBinaryTreeNode* root = CdsBinaryTreeRoot(gTree);
    RTT_ASSERT(root != NULL);
    CdsBinaryTreeNode* parent = CdsBinaryTreeLeftNode(root);
    RTT_ASSERT(parent != NULL);

    TestNode* child1 = testNodeAlloc(2, 0);
    RTT_ASSERT(CdsBinaryTreeInsertLeft(parent, (CdsBinaryTreeNode*)child1));
    RTT_ASSERT(CdsBinaryTreeLeftNode(parent) == (CdsBinaryTreeNode*)child1);
    RTT_ASSERT(CdsBinaryTreeRightNode(parent) == NULL);

    TestNode* child2 = testNodeAlloc(2, 1);
    RTT_ASSERT(CdsBinaryTreeInsertRight(parent, (CdsBinaryTreeNode*)child2));
    RTT_ASSERT(CdsBinaryTreeRightNode(parent) == (CdsBinaryTreeNode*)child2);
}
RTT_TEST_END

RTT_TEST_START(cds_binary_tree_should_insert_2_nodes_under_right)
{
    CdsBinaryTreeNode* root = CdsBinaryTreeRoot(gTree);
    RTT_ASSERT(root != NULL);
    CdsBinaryTreeNode* parent = CdsBinaryTreeRightNode(root);
    RTT_ASSERT(parent != NULL);

    TestNode* child = testNodeAlloc(2, 2);
    RTT_ASSERT(CdsBinaryTreeInsertLeft(parent, (CdsBinaryTreeNode*)child));
    RTT_ASSERT(CdsBinaryTreeLeftNode(parent) == (CdsBinaryTreeNode*)child);
    RTT_ASSERT(CdsBinaryTreeRightNode(parent) == NULL);

    TestNode* grandchild = testNodeAlloc(3, 5);
    RTT_ASSERT(CdsBinaryTreeInsertRight((CdsBinaryTreeNode*)child,
                (CdsBinaryTreeNode*)grandchild));
    RTT_ASSERT(CdsBinaryTreeRightNode((CdsBinaryTreeNode*)child)
            == (CdsBinaryTreeNode*)grandchild);
    RTT_ASSERT(CdsBinaryTreeLeftNode((CdsBinaryTreeNode*)child) == NULL);
}
RTT_TEST_END

RTT_TEST_START(cds_binary_tree_size_should_be_7)
{
    RTT_ASSERT(CdsBinaryTreeSize(gTree) == 7);
    RTT_ASSERT(gNumberOfNodesInExistence == 7);
}
RTT_TEST_END

RTT_TEST_START(cds_binary_tree_capacity_should_be_7)
{
    RTT_ASSERT(CdsBinaryTreeCapacity(gTree) == 7);
}
RTT_TEST_END

RTT_TEST_START(cds_binary_tree_should_not_be_empty)
{
    RTT_ASSERT(!CdsBinaryTreeIsEmpty(gTree));
}
RTT_TEST_END

RTT_TEST_START(cds_binary_tree_should_be_full)
{
    RTT_ASSERT(CdsBinaryTreeIsFull(gTree));
}
RTT_TEST_END

RTT_TEST_START(cds_binary_tree_traverse_pre_order)
{
    CdsBinaryTreeNode* root = CdsBinaryTreeRoot(gTree);
    RTT_ASSERT(root != NULL);

    TraverseData d;
    d.nextLevel = 0;
    d.nextRank = 0;
    d.ok = true;

    CdsBinaryTreeTraversePreOrder(root, testNodeActionPreOrder, &d);

    RTT_ASSERT(d.ok);
    RTT_ASSERT(d.nextLevel = MAGIC_LEVEL_DONE);
    RTT_ASSERT(d.nextRank = MAGIC_RANK_DONE);

}
RTT_TEST_END

RTT_TEST_START(cds_binary_tree_should_remove_left_node)
{
    CdsBinaryTreeNode* root = CdsBinaryTreeRoot(gTree);
    RTT_ASSERT(root != NULL);

    TestNode* left = (TestNode*)CdsBinaryTreeLeftNode(root);
    RTT_ASSERT(left != NULL);
    RTT_ASSERT(left->level == 1);
    RTT_ASSERT(left->rank == 0);

    CdsBinaryTreeRemoveNode((CdsBinaryTreeNode*)left);

    RTT_ASSERT(CdsBinaryTreeSize(gTree) == 4);
    RTT_ASSERT(gNumberOfNodesInExistence == 4);
}
RTT_TEST_END

RTT_TEST_START(cds_binary_tree_should_destroy_tree)
{
    CdsBinaryTreeDestroy(gTree);
    RTT_ASSERT(gNumberOfNodesInExistence == 0);
}
RTT_TEST_END

RTT_GROUP_END(TestCdsBinaryTree,
        cds_should_create_binary_tree,
        cds_should_get_binary_tree_name,
        cds_binary_tree_size_should_be_0_after_creation,
        cds_binary_tree_should_be_empty_after_creation,
        cds_binary_tree_set_root,
        cds_binary_tree_should_failed_to_set_root_twice,
        cds_binary_tree_root_node_should_be_leaf_after_set_root,
        cds_binary_tree_size_should_be_1_after_set_root,
        cds_binary_tree_capacity_should_be_7_after_set_root,
        cds_binary_tree_should_not_be_empty_after_set_root,
        cds_binary_tree_should_not_be_full_after_set_root,
        cds_binary_tree_should_insert_left_of_root,
        cds_binary_tree_should_fail_to_insert_left_twice,
        cds_binary_tree_should_insert_right_of_root,
        cds_binary_tree_should_fail_to_insert_right_twice,
        cds_binary_tree_size_should_be_3_after_inserting_root_children,
        cds_binary_tree_capacity_should_be_7_after_inserting_root_children,
        cds_binary_tree_should_insert_2_nodes_under_left,
        cds_binary_tree_should_insert_2_nodes_under_right,
        cds_binary_tree_size_should_be_7,
        cds_binary_tree_capacity_should_be_7,
        cds_binary_tree_should_not_be_empty,
        cds_binary_tree_should_be_full,
        cds_binary_tree_traverse_pre_order,
        cds_binary_tree_should_remove_left_node,
        cds_binary_tree_should_destroy_tree);
