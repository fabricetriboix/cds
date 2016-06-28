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

#include <stdlib.h>
#include <stdio.h>

#include "cdslist.h"


typedef struct
{
    CdsListItem item;
    int ref;
    long long value;
} MyItem;

static MyItem* myItemCreate(long long value)
{
    MyItem* item = CdsMallocZ(sizeof(*item));
    item->ref = 1;
    item->value = value;
    return item;
}

static void myItemUnref(CdsListItem* litem)
{
    MyItem* item = (MyItem*)litem;
    item->ref--;
    if (item->ref <= 0) {
        free(item);
    }
}


int main(int argc, char** argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: ./cdslistperf ITEMCOUNT\n");
        exit(2);
    }
    long long count;
    if (sscanf(argv[1], "%lld", &count) != 1) {
        fprintf(stderr, "Invalid COUNT argument: '%s'\n", argv[1]);
        exit(2);
    }
    if (count <= 0) {
        fprintf(stderr, "Invalid COUNT: %lld\n", count);
        exit(2);
    }

    CdsList* list = CdsListCreate(NULL, 0, myItemUnref);

    printf("Inserting %lld items at the front\n", count / 2);
    for (long long i = 0; i < (count / 2); i++) {
        CdsListPushFront(list, (CdsListItem*)myItemCreate(i));
    }

    printf("Inserting %lld items at the back\n", count / 2);
    for (long long i = (count / 2); i < count; i++) {
        CdsListPushBack(list, (CdsListItem*)myItemCreate(i));
    }

    printf("Walking through the list\n");
    CDSLIST_FOREACH(list, MyItem, item) {
        volatile long long x = item->value;
        (void)x;
    }

    printf("Popping %lld items from the front\n", count / 2);
    for (long long i = 0; i < (count / 2); i++) {
        MyItem* item = (MyItem*)CdsListPopFront(list);
        myItemUnref((CdsListItem*)item);
    }

    printf("Popping %lld items from the back\n", count / 2);
    for (long long i = (count / 2); i < count; i++) {
        MyItem* item = (MyItem*)CdsListPopFront(list);
        myItemUnref((CdsListItem*)item);
    }

    CdsListDestroy(list);
    return 0;
}
