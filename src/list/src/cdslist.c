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

#include "cdslist.h"
#include <stdlib.h>
#include <string.h>



/*-------+
 | Types |
 +-------*/


struct CdsList
{
    char*            name;
    int64_t          size;
    int64_t          capacity;
    CdsListItem      head;
    CdsListItemUnref unref;
};



/*---------------------------------+
 | Public function implementations |
 +---------------------------------*/


CdsList* CdsListCreate(const char* name, int64_t capacity,
        CdsListItemUnref unref)
{
    CdsList* list = CdsMallocZ(sizeof(*list));

    if (name != NULL) {
        list->name = strdup(name);
    }
    if (capacity > 0) {
        list->capacity = capacity;
    }
    list->head.next = &(list->head);
    list->head.prev = &(list->head);
    list->head.parent = list;
    list->unref = unref;

    return list;
}


void CdsListDestroy(CdsList* list)
{
    CDSASSERT(list != NULL);

    while (!CdsListIsEmpty(list)) {
        CdsListItem* tmp = CdsListPopFront(list);
        CDSASSERT(tmp != NULL);
        if (list->unref != NULL) {
            list->unref(tmp);
        }
    }
    free(list->name);
    free(list);
}


const char* CdsListName(const CdsList* list)
{
    CDSASSERT(list != NULL);
    return list->name;
}


int64_t CdsListSize(const CdsList* list)
{
    CDSASSERT(list != NULL);
    return list->size;
}


int64_t CdsListCapacity(const CdsList* list)
{
    CDSASSERT(list != NULL);
    return list->capacity;
}


bool CdsListIsEmpty(const CdsList* list)
{
    CDSASSERT(list != NULL);
    return list->size <= 0;
}


bool CdsListIsFull(const CdsList* list)
{
    CDSASSERT(list != NULL);
    bool isFull = false;
    if ((list->capacity > 0) && (list->size >= list->capacity)) {
        isFull = true;
    }
    return isFull;
}


bool CdsListPushFront(CdsList* list, CdsListItem* item)
{
    CDSASSERT(list != NULL);
    return CdsListInsertAfter(&(list->head), item);
}


bool CdsListPushBack(CdsList* list, CdsListItem* item)
{
    CDSASSERT(list != NULL);
    return CdsListInsertBefore(&(list->head), item);
}


bool CdsListInsertAfter(CdsListItem* pos, CdsListItem* item)
{
    CDSASSERT(pos != NULL);
    CdsList* list = pos->parent;
    CDSASSERT(list != NULL);
    CDSASSERT(item != NULL);

    bool inserted = false;
    if ((list->capacity <= 0) || (list->size < list->capacity)) {
        item->parent = list;
        item->next = pos->next;
        item->prev = pos;
        pos->next->prev = item;
        pos->next = item;
        list->size++;
        inserted = true;
    }
    return inserted;
}


bool CdsListInsertBefore(CdsListItem* pos, CdsListItem* item)
{
    CDSASSERT(pos != NULL);
    CdsList* list = pos->parent;
    CDSASSERT(list != NULL);
    CDSASSERT(item != NULL);

    bool inserted = false;
    if ((list->capacity <= 0) || (list->size < list->capacity)) {
        item->parent = list;
        item->next = pos;
        item->prev = pos->prev;
        pos->prev->next = item;
        pos->prev = item;
        list->size++;
        inserted = true;
    }
    return inserted;
}


CdsListItem* CdsListFront(const CdsList* list)
{
    CDSASSERT(list != NULL);
    if (list->size <= 0) {
        return NULL;
    }
    return list->head.next;
}


CdsListItem* CdsListBack(const CdsList* list)
{
    CDSASSERT(list != NULL);
    if (list->size <= 0) {
        return NULL;
    }
    return list->head.prev;
}


CdsListItem* CdsListNext(const CdsListItem* item)
{
    CDSASSERT(item != NULL);
    CdsList* list = item->parent;
    CDSASSERT(list != NULL);
    CdsListItem* next = item->next;
    if (next == &(list->head)) {
        next = NULL;
    }
    return next;
}


CdsListItem* CdsListPrev(const CdsListItem* item)
{
    CDSASSERT(item != NULL);
    CdsList* list = item->parent;
    CDSASSERT(list != NULL);
    CdsListItem* prev = item->prev;
    if (prev == &(list->head)) {
        prev = NULL;
    }
    return prev;
}


void CdsListRemove(CdsListItem* item)
{
    CDSASSERT(item != NULL);
    CdsList* list = item->parent;
    CDSASSERT(list != NULL);

    item->next->prev = item->prev;
    item->prev->next = item->next;
    list->size--;

    item->next = NULL;
    item->prev = NULL;
    item->parent = NULL;
}


CdsListItem* CdsListPopFront(CdsList* list)
{
    CdsListItem* front = CdsListFront(list);
    if (front != NULL) {
        CdsListRemove(front);
    }
    return front;
}


CdsListItem* CdsListPopBack(CdsList* list)
{
    CdsListItem* back = CdsListBack(list);
    if (back != NULL) {
        CdsListRemove(back);
    }
    return back;
}
