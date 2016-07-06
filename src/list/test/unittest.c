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
#include "rttest.h"

#include <string.h>


typedef struct {
    CdsListItem CdsListItem;
    int         ref;
    int         x;
} TestItem;

static int gNumberOfItemsInExistence = 0;

static void testItemUnref(CdsListItem* cdsListItem)
{
    TestItem* item = (TestItem*)cdsListItem;
    item->ref--;
    if (item->ref <= 0) {
        free(item);
        gNumberOfItemsInExistence--;
    }
}

static TestItem* testItemAlloc(void)
{
    TestItem* item = malloc(sizeof(*item));
    memset(item, 0, sizeof(*item));
    item->ref = 1;
    gNumberOfItemsInExistence++;
    return item;
}

CdsList* gList = NULL;


RTT_GROUP_START(TestCdsListSmall, 0x00030001u, NULL, NULL)

RTT_TEST_START(cds_should_create_small_list)
{
    gList = CdsListCreate("SmallList", 20, testItemUnref);
    RTT_ASSERT(gList != NULL);
}
RTT_TEST_END

RTT_TEST_START(cds_should_get_small_list_name)
{
    const char* name = CdsListName(gList);
    RTT_EXPECT(strcmp(name, "SmallList") == 0);
}
RTT_TEST_END

RTT_TEST_START(cds_small_list_size_should_be_0_after_creation)
{
    RTT_ASSERT(CdsListSize(gList) == 0);
    RTT_ASSERT(0 == gNumberOfItemsInExistence);
}
RTT_TEST_END

RTT_TEST_START(cds_small_list_capacity_should_be_20_after_creation)
{
    RTT_ASSERT(CdsListCapacity(gList) == 20);
}
RTT_TEST_END

RTT_TEST_START(cds_small_list_should_be_empty_after_creation)
{
    RTT_ASSERT(CdsListIsEmpty(gList));
}
RTT_TEST_END

RTT_TEST_START(cds_small_list_should_not_be_full_after_creation)
{
    RTT_ASSERT(!CdsListIsFull(gList));
}
RTT_TEST_END

RTT_TEST_START(cds_should_push_5_items_at_front_of_small_list)
{
    for (int i = 0; i < 5; i++) {
        TestItem* item = testItemAlloc();
        item->x = (4 - i) * 10;
        RTT_ASSERT(CdsListPushFront(gList, (CdsListItem*)item));
    }
    RTT_EXPECT(5 == gNumberOfItemsInExistence);
}
RTT_TEST_END

RTT_TEST_START(cds_small_list_size_should_be_5_when_partially_filled)
{
    RTT_ASSERT(CdsListSize(gList) == 5);
    RTT_ASSERT(5 == gNumberOfItemsInExistence);
}
RTT_TEST_END

RTT_TEST_START(cds_small_list_capacity_should_remain_20_when_partially_filled)
{
    RTT_ASSERT(CdsListCapacity(gList) == 20);
}
RTT_TEST_END

RTT_TEST_START(cds_small_list_should_not_be_empty_when_partially_filled)
{
    RTT_ASSERT(!CdsListIsEmpty(gList));
}
RTT_TEST_END


RTT_TEST_START(cds_small_list_should_not_be_full_when_partially_filled)
{
    RTT_ASSERT(!CdsListIsFull(gList));
}
RTT_TEST_END

RTT_TEST_START(cds_should_push_15_items_at_back_of_small_list)
{
    for (int i = 0; i < 15; i++) {
        TestItem* item = testItemAlloc();
        item->x = (i + 5) * 10;
        RTT_ASSERT(CdsListPushBack(gList, (CdsListItem*)item));
    }
    RTT_EXPECT(20 == gNumberOfItemsInExistence);
}
RTT_TEST_END

RTT_TEST_START(cds_small_list_size_should_be_20_when_full)
{
    RTT_ASSERT(CdsListSize(gList) == 20);
    RTT_ASSERT(20 == gNumberOfItemsInExistence);
}
RTT_TEST_END

RTT_TEST_START(cds_small_list_capacity_should_remain_20_when_full)
{
    RTT_ASSERT(CdsListCapacity(gList) == 20);
}
RTT_TEST_END

RTT_TEST_START(cds_small_list_should_not_be_empty_when_full)
{
    RTT_ASSERT(!CdsListIsEmpty(gList));
}
RTT_TEST_END

RTT_TEST_START(cds_small_list_should_be_full_when_full)
{
    RTT_ASSERT(CdsListIsFull(gList));
}
RTT_TEST_END

RTT_TEST_START(cds_should_fail_to_push_at_the_front_of_small_list_when_full)
{
    TestItem* item = testItemAlloc();
    RTT_ASSERT(!CdsListPushFront(gList, (CdsListItem*)item));
    testItemUnref((CdsListItem*)item);
}
RTT_TEST_END

RTT_TEST_START(cds_should_fail_to_push_at_the_back_of_small_list_when_full)
{
    TestItem* item = testItemAlloc();
    RTT_ASSERT(!CdsListPushBack(gList, (CdsListItem*)item));
    testItemUnref((CdsListItem*)item);
}
RTT_TEST_END

RTT_TEST_START(cds_should_walk_through_small_list)
{
    int i = 0;
    CDSLIST_FOREACH(gList, TestItem, item) {
        RTT_EXPECT(item->x == i);
        i += 10;
    }
    RTT_EXPECT(200 == i);
}
RTT_TEST_END

RTT_TEST_START(cds_should_walk_backwards_through_small_list)
{
    int i = 190;
    CDSLIST_FOREACH_REVERSE(gList, TestItem, item) {
        RTT_EXPECT(item->x == i);
        i -= 10;
    }
    RTT_EXPECT(-10 == i);
}
RTT_TEST_END

RTT_TEST_START(cds_should_pop_5_items_from_front_of_small_list)
{
    for (int i = 0; i < 5; i++) {
        TestItem* item = (TestItem*)CdsListPopFront(gList);
        RTT_ASSERT(item != NULL);
        RTT_EXPECT((i * 10) == item->x);
        testItemUnref((CdsListItem*)item);
    }
    RTT_EXPECT(15 == gNumberOfItemsInExistence);
}
RTT_TEST_END

RTT_TEST_START(cds_should_pop_5_items_from_back_of_small_list)
{
    for (int i = 0; i < 5; i++) {
        TestItem* item = (TestItem*)CdsListPopBack(gList);
        RTT_ASSERT(item != NULL);
        RTT_EXPECT((19 - i) * 10 == item->x);
        testItemUnref((CdsListItem*)item);
    }
    RTT_EXPECT(10 == gNumberOfItemsInExistence);
}
RTT_TEST_END

RTT_TEST_START(cds_should_insert_item_before_front_of_small_list)
{
    TestItem* front = (TestItem*)CdsListFront(gList);
    RTT_ASSERT(front != NULL);
    RTT_EXPECT(50 == front->x);
    TestItem* item = testItemAlloc();
    item->x = 40;
    RTT_ASSERT(CdsListInsertBefore((CdsListItem*)front, (CdsListItem*)item));
    RTT_EXPECT(11 == gNumberOfItemsInExistence);
}
RTT_TEST_END

RTT_TEST_START(cds_should_insert_item_after_front_of_small_list)
{
    TestItem* front = (TestItem*)CdsListFront(gList);
    RTT_ASSERT(front != NULL);
    RTT_EXPECT(40 == front->x);
    TestItem* item = testItemAlloc();
    item->x = 45;
    RTT_ASSERT(CdsListInsertAfter((CdsListItem*)front, (CdsListItem*)item));
    RTT_EXPECT(12 == gNumberOfItemsInExistence);
}
RTT_TEST_END

RTT_TEST_START(cds_should_insert_item_before_back_of_small_list)
{
    TestItem* back = (TestItem*)CdsListBack(gList);
    RTT_ASSERT(back != NULL);
    RTT_EXPECT(140 == back->x);
    TestItem* item = testItemAlloc();
    item->x = 135;
    RTT_ASSERT(CdsListInsertBefore((CdsListItem*)back, (CdsListItem*)item));
    RTT_EXPECT(13 == gNumberOfItemsInExistence);
}
RTT_TEST_END

RTT_TEST_START(cds_should_insert_item_after_back_of_small_list)
{
    TestItem* back = (TestItem*)CdsListBack(gList);
    RTT_ASSERT(back != NULL);
    RTT_EXPECT(140 == back->x);
    TestItem* item = testItemAlloc();
    item->x = 150;
    RTT_ASSERT(CdsListInsertAfter((CdsListItem*)back, (CdsListItem*)item));
    RTT_EXPECT(14 == gNumberOfItemsInExistence);
}
RTT_TEST_END

RTT_TEST_START(cds_small_list_size_should_be_14_after_direct_inserts)
{
    RTT_ASSERT(CdsListSize(gList) == 14);
    RTT_ASSERT(14 == gNumberOfItemsInExistence);
}
RTT_TEST_END

RTT_TEST_START(cds_small_list_capacity_should_remain_20_after_direct_inserts)
{
    RTT_ASSERT(CdsListCapacity(gList) == 20);
}
RTT_TEST_END

RTT_TEST_START(cds_small_list_should_not_be_empty_after_direct_inserts)
{
    RTT_ASSERT(!CdsListIsEmpty(gList));
}
RTT_TEST_END

RTT_TEST_START(cds_small_list_should_not_be_full_after_direct_inserts)
{
    RTT_ASSERT(!CdsListIsFull(gList));
}
RTT_TEST_END

RTT_TEST_START(cds_small_list_should_remove_items)
{
    TestItem* item = (TestItem*)CdsListFront(gList);
    while (item != NULL) {
        TestItem* next = (TestItem*)CdsListNext((const CdsListItem*)item);
        if ((item->x % 10) != 0) {
            CdsListRemove((CdsListItem*)item);
            testItemUnref((CdsListItem*)item);
        }
        item = next;
    }
    RTT_EXPECT(12 == gNumberOfItemsInExistence);
}
RTT_TEST_END

RTT_TEST_START(cds_small_list_size_should_be_12_after_removing_items)
{
    RTT_ASSERT(CdsListSize(gList) == 12);
    RTT_ASSERT(12 == gNumberOfItemsInExistence);
}
RTT_TEST_END

RTT_TEST_START(cds_small_list_should_be_as_expected_after_removing_items)
{
    int i = 40;
    CDSLIST_FOREACH(gList, TestItem, item) {
        RTT_EXPECT(item->x == i);
        i += 10;
    }
}
RTT_TEST_END

RTT_TEST_START(cds_should_destroy_small_list)
{
    CdsListDestroy(gList);
    RTT_EXPECT(0 == gNumberOfItemsInExistence);
}
RTT_TEST_END

RTT_GROUP_END(TestCdsListSmall,
        cds_should_create_small_list,
        cds_should_get_small_list_name,
        cds_small_list_size_should_be_0_after_creation,
        cds_small_list_capacity_should_be_20_after_creation,
        cds_small_list_should_be_empty_after_creation,
        cds_small_list_should_not_be_full_after_creation,
        cds_should_push_5_items_at_front_of_small_list,
        cds_small_list_size_should_be_5_when_partially_filled,
        cds_small_list_capacity_should_remain_20_when_partially_filled,
        cds_small_list_should_not_be_empty_when_partially_filled,
        cds_small_list_should_not_be_full_when_partially_filled,
        cds_should_push_15_items_at_back_of_small_list,
        cds_small_list_size_should_be_20_when_full,
        cds_small_list_capacity_should_remain_20_when_full,
        cds_small_list_should_not_be_empty_when_full,
        cds_small_list_should_be_full_when_full,
        cds_should_fail_to_push_at_the_front_of_small_list_when_full,
        cds_should_fail_to_push_at_the_back_of_small_list_when_full,
        cds_should_walk_through_small_list,
        cds_should_walk_backwards_through_small_list,
        cds_should_pop_5_items_from_front_of_small_list,
        cds_should_pop_5_items_from_back_of_small_list,
        cds_should_insert_item_before_front_of_small_list,
        cds_should_insert_item_after_front_of_small_list,
        cds_should_insert_item_before_back_of_small_list,
        cds_should_insert_item_after_back_of_small_list,
        cds_small_list_size_should_be_14_after_direct_inserts,
        cds_small_list_capacity_should_remain_20_after_direct_inserts,
        cds_small_list_should_not_be_empty_after_direct_inserts,
        cds_small_list_should_not_be_full_after_direct_inserts,
        cds_small_list_should_remove_items,
        cds_small_list_size_should_be_12_after_removing_items,
        cds_small_list_should_be_as_expected_after_removing_items,
        cds_should_destroy_small_list)
