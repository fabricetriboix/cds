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

/** Map (aka associatve array)
 *
 * \defgroup cdsmap Map (aka associatve array)
 * \addtogroup cdsmap
 * @{
 *
 * Map.
 */

#ifndef CDSMAP_h_
#define CDSMAP_h_

#include "cdscommon.h"
#include "cdsmap_private.h"



/*----------------+
 | Types & Macros |
 +----------------*/


/** Opaque type that represents a map */
typedef struct CdsMap CdsMap;


/** Opaque type that represents an iterator */
typedef struct CdsMapIterator CdsMapIterator;


/** Map item
 *
 * You can "derive" from this structure, as long as it remains at the top of
 * your own structure definition. For example:
 *
 *     typedef struct {
 *         CdsMapItem item;
 *         int x;
 *         float y;
 *         char* z;
 *     } MyItem;
 */
typedef struct CdsMapItem CdsMapItem;


/** Prototype of a function to remove a reference to a key
 *
 * This function should decrement the internal reference counter of the key by
 * one. If the internal reference counter reaches zero, the key should be
 * freed.
 */
typedef void (*CdsMapKeyUnref)(void* key);


/** Prototype of a function to remove a reference to a item
 *
 * This function should decrement the internal reference counter of the item by
 * one. If the internal reference counter reaches zero, the item should be
 * freed.
 */
typedef void (*CdsMapItemUnref)(CdsMapItem* item);


/** Prototype of a function to compare two keys
 *
 * \param leftKey  [in] Left-hand side of the comparison
 * \param rightKey [in] Right-hand side of the comparison
 * \param cookie   [in] Cookie for this function
 *
 * \return -1 if `leftKey` < `rightKey`, 0 if `leftKey` == `rightKey`
 *         or 1 if `leftKey` > `rightKey`
 */
typedef int (*CdsMapCompare)(void* leftKey, void* rightKey, void* cookie);



/*------------------------------+
 | Public function declarations |
 +------------------------------*/


/** Create a map
 *
 * \param name      [in] Name for this map; may be NULL
 * \param capacity  [in] Max # of items the map can store; 0 = no limit
 * \param compare   [in] Function to compare two keys; must not be NULL
 * \param cookie    [in] Cookie for the previous function
 * \param keyUnref  [in] Function to remove a reference to a key; may be NULL
 *                       if you don't need it
 * \param itemUnref [in] Function to remove a reference to a item; may be NULL
 *                       if you don't need it
 *
 * \return The newly-allocated map, never NULL
 */
CdsMap* CdsMapCreate(const char* name, int64_t capacity,
        CdsMapCompare compare, void* cookie,
        CdsMapKeyUnref keyUnref, CdsMapItemUnref itemUnref);


/** Destroy a map
 *
 * Any key and item remaining in the map will be unreferenced.
 *
 * \param map [in,out] Map to destroy; must not be NULL
 */
void CdsMapDestroy(CdsMap* map);


/** Clear a map
 *
 * This will remove all items in the map.
 *
 * \param map [in,out] Map to clear; must not be NULL
 */
void CdsMapClear(CdsMap* map);


/** Get the map's name
 *
 * \param map [in] Map to query; must not be NULL
 *
 * \return The map's name, which may be NULL
 */
const char* CdsMapName(const CdsMap* map);


/** Get the maximum number of items this map can hold
 *
 * \param map [in] Map to query; must not be NULL
 *
 * \return The `map` capacity, or 0 if no limit
 */
int64_t CdsMapCapacity(const CdsMap* map);


/** Get the number of items currently in the map
 *
 * \param map [in] Map to query; must not be NULL
 *
 * \return The number of items currently in the `map`
 */
int64_t CdsMapSize(const CdsMap* map);


/** Test if the map is empty
 *
 * \param map [in] Map to query; must not be NULL
 *
 * \return `true` if the map is empty, `false` otherwise
 */
bool CdsMapIsEmpty(const CdsMap* map);


/** Test if the map is full
 *
 * If the `map` capacity has been set to 0 at creation, this function always
 * returns `false`.
 *
 * \param map [in] Map to query; must not be NULL
 *
 * \return `true` if the map is full, `false` otherwise
 */
bool CdsMapIsFull(const CdsMap* map);


/** Insert an item into the map
 *
 * If this function succeeds, the ownership of both `key` and `item` will be
 * transfered to the `map`. If you want to do more work on `key` or `item`, you
 * should take a reference from them prior to calling this function.
 *
 * If an item already exists for the given `key`, it is replaced by the new
 * `item` and de-referenced.
 *
 * **IMPORTANT** Do not insert the same `item` or `key` pointers twice! This
 * will break everything in the map implementation, most probably resulting in
 * random crashes!
 *
 * If you want to use the same key, either allocate a new key object with the
 * same value, use key objects that don't need dereferencing and set the
 * `keyUnref` function to NULL when creating the map, or add a reference to the
 * key prior to calling this function.
 *
 * \param map  [in,out] Map to manipulate; must not be NULL
 * \param key  [in]     Item key
 * \param item [in]     Item to insert into `map`; must not be NULL
 *
 * \return `true` if OK, `false` if map is full
 */
bool CdsMapInsert(CdsMap* map, void* key, CdsMapItem* item);


/** Search for an item in a map
 *
 * The ownership of `key` remains with the caller. The ownership of the returned
 * item remains with the `map`.
 *
 * \param map [in] Map to search; must not be NULL
 * \param key [in] Key to search for
 *
 * \return The found item, to NULL if not found
 */
CdsMapItem* CdsMapSearch(CdsMap* map, void* key);


/** Remove an item identified by its key
 *
 * If found, both the item and its key will be unreferenced (but not the `key`
 * argument, whose ownership remains with the caller).
 *
 * \param map [in,out] Map to manipulate; must not be NULL
 * \param key [in]     Key to search for
 *
 * \return `true` if item found and removed, `false` if item not found
 */
bool CdsMapRemove(CdsMap* map, void* key);


/** Remove an item directly
 *
 * \param map  [in,out] Map to manipulate; must not be NULL
 * \param item [in,out] Item to remove; must not be NULL
 */
void CdsMapItemRemove(CdsMap* map, CdsMapItem* item);


/** Create a map iterator
 *
 * You must call `CdsMapIteratorDestroy()` on the created iterator once you
 * finished with it.
 *
 * Only one iterator can be active at any time on a given map. Items will be
 * iterated in an in-order manner, either in ascending or descending order,
 * depending on the value of the `ascending` argument.
 *
 * You must not insert or remove items while iterating through the map.
 *
 * \param map       [in] Map to iterate through; must not be NULL
 * \param ascending [in] Whether to iterate in ascending or descending order
 *
 * \return An iterator object, never NULL
 */
CdsMapIterator* CdsMapIteratorCreate(CdsMap* map, bool ascending);


/** Iterate to next item
 *
 * The first time this function is called on an iterator, the very first item
 * will be returned. For example, for a `CDSMAP_ITERATOR_ASCENDING` iterator,
 * the first call to this function will return the item with the smallest key.
 *
 * \param iterator [in,out] Iterator to use; must not be NULL
 * \param pKey     [out]    Key for the corresponding item; set to NULL if you
 *                          don't need the key
 *
 * \return Next item, or NULL if the end is reached
 */
CdsMapItem* CdsMapIteratorNext(CdsMapIterator* iterator, void** pKey);


/** Destroy an iterator
 *
 * \param iterator [in,out] Iterator to destroy; must not be NULL
 */
void CdsMapIteratorDestroy(CdsMapIterator* iterator);



#endif /* CDSMAP_h_ */
/* @} */
