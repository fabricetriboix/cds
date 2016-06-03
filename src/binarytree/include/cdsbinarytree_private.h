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

#ifndef CDSBINARYTREE_PRIVATE_h_
#define CDSBINARYTREE_PRIVATE_h_



/*----------------+
 | Types & Macros |
 +----------------*/


/* Forward declaration */
typedef struct CdsBinaryTree CdsBinaryTree;


/* Binary tree node */
struct CdsBinaryTreeNode
{
    struct CdsBinaryTree*     tree;
    struct CdsBinaryTreeNode* parent;
    struct CdsBinaryTreeNode* left;
    struct CdsBinaryTreeNode* right;
    uint8_t                   flags;
} CdsBinaryTreeNode;



#endif /* CDSBINARYTREE_PRIVATE_h_ */
