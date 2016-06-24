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

#include "cdsmap.h"
#include <stdlib.h>
#include <string.h>



/*----------------+
 | Macros & Types |
 +----------------*/


#define CDSMAP_FLAG_LEFT  0x01
#define CDSMAP_FLAG_RIGHT 0x02


struct CdsMap
{
    CdsMapItem*     root; // Keep this at the top, it's necessary for unit tests
    char*           name;
    int64_t         capacity;
    int64_t         size;
    CdsMapCompare   compare;
    void*           cookie;
    CdsMapKeyUnref  keyUnref;
    CdsMapItemUnref itemUnref;
    bool            hasIterator;
};


struct CdsMapIterator
{
    CdsMap*     map;
    bool        ascending;
    CdsMapItem* current;
};



/*------------------------------+
 | Privte function declarations |
 +------------------------------*/


/** Check if the given item is the left child of its parent
 *
 * \param item [in] Item to test; must not be NULL
 *
 * \return `true` if item is the left child of its parent; `false` if it is the
 *         right child or has no parent
 */
static inline bool cdsMapIsLeftChild(const CdsMapItem* item)
{
    CDSASSERT(item != NULL);
    return (item->parent != NULL) && (item->parent->left == item);
}


/** Check if the given item is the right child of its parent
 *
 * \param item [in] Item to test; must not be NULL
 *
 * \return `true` if item is the right child of its parent; `false` if it is the
 *         left child or has no parent
 */
static bool cdsMapIsRightChild(const CdsMapItem* item)
{
    CDSASSERT(item != NULL);
    return (item->parent != NULL) && (item->parent->right == item);
}


/** Check if the given item is a leaf
 *
 * \param item [in] Item to test; must not be NULL
 *
 * \return `true` if `item` is a leaf, `false` if it has at least one child
 */
static bool cdsMapIsLeaf(const CdsMapItem* item)
{
    CDSASSERT(item != NULL);
    return (item->left == NULL) && (item->right == NULL);
}


/** Go down a sub-tree as far as possible on the left
 *
 * \param item [in] The sub-tree root, must not be NULL
 *
 * \return The left-most item in the sub-tree, never NULL
 */
static CdsMapItem* cdsMapDigLeft(CdsMapItem* item);


/** Go down a sub-tree as far as possible on the right
 *
 * \param item [in] The sub-tree root, must not be NULL
 *
 * \return The right-most item in the sub-tree, never NULL
 */
static CdsMapItem* cdsMapDigRight(CdsMapItem* item);


/** Perform a single RR rotation of the sub-tree rooted at `subroot`
 *
 * \param map     [in,out] Map to manipulate; must not be NULL
 * \param subroot [in,out] Root of the sub-tree to rotate; must not be NULL
 *
 * \return The root of the rotated sub-tree
 */
static CdsMapItem* cdsMapRotateRightRight(CdsMap* map, CdsMapItem* subroot);


/** Perform a single LL rotation of the sub-tree rooted at `subroot`
 *
 * \param map     [in,out] Map to manipulate; must not be NULL
 * \param subroot [in,out] Root of the sub-tree to rotate; must not be NULL
 *
 * \return The root of the rotated sub-tree
 */
static CdsMapItem* cdsMapRotateLeftLeft(CdsMap* map, CdsMapItem* subroot);


/** Perform a double RL rotation of the sub-tree rooted at `subroot`
 *
 * \param map     [in,out] Map to manipulate; must not be NULL
 * \param subroot [in,out] Root of the sub-tree to rotate; must not be NULL
 *
 * \return The root of the rotated sub-tree
 */
static CdsMapItem* cdsMapRotateRightLeft(CdsMap* map, CdsMapItem* subroot);


/** Perform a double LR rotation of the sub-tree rooted at `subroot`
 *
 * \param map     [in,out] Map to manipulate; must not be NULL
 * \param subroot [in,out] Root of the sub-tree to rotate; must not be NULL
 *
 * \return The root of the rotated sub-tree
 */
static CdsMapItem* cdsMapRotateLeftRight(CdsMap* map, CdsMapItem* subroot);


/** Insert a new item left or right of the given `item`
 *
 * This function takes care of rebalancing the whole tree.
 *
 * \param map        [in,out] Map to manipulate; must not be NULL
 * \param item       [in,out] Where to insert `newitem`; must not be NULL
 * \param newitem    [in,out] New item to insert; must not be NULL
 * \param key        [in]     Key for the new item
 * \param insertLeft [in]     Should `newitem` be inserted at left or right?
 */
static void cdsMapInsertOne(CdsMap* map, CdsMapItem* item,
        CdsMapItem* newitem, void* key, bool insertLeft);



/*---------------------------------+
 | Public function implementations |
 +---------------------------------*/


CdsMap* CdsMapCreate(const char* name, int64_t capacity, CdsMapCompare compare,
        void* cookie, CdsMapKeyUnref keyUnref, CdsMapItemUnref itemUnref)
{
    CDSASSERT(compare != NULL);

    CdsMap* map = CdsMallocZ(sizeof(*map));

    if (name != NULL) {
        map->name = strdup(name);
        CDSASSERT(map->name != NULL);
    }
    if (capacity > 0) {
        map->capacity = capacity;
    }
    map->compare = compare;
    map->cookie = cookie;
    map->keyUnref = keyUnref;
    map->itemUnref = itemUnref;

    return map;
}


void CdsMapDestroy(CdsMap* map)
{
    CDSASSERT(map != NULL);

    if (map->root != NULL) {
        // Traverse the tree in post-order fashion
        map->root->flags = 0;
        for (CdsMapItem* curr = map->root; curr != NULL; ) {
            if (!(curr->flags & CDSMAP_FLAG_LEFT) && (curr->left != NULL)) {
                curr->flags |= CDSMAP_FLAG_LEFT;
                curr = curr->left;
                curr->flags = 0;

            } else if (   !(curr->flags & CDSMAP_FLAG_RIGHT)
                        && (curr->right != NULL)) {
                curr->flags |= CDSMAP_FLAG_RIGHT;
                curr = curr->right;
                curr->flags = 0;

            } else {
                CdsMapItem* tmp = curr->parent;
                if (map->keyUnref != NULL) {
                    map->keyUnref(curr->key);
                }
                if (map->itemUnref != NULL) {
                    map->itemUnref(curr);
                }
                curr = tmp;
            }
        }
    }

    free(map->name);
    free(map);
}


const char* CdsMapName(const CdsMap* map)
{
    CDSASSERT(map != NULL);
    return map->name;
}


int64_t CdsMapCapacity(const CdsMap* map)
{
    CDSASSERT(map != NULL);
    return map->capacity;
}


int64_t CdsMapSize(const CdsMap* map)
{
    CDSASSERT(map != NULL);
    return map->size;
}


bool CdsMapIsEmpty(const CdsMap* map)
{
    CDSASSERT(map != NULL);
    return (map->size <= 0);
}


bool CdsMapIsFull(const CdsMap* map)
{
    CDSASSERT(map != NULL);
    bool isFull = false;
    if ((map->capacity > 0) && (map->size >= map->capacity)) {
        isFull = true;
    }
    return isFull;
}


bool CdsMapInsert(CdsMap* map, void* key, CdsMapItem* item)
{
    CDSASSERT(map != NULL);
    CDSASSERT(map->compare != NULL);
    CDSASSERT(item != NULL);

    if (CdsMapIsFull(map)) {
        return false;
    }

    if (map->root == NULL) {
        item->parent = NULL;
        item->left = NULL;
        item->right = NULL;
        item->key = key;
        item->factor = 0;
        map->root = item;
        map->size = 1;
        return true;
    }

    bool inserted = false;
    CdsMapItem* curr = map->root;
    while (!inserted && (curr != NULL)) {
        int cmp = map->compare(key, curr->key, map->cookie);
        if (cmp < 0) {
            // Insert new item left of `curr`
            if (curr->left == NULL) {
                cdsMapInsertOne(map, curr, item, key, true);
                inserted = true;
            } else {
                curr = curr->left;
            }
        } else if (cmp > 0) {
            // Insert item right of `curr`
            if (curr->right == NULL) {
                cdsMapInsertOne(map, curr, item, key, false);
                inserted = true;
            } else {
                curr = curr->right;
            }
        } else {
            // Replace `curr` by `item`
            item->parent = curr->parent;
            item->left = curr->left;
            item->right = curr->right;
            item->key = key;
            item->factor = curr->factor;
            if (cdsMapIsLeftChild(curr)) {
                curr->parent->left = item;
            } else if (cdsMapIsRightChild(curr)) {
                curr->parent->right = item;
            } else {
                map->root = item;
            }
            if (map->keyUnref != NULL) {
                map->keyUnref(curr->key);
            }
            if (map->itemUnref != NULL) {
                map->itemUnref(curr);
            }
            inserted = true;
        }
    }
    CDSASSERT(inserted);
    return true;
}


CdsMapItem* CdsMapSearch(CdsMap* map, void* key)
{
    CDSASSERT(map != NULL);

    bool found = false;
    CdsMapItem* item = map->root;
    while ((item != NULL) && !found) {
        int cmp = map->compare(key, item->key, map->cookie);
        if (cmp < 0) {
            item = item->left;
        } else if (cmp > 0) {
            item = item->right;
        } else {
            found = true;
        }
    }
    return item;
}


bool CdsMapRemove(CdsMap* map, void* key)
{
    CDSASSERT(map != NULL);
    CdsMapItem* item = CdsMapSearch(map, key);
    if (item != NULL) {
        CdsMapItemRemove(map, item);
    }
    return (item != NULL);
}


void CdsMapItemRemove(CdsMap* map, CdsMapItem* item)
{
    CDSASSERT(map != NULL);
    CDSASSERT(map->root != NULL);
    CDSASSERT(item != NULL);

    CDSASSERT(map->size > 0);
    map->size--;

    CdsMapItem* tmp;
    if ((item->left != NULL) && (item->right != NULL)) {
        // Find the previous (or next) in-order item
        // NB: The choice to take the previous or next in-order item is based on
        // the item's factor, in an attempt to minimise the chances of a
        // rotation being required.
        tmp = NULL;
        if (item->factor <= 0) {
            tmp = cdsMapDigRight(item->left); // use previous in-order item
        } else {
            tmp = cdsMapDigLeft(item->right); // use next in-order item
        }
        CDSASSERT(tmp != NULL);
        CDSASSERT((tmp->left == NULL) || (tmp->right == NULL));

        // Exchange `item` and `tmp`
        // NB: This temporarily breaks the binary search tree, until we actually
        // delete `item`

        CdsMapItem* itemParent = item->parent;
        bool itemIsLeftChild = cdsMapIsLeftChild(item);
        CdsMapItem* itemLeft = item->left;
        CDSASSERT(itemLeft != NULL);
        CdsMapItem* itemRight = item->right;
        CDSASSERT(itemRight != NULL);

        CdsMapItem* tmpParent = tmp->parent;
        CDSASSERT(tmpParent != NULL);
        bool tmpIsLeftChild = cdsMapIsLeftChild(tmp);
        CdsMapItem* tmpLeft = tmp->left;
        CdsMapItem* tmpRight = tmp->right;

        tmp->factor = item->factor;
        tmp->parent = itemParent;
        if (itemParent == NULL) {
            map->root = tmp;
        } else if (itemIsLeftChild) {
            itemParent->left = tmp;
        } else {
            itemParent->right = tmp;
        }
        if (tmp != itemLeft) {
            tmp->left = itemLeft;
            if (itemLeft != NULL) {
                itemLeft->parent = tmp;
            }
        } else {
            tmp->left = item;
        }
        if (tmp != itemRight) {
            tmp->right = itemRight;
            if (itemRight != NULL) {
                itemRight->parent = tmp;
            }
        } else {
            tmp->right = item;
        }

        if (item != tmpParent) {
            item->parent = tmpParent;
            if (tmpIsLeftChild) {
                tmpParent->left = item;
            } else {
                tmpParent->right = item;
            }
        } else {
            item->parent = tmp;
        }
        item->left = tmpLeft;
        if (tmpLeft != NULL) {
            tmpLeft->parent = item;
        }
        item->right = tmpRight;
        if (tmpRight != NULL) {
            tmpRight->parent = item;
        }
    }

    // Here, `item` is a leaf or has only one child; remove `item` from the tree
    CDSASSERT((item->left == NULL) || (item->right == NULL));
    tmp = NULL;
    if (item->left != NULL) {
        tmp = item->left;
    } else if (item->right != NULL) {
        tmp = item->right;
    }
    if (tmp != NULL) {
        tmp->parent = item->parent;
    }
    bool leftDecrease = false;
    if (cdsMapIsLeftChild(item)) {
        item->parent->left = tmp;
        leftDecrease = true;
    } else if (cdsMapIsRightChild(item)) {
        item->parent->right = tmp;
    } else {
        map->root = tmp;
    }

    // Retrace the tree
    //
    // This is done by going up the tree, starting from `item->parent`, and
    // finishing when the current node's factor does not change, or if we reach
    // the root of the tree. Please remember we just removed `item` from the
    // tree.
    //
    // For each node going up, we check if the left or right subtree decreased.

    bool balanced = false;
    for (   CdsMapItem* subroot = item->parent;
            (subroot != NULL) && !balanced;
            subroot = subroot->parent) {
        if (leftDecrease) {
            // The sub-tree on the left of `subroot` decreased its height by 1
            switch (subroot->factor) {
            case -1 :
                subroot->factor = 0;
                break;
            case 0 :
                subroot->factor = 1;
                balanced = true;
                break;
            case 1 :
                tmp = subroot->right;
                CDSASSERT(tmp != NULL);
                if (tmp->factor >= 0) {
                    subroot = cdsMapRotateRightRight(map, subroot);
                } else {
                    subroot = cdsMapRotateRightLeft(map, subroot);
                }
                if (subroot->factor != 0) {
                    balanced = true;
                }
                break;
            default :
                CDSPANIC_MSG("Impossible balance factor: %d",
                        (int)subroot->factor);
            }
        } else {
            // The sub-tree on the right of `subroot` decreased its height by 1
            switch (subroot->factor) {
            case -1 :
                tmp = subroot->left;
                CDSASSERT(tmp != NULL);
                if (tmp->factor <= 0) {
                    subroot = cdsMapRotateLeftLeft(map, subroot);
                } else {
                    subroot = cdsMapRotateLeftRight(map, subroot);
                }
                if (subroot->factor != 0) {
                    balanced = true;
                }
                break;
            case 0 :
                subroot->factor = -1;
                balanced = true;
                break;
            case 1 :
                subroot->factor = 0;
                break;
            default :
                CDSPANIC_MSG("Impossible balance factor: %d",
                        (int)subroot->factor);
            }
        }
        leftDecrease = cdsMapIsLeftChild(subroot);
    }

    // Dereference `item` and its key
    if (map->keyUnref != NULL) {
        map->keyUnref(item->key);
    }
    if (map->itemUnref != NULL) {
        map->itemUnref(item);
    }
}


CdsMapIterator* CdsMapIteratorCreate(CdsMap* map, bool ascending)
{
    CDSASSERT(map != NULL);
    CDSASSERT(!(map->hasIterator));

    CdsMapIterator* iterator = CdsMallocZ(sizeof(*iterator));

    map->hasIterator = true;
    iterator->map = map;
    iterator->ascending = ascending;

    if (map->root == NULL) {
        iterator->current = NULL;
    } else {
        if (ascending) {
            iterator->current = cdsMapDigLeft(map->root);
        } else {
            iterator->current = cdsMapDigRight(map->root);
        }
    }
    return iterator;
}


CdsMapItem* CdsMapIteratorNext(CdsMapIterator* iterator)
{
    CDSASSERT(iterator != NULL);

    CdsMapItem* curr = iterator->current;
    CdsMapItem* next = NULL;
    if (curr != NULL) {
        if (iterator->ascending) {
            // NB: We have already visited all the nodes on the left side of
            // `curr`
            CdsMapItem* parent = curr->parent;
            if (parent == NULL) {
                next = NULL; // `curr` was actually the root node
            } else if (parent->right == NULL) {
                next = parent;
            } else {
                next = cdsMapDigLeft(parent->right);
            }

        } else {
            // NB: We have already visited all the nodes on the right side of
            // `curr`
            CdsMapItem* parent = curr->parent;
            if (parent == NULL) {
                next = NULL; // `curr` was actually the root node
            } else if (parent->left == NULL) {
                next = parent;
            } else {
                next = cdsMapDigRight(parent->left);
            }
        }
        iterator->current = next;
    }
    return curr;
}


void CdsMapIteratorDestroy(CdsMapIterator* iterator)
{
    CDSASSERT(iterator != NULL);
    CDSASSERT(iterator->map != NULL);
    iterator->map->hasIterator = false;
    free(iterator);
}



/*----------------------------------+
 | Private function implementations |
 +----------------------------------*/


static CdsMapItem* cdsMapDigLeft(CdsMapItem* item)
{
    CDSASSERT(item != NULL);

    item->flags = 0;
    while ((item->left != NULL) && !(item->flags & CDSMAP_FLAG_LEFT)) {
        item->flags |= CDSMAP_FLAG_LEFT;
        item = item->left;
        item->flags = 0;
    }
    return item;
}


static CdsMapItem* cdsMapDigRight(CdsMapItem* item)
{
    CDSASSERT(item != NULL);

    item->flags = 0;
    while ((item->right != NULL) && !(item->flags & CDSMAP_FLAG_RIGHT)) {
        item->flags |= CDSMAP_FLAG_RIGHT;
        item = item->right;
        item->flags = 0;
    }
    return item;
}


static CdsMapItem* cdsMapRotateRightRight(CdsMap* map, CdsMapItem* subroot)
{
    CDSASSERT(map != NULL);
    CDSASSERT(subroot != NULL);

    CdsMapItem* item = subroot->right;
    CDSASSERT(item != NULL);
    CDSASSERT(item->factor >= 0);

    // Make `item` the root of the sub-tree
    if (cdsMapIsLeftChild(subroot)) {
        subroot->parent->left = item;
    } else if (cdsMapIsRightChild(subroot)) {
        subroot->parent->right = item;
    } else {
        CDSASSERT(subroot->parent == NULL);
        map->root = item;
    }
    item->parent = subroot->parent;

    // Make the left child of `item` the right child of `subroot`
    CdsMapItem* tmp = item->left;
    subroot->right = tmp;
    if (tmp != NULL) {
        tmp->parent = subroot;
    }

    // Make `subroot` the left child of `item`
    item->left = subroot;
    subroot->parent = item;

    // Update balance factors
    if (item->factor == 0) {
        subroot->factor = 1;
        item->factor = -1;
    } else {
        CDSASSERT(item->factor == 1);
        subroot->factor = 0;
        item->factor = 0;
    }

    return item;
}


static CdsMapItem* cdsMapRotateLeftLeft(CdsMap* map, CdsMapItem* subroot)
{
    CDSASSERT(map != NULL);
    CDSASSERT(subroot != NULL);

    CdsMapItem* item = subroot->left;
    CDSASSERT(item != NULL);
    CDSASSERT(item->factor <= 0);

    // Make `item` the root of the sub-tree
    if (cdsMapIsLeftChild(subroot)) {
        subroot->parent->left = item;
    } else if (cdsMapIsRightChild(subroot)) {
        subroot->parent->right = item;
    } else {
        CDSASSERT(subroot->parent == NULL);
        map->root = item;
    }
    item->parent = subroot->parent;

    // Make the right child of `item` the left child of `subroot`
    CdsMapItem* tmp = item->right;
    subroot->left = tmp;
    if (tmp != NULL) {
        tmp->parent = subroot;
    }

    // Make `subroot` the right child of `item`
    item->right = subroot;
    subroot->parent = item;

    // Update balance factors
    if (item->factor == 0) {
        subroot->factor = -1;
        item->factor = 1;
    } else {
        CDSASSERT(item->factor == -1);
        subroot->factor = 0;
        item->factor = 0;
    }

    return item;
}


static CdsMapItem* cdsMapRotateRightLeft(CdsMap* map, CdsMapItem* subroot)
{
    CDSASSERT(map != NULL);
    CDSASSERT(subroot != NULL);

    CdsMapItem* item = subroot->right;
    CDSASSERT(item != NULL);
    CDSASSERT(item->factor < 0);

    CdsMapItem* grandchild = item->left;
    CDSASSERT(grandchild != NULL);

    // Make `grandchild` the root of the subtree
    if (cdsMapIsLeftChild(subroot)) {
        subroot->parent->left = grandchild;
    } else if (cdsMapIsRightChild(subroot)) {
        subroot->parent->right = grandchild;
    } else {
        map->root = grandchild;
    }
    grandchild->parent = subroot->parent;

    // Make the right child of `grandchild` the left child of `item`
    CdsMapItem* tmp = grandchild->right;
    item->left = tmp;
    if (tmp != NULL) {
        tmp->parent = item;
    }

    // Make `item` the right child of `grandchild`
    grandchild->right = item;
    item->parent = grandchild;

    // Make the left child of `grandchild` the right child of `subroot`
    tmp = grandchild->left;
    subroot->right = tmp;
    if (tmp != NULL) {
        tmp->parent = subroot;
    }

    // Make `subroot` the left child of `grandchild`
    grandchild->left = subroot;
    subroot->parent = grandchild;

    // Update balance factors
    switch (grandchild->factor) {
    case -1 :
        subroot->factor = 0;
        item->factor = 1;
        break;
    case 0 :
        subroot->factor = 0;
        item->factor = 0;
        break;
    case 1 :
        subroot->factor = -1;
        item->factor = 0;
        break;
    default :
        CDSPANIC_MSG("Impossible factor: %d", (int)grandchild->factor);
    }
    grandchild->factor = 0;

    return grandchild;
}


static CdsMapItem* cdsMapRotateLeftRight(CdsMap* map, CdsMapItem* subroot)
{
    CDSASSERT(map != NULL);
    CDSASSERT(subroot != NULL);

    CdsMapItem* item = subroot->left;
    CDSASSERT(item != NULL);
    CDSASSERT(item->factor > 0);

    CdsMapItem* grandchild = item->right;
    CDSASSERT(grandchild != NULL);

    // Make `grandchild` the root of the subtree
    if (cdsMapIsLeftChild(subroot)) {
        subroot->parent->left = grandchild;
    } else if (cdsMapIsRightChild(subroot)) {
        subroot->parent->right = grandchild;
    } else {
        map->root = grandchild;
    }
    grandchild->parent = subroot->parent;

    // Make the left child of `grandchild` the right child of `item`
    CdsMapItem* tmp = grandchild->left;
    item->right = tmp;
    if (tmp != NULL) {
        tmp->parent = item;
    }

    // Make `item` the left child of `grandchild`
    grandchild->left = item;
    item->parent = grandchild;

    // Make the right child of `grandchild` the left child of `subroot`
    tmp = grandchild->right;
    subroot->left = tmp;
    if (tmp != NULL) {
        tmp->parent = subroot;
    }

    // Make `subroot` the right child of `grandchild`
    grandchild->right = subroot;
    subroot->parent = grandchild;

    // Update balance factors
    switch (grandchild->factor) {
    case -1 :
        subroot->factor = 1;
        item->factor = 0;
        break;
    case 0 :
        subroot->factor = 0;
        item->factor = 0;
        break;
    case 1 :
        subroot->factor = 0;
        item->factor = -1;
        break;
    default :
        CDSPANIC_MSG("Impossible factor: %d", (int)grandchild->factor);
    }
    grandchild->factor = 0;

    return grandchild;
}


/* XXX rm this
static CdsMapItem* cdsMapRotateLeft(CdsMapItem* item)
{
    CDSASSERT(item != NULL);
    CDSASSERT(item->map != NULL);

    CdsMapItem* top = item;

    CdsMapItem* left = item->left;
    CDSASSERT(left != NULL);
    if (left->factor < 0) {
        // LL rotation
        item->left = left->right;
        if (item->left != NULL) {
            item->left->parent = item;
        }

        // Set `left` as the new top node of this sub-tree
        left->parent = item->parent;
        if (item->parent == NULL) {
            map->root = left;
        } else {
            if (cdsMapIsLeftChild(item)) {
                item->parent->left = left;
            } else {
                CDSASSERT(cdsMapIsRightChild(item));
                item->parent->right = left;
            }
        }

        left->right = item;
        item->parent = left;

        left->factor = 0;
        item->factor = 0;

        top = left;

    } else if (left->factor > 0) {
        // LR rotation
        top = left->right;

        left->right = top->left;
        if (left->right != NULL) {
            left->right->parent = left;
        }

        top->left = left;
        left->parent = top;

        item->left = top->right;
        if (item->left != NULL) {
            item->left->parent = item;
        }

        top->right = item;
        item->parent = top;

        if (cdsMapIsLeftChild(item)) {
            item->parent->left = top;
        } else if (cdsMapIsRightChild(item)) {
            item->parent->right = top;
        } else {
            CDSASSERT(item->parent == NULL);
            map->root = top;
        }

        switch (top->factor) {
        case -1 :
            item->factor = 1;
            left->factor = 0;
            break;
        case 0 :
            item->factor = 0;
            left->factor = 0;
            break;
        case 1 :
            item->factor = 0;
            left->factor = -1;
            break;
        default :
            CDSPANIC_MSG("Impossible balance factor: %d", top->factor);
        }
        top->factor = 0;
    }

    return top;
}


static CdsMapItem* cdsMapRotateRight(CdsMapItem* item)
{
    CDSASSERT(item != NULL);
    CDSASSERT(item->map != NULL);

    CdsMapItem* top = item;

    CdsMapItem* right = item->right;
    CDSASSERT(right != NULL);
    if (right->factor < 0) {
        // RR rotation
        item->right = right->left;
        if (item->right != NULL) {
            item->right->parent = item;
        }

        // Set `right` as the new top node of this sub-tree
        right->parent = item->parent;
        if (item->parent == NULL) {
            map->root = right;
        } else {
            if (cdsMapIsRightChild(item)) {
                item->parent->right = right;
            } else {
                CDSASSERT(cdsMapIsLeftChild(item));
                item->parent->left = right;
            }
        }

        right->left = item;
        item->parent = right;

        right->factor = 0;
        item->factor = 0;

        top = right;

    } else if (right->factor > 0) {
        // RL rotation
        CdsMapItem* top = right->left;

        right->left = top->right;
        if (right->left != NULL) {
            right->left->parent = right;
        }

        top->right = right;
        right->parent = top;

        item->right = top->left;
        if (item->right != NULL) {
            item->right->parent = item;
        }

        top->left = item;
        item->parent = top;

        if (cdsMapIsRightChild(item)) {
            item->parent->right = top;
        } else if (cdsMapIsLeftChild(item)) {
            item->parent->left = top;
        } else {
            CDSASSERT(item->parent == NULL);
            map->root = top;
        }

        switch (top->factor) {
        case -1 :
            item->factor = 0;
            right->factor = 1;
            break;
        case 0 :
            item->factor = 0;
            right->factor = 0;
            break;
        case 1 :
            item->factor = -1;
            right->factor = 0;
            break;
        default :
            CDSPANIC_MSG("Impossible balance factor: %d", top->factor);
        }
        top->factor = 0;
    }

    return top;
}
*/


static void cdsMapInsertOne(CdsMap* map, CdsMapItem* item,
        CdsMapItem* newitem, void* key, bool insertLeft)
{
    CDSASSERT(map != NULL);
    CDSASSERT(item != NULL);
    CDSASSERT(newitem != NULL);

    // Initialise the new item
    newitem->parent = item;
    newitem->left = NULL;
    newitem->right = NULL;
    newitem->key = key;
    newitem->factor = 0;

    // Make `newitem` the child of `item`
    map->size++;
    if (insertLeft) {
        CDSASSERT(item->left == NULL);
        item->left = newitem;
        if (item->right != NULL) {
            CDSASSERT(cdsMapIsLeaf(item->right));
            item->factor = 0;
            return; // Balance factor of `item->parent` does not change
        }
        item->factor = -1;
    } else {
        CDSASSERT(item->right == NULL);
        item->right = newitem;
        if (item->left != NULL) {
            CDSASSERT(cdsMapIsLeaf(item->left));
            item->factor = 0;
            return; // Balance factor of `item->parent` does not change
        }
        item->factor = 1;
    }


    // Retrace the tree
    //
    // If we are here, the insertion changes the balance factor of
    // `item->parent`, so we need to update the balance factors of all ancestors
    // and perform rotations as necessary.
    //
    // This is done by going up the tree, starting from `item` and finishing
    // when the current node's factor does not change, or if we reach the root
    // of the tree.
    //
    // For each node going up, we check if the left or right subtree grew.

    bool balanced = false;
    for (   CdsMapItem* subroot = item->parent;
            (subroot != NULL) && !balanced;
            subroot = subroot->parent) {
        if (item == subroot->right) {
            // The sub-tree on the right of `subroot` increased its height by 1
            switch (subroot->factor) {
            case -1 :
                subroot->factor = 0;
                balanced = true;
                break;
            case 0 :
                subroot->factor = 1;
                break;
            case 1 :
                if (item->factor >= 0) {
                    subroot = cdsMapRotateRightRight(map, subroot);
                } else {
                    subroot = cdsMapRotateRightLeft(map, subroot);
                }
                balanced = true;
                break;
            default :
                CDSPANIC_MSG("Impossible balance factor: %d",
                        (int)subroot->factor);
            }
        } else {
            CDSASSERT(item == subroot->left);
            // The sub-tree on the left of `subroot` increased its height by 1
            switch (subroot->factor) {
            case -1 :
                if (item->factor <= 0) {
                    subroot = cdsMapRotateLeftLeft(map, subroot);
                } else {
                    subroot = cdsMapRotateLeftRight(map, subroot);
                }
                balanced = true;
                break;
            case 0 :
                subroot->factor = -1;
                break;
            case 1 :
                subroot->factor = 0;
                balanced = true;
                break;
            default :
                CDSPANIC_MSG("Impossible balance factor: %d",
                        (int)subroot->factor);
            }
        }
        item = subroot;
    }
}
