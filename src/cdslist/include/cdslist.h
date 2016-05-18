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

/** Double-linked lists
 *
 * \defgroup cdslist Lists
 * \addtogroup cdslist
 * @{
 *
 * Double-linked lists. Please note that this implementation does not do any
 * memory copy. Instead, it relies on reference counters for each individual
 * items such that the items themselves know when they can be freed.
 */

#ifndef CDSLIST_h_
#define CDSLIST_h_

#include "cdscommon.h"



/*----------------+
 | Types & Macros |
 +----------------*/


/** Opaque type that represents a list */
typedef struct CdsList CdsList;


/* List item
 *
 * You can "derive" from this structure, as long as it remains at the top of
 * your own structure definition. For example:
 *
 *     typedef struct {
 *         CdsListItem cdsListItem;
 *         int x;
 *         float y;
 *         char* z;
 *     } MyItem;
 */
typedef struct CdsListItem
{
    CdsList*            parent;
    struct CdsListItem* next;
    struct CdsListItem* prev;
} CdsListItem;


/** Prototype of a function to add or remove references on an item
 *
 * You shoud add the `n` argument to the reference counter of the item. If the
 * reference counter of the item drops to 0, the item is not referenced anymore
 * and must be freed.
 *
 * The `n` argument will only be +1 or -1.
 */
typedef void (*CdsListItemRefFn)(CdsListItem* item, int n);


/** Macro to walk through a list */
#define CDSLIST_FOREACH(_list, _type, _iter) \
    for (   _type* _iter = (_type*)CdsListFront(_list); \
            _iter != NULL; \
            _iter = (_type*)CdsListNext((CdsListItem*)_iter) )


/** Macro to walk through a list backwards */
#define CDSLIST_FOREACH_REVERSE(_list, _type, _iter) \
    for (   _type* _iter = (_type*)CdsListBack(_list); \
            _iter != NULL; \
            _iter = (_type*)CdsListPrev((CdsListItem*)_iter) )



/*------------------------------+
 | Public function declarations |
 +------------------------------*/


/** Create a list
 *
 * \param name     [in] Name for this list; may be NULL
 * \param refFn    [in] Function to add or remove a reference to a list item;
 *                      may be NULL if you don't need it.
 * \param capacity [in] Max # of items the list can hold, or 0 for no limit
 *
 * \return The newly-allocated list, never NULL
 */
CdsList* CdsListCreate(const char* name,
        CdsListItemRefFn refFn, int64_t capacity);


/** Destroy a list
 *
 * Any item in the list will be unreferrenced.
 *
 * \param self [in,out] The list to destroy; must not be NULL.
 */
void CdsListDestroy(CdsList* self);


/** Get the list's name
 *
 * \param self [in] The list to query; must not be NULL
 */
const char* CdsListName(const CdsList* self);


/** Get the number of items currently in the list
 *
 * \param self [in] The list to query; must not be NULL
 */
int64_t CdsListSize(const CdsList* self);


/** Get the list's capacity
 *
 * \param self [in] The list to query; must not be NULL
 *
 * \return The list capacity, or 0 if no limit
 */
int64_t CdsListCapacity(const CdsList* self);


/** Test if a list is empty
 *
 * \param self [in] The list to query; must not be NULL
 *
 * \return `true` if the list is empty, `false` otherwise
 */
bool CdsListIsEmpty(const CdsList* self);


/** Test is a list is full
 *
 * This function will always return `false` if no limit has been set on the list
 * capacity when `CdsListCreate()` has been called.
 *
 * \param self [in] The list to query; must not be NULL
 *
 * \return `true` if the list is full, `false` otherwise
 */
bool CdsListIsFull(const CdsList* self);


/** Insert an item at the front of the list
 *
 * If no limit has been set to limit the list capacity, this function always
 * succeeds.
 *
 * \param self [in,out] The list where to insert the item; must not be NULL
 * \param item [in,out] The item to insert; must not be NULL
 *
 * \return `true` if success, `false` if the list is full
 */
bool CdsListPushFront(CdsList* self, CdsListItem* item);


/** Insert an item at the back of the list
 *
 * If no limit has been set to limit the list capacity, this function always
 * succeeds.
 *
 * \param self [in,out] The list where to insert the item; must not be NULL
 * \param item [in,out] The item to insert; must not be NULL
 *
 * \return `true` if success, `false` if the list is full
 */
bool CdsListPushBack(CdsList* self, CdsListItem* item);


/** Insert an item after the given item
 *
 * If no limit has been set to limit the list capacity, this function always
 * succeeds.
 *
 * \param pos  [in,out] Item after which to insert new item; must not be NULL
 * \param item [in,out] The item to insert; must not be NULL
 *
 * \return `true` if success, `false` if the list is full
 */
bool CdsListInsertAfter(CdsListItem* pos, CdsListItem* item);


/** Insert an item before the given item
 *
 * If no limit has been set to limit the list capacity, this function always
 * succeeds.
 *
 * \param pos  [in,out] Item before which to insert new item; must not be NULL
 * \param item [in,out] The item to insert; must not be NULL
 *
 * \return `true` if success, `false` if the list is full
 */
bool CdsListInsertBefore(CdsListItem* pos, CdsListItem* item);


/** Get the item at the front of the list
 *
 * Please note the item is not removed from the list, use `CdsListPopFront()`
 * for that effect.
 *
 * \param self [in] The list to query
 *
 * \return The item at the front, or NULL if `list` is empty
 */
CdsListItem* CdsListFront(const CdsList* self);


/** Get the item at the back of the list
 *
 * Please note the item is not removed from the list, use `CdsListPopBack()`
 * for that effect.
 *
 * \param self [in] The list to query
 *
 * \return The item at the back, or NULL if `list` is empty
 */
CdsListItem* CdsListBack(const CdsList* self);


/** Get the next item in the list
 *
 * \param pos [in] Position item
 *
 * \return The item after `pos`, or NULL if no more items
 */
CdsListItem* CdsListNext(const CdsListItem* pos);


/** Get the previous item in the list
 *
 * \param pos [in] Position item
 *
 * \return The item before `pos`, or NULL if no more items
 */
CdsListItem* CdsListPrev(const CdsListItem* pos);


/** Remove the given item from its list
 *
 * \param item [in,out] The item to remove from the list
 */
void CdsListRemove(CdsListItem* item);


/** Pop an item at the front of the list
 *
 * \param self [in,out] The list from where to pop an item
 *
 * \return The popped item, or NULL if list is empty
 */
CdsListItem* CdsListPopFront(CdsList* self);


/** Pop an item at the back of the list
 *
 * \param self [in,out] The list from where to pop an item
 *
 * \return The popped item, or NULL if list is empty
 */
CdsListItem* CdsListPopBack(CdsList* self);



#endif /* CDSLIST_h_ */
/* @} */