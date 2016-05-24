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
    CdsListItemRefFn ref;
};



/*---------------------------------+
 | Public function implementations |
 +---------------------------------*/


CdsList* CdsListCreate(const char* name,
        CdsListItemRefFn refFn, int64_t capacity)
{
    CdsList* self = malloc(sizeof(*self));
    CDSASSERT(self != NULL);
    memset(self, 0, sizeof(*self));

    if (name != NULL) {
        self->name = strdup(name);
    }
    if (capacity > 0) {
        self->capacity = capacity;
    }
    self->head.next = &(self->head);
    self->head.prev = &(self->head);
    self->head.parent = self;
    self->ref = refFn;

    return self;
}


void CdsListDestroy(CdsList* self)
{
    CDSASSERT(self != NULL);

    while (!CdsListIsEmpty(self)) {
        (void)CdsListPopFront(self);
    }
    free(self->name);
    free(self);
}


const char* CdsListName(const CdsList* self)
{
    CDSASSERT(self != NULL);
    return self->name;
}


int64_t CdsListSize(const CdsList* self)
{
    CDSASSERT(self != NULL);
    return self->size;
}


int64_t CdsListCapacity(const CdsList* self)
{
    CDSASSERT(self != NULL);
    return self->capacity;
}


bool CdsListIsEmpty(const CdsList* self)
{
    CDSASSERT(self != NULL);
    return self->size <= 0;
}


bool CdsListIsFull(const CdsList* self)
{
    CDSASSERT(self != NULL);
    bool isFull = false;
    if ((self->capacity > 0) && (self->size >= self->capacity)) {
        isFull = true;
    }
    return isFull;
}


bool CdsListPushFront(CdsList* self, CdsListItem* item)
{
    CDSASSERT(self != NULL);
    return CdsListInsertAfter(&(self->head), item);
}


bool CdsListPushBack(CdsList* self, CdsListItem* item)
{
    CDSASSERT(self != NULL);
    return CdsListInsertBefore(&(self->head), item);
}


bool CdsListInsertAfter(CdsListItem* pos, CdsListItem* item)
{
    CDSASSERT(pos != NULL);
    CdsList* list = pos->parent;
    CDSASSERT(list != NULL);
    CDSASSERT(item != NULL);

    bool inserted = false;
    if ((list->capacity <= 0) || (list->size < list->capacity)) {
        if (list->ref != NULL) {
            list->ref(item, 1);
        }
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
        if (list->ref != NULL) {
            list->ref(item, 1);
        }
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


CdsListItem* CdsListFront(const CdsList* self)
{
    CDSASSERT(self != NULL);
    if (self->size <= 0) {
        return NULL;
    }
    return self->head.next;
}


CdsListItem* CdsListBack(const CdsList* self)
{
    CDSASSERT(self != NULL);
    if (self->size <= 0) {
        return NULL;
    }
    return self->head.prev;
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
    if (list->ref != NULL) {
        list->ref(item, -1);
    }
}


CdsListItem* CdsListPopFront(CdsList* self)
{
    CdsListItem* front = CdsListFront(self);
    if (front != NULL) {
        CdsListRemove(front);
    }
    return front;
}


CdsListItem* CdsListPopBack(CdsList* self)
{
    CdsListItem* back = CdsListBack(self);
    if (back != NULL) {
        CdsListRemove(back);
    }
    return back;
}
