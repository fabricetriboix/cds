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
#include "rttest.h"
#include <string.h>


#define KEYSIZE 64


typedef struct {
    CdsMapItem item;
    int ref;
    int value;
} TestItem;

static int gNumberOfItemsInExistence = 0;

static void testItemUnref(CdsMapItem* titem)
{
    TestItem* item = (TestItem*)titem;
    item->ref--;
    if (item->ref <= 0) {
        free(item);
        gNumberOfItemsInExistence--;
    }
}

static TestItem* testItemAlloc(int value)
{
    TestItem* item = malloc(sizeof(*item));
    if (item == NULL) {
        abort();
    }
    memset(item, 0, sizeof(*item));
    item->ref = 1;
    item->value = value;
    gNumberOfItemsInExistence++;
    return item;
}


static int gNumberOfKeysInExistence = 0;

static void testKeyUnref(void* tkey)
{
    // The last character is the reference counter, on 8 bit.
    // The rest is the actual string.
    char* key = (char*)tkey;
    key[KEYSIZE-1]--;
    if (key[KEYSIZE-1] <= 0) {
        free(key);
        gNumberOfKeysInExistence--;
    }
}

static char* testKeyCreate(int value)
{
    char* key = malloc(KEYSIZE);
    if (key == NULL) {
        abort();
    }
    key[KEYSIZE-1] = 1;
    snprintf(key, KEYSIZE - 1, "%08d", value);
    gNumberOfKeysInExistence++;
    return key;
}

static int testKeyCompare(void* leftKey, void* rightKey, void* cookie)
{
    (void)cookie; // unused argument
    char* left = (char*)leftKey;
    char* right = (char*)rightKey;
    return strcmp(left, right);
}

CdsMap* gMap = NULL;


RTT_GROUP_START(TestCdsMap, 0x00050001u, NULL, NULL)

RTT_TEST_START(cds_should_create_map)
{
    gMap = CdsMapCreate("MyMap", 7, testKeyCompare, NULL,
            testKeyUnref, testItemUnref);
    RTT_ASSERT(gMap != NULL);
}
RTT_TEST_END

RTT_TEST_START(cds_should_get_map_name)
{
    const char* name = CdsMapName(gMap);
    RTT_EXPECT(strcmp(name, "MyMap") == 0);
}
RTT_TEST_END

RTT_TEST_START(cds_map_size_should_be_0_after_creation)
{
    RTT_ASSERT(CdsMapSize(gMap) == 0);
    RTT_ASSERT(gNumberOfItemsInExistence == 0);
    RTT_ASSERT(gNumberOfKeysInExistence == 0);
}
RTT_TEST_END

RTT_TEST_START(cds_map_should_be_empty_after_creation)
{
    RTT_ASSERT(CdsMapIsEmpty(gMap));
}
RTT_TEST_END

RTT_TEST_START(cds_map_should_not_be_full_after_creation)
{
    RTT_ASSERT(!CdsMapIsFull(gMap));
}
RTT_TEST_END

RTT_TEST_START(cds_map_capacity_should_be_7_after_creation)
{
    RTT_ASSERT(CdsMapCapacity(gMap) == 7);
}
RTT_TEST_END

RTT_TEST_START(cds_map_insert_first_item)
{
    TestItem* item = testItemAlloc(100);
    char* key = testKeyCreate(100);
    RTT_ASSERT(CdsMapInsert(gMap, key, (CdsMapItem*)item));
}
RTT_TEST_END

RTT_TEST_START(cds_map_size_should_be_1_after_inserting_first_item)
{
    RTT_ASSERT(CdsMapSize(gMap) == 1);
}
RTT_TEST_END

RTT_TEST_START(cds_map_should_not_be_empty_after_inserting_first_item)
{
    RTT_ASSERT(!CdsMapIsEmpty(gMap));
}
RTT_TEST_END

RTT_TEST_START(cds_check_map_shape_after_inserting_first_item)
{
    // NB: The first field in the `CdsMap` structure is the pointer to the root
    // This is ugly, but necessary...
    TestItem* root = *((TestItem**)gMap);
    RTT_ASSERT(root != NULL);
    RTT_ASSERT(root->item.parent == NULL);
    RTT_ASSERT(root->item.left == NULL);
    RTT_ASSERT(root->item.right == NULL);
    char* key = (char*)(root->item.key);
    RTT_ASSERT(strcmp(key, "00000100") == 0);
    RTT_ASSERT(root->item.factor == 0);
    RTT_ASSERT(root->value == 100);
}
RTT_TEST_END

RTT_TEST_START(cds_map_insert_2nd_item)
{
    TestItem* item = testItemAlloc(200);
    char* key = testKeyCreate(200);
    RTT_ASSERT(CdsMapInsert(gMap, key, (CdsMapItem*)item));
}
RTT_TEST_END

RTT_TEST_START(cds_map_size_should_be_2_after_2nd_insert)
{
    RTT_ASSERT(CdsMapSize(gMap) == 2);
}
RTT_TEST_END

RTT_TEST_START(cds_check_map_shape_after_inserting_2nd_item)
{
    TestItem* root = *((TestItem**)gMap);
    RTT_ASSERT(root != NULL);
    RTT_ASSERT(root->item.parent == NULL);
    RTT_ASSERT(root->item.left == NULL);
    char* key = (char*)(root->item.key);
    RTT_ASSERT(strcmp(key, "00000100") == 0);
    RTT_ASSERT(root->item.factor == 1);
    RTT_ASSERT(root->value == 100);

    TestItem* right = (TestItem*)(root->item.right);
    RTT_ASSERT(right != NULL);
    RTT_ASSERT(right->item.parent == (CdsMapItem*)root);
    RTT_ASSERT(right->item.left == NULL);
    RTT_ASSERT(right->item.right == NULL);
    key = (char*)(right->item.key);
    RTT_ASSERT(strcmp(key, "00000200") == 0);
    RTT_ASSERT(right->item.factor == 0);
    RTT_ASSERT(right->value == 200);
}
RTT_TEST_END

RTT_TEST_START(cds_map_insert_3rd_item_and_perform_single_RR_rotation)
{
    TestItem* item = testItemAlloc(300);
    char* key = testKeyCreate(300);
    RTT_ASSERT(CdsMapInsert(gMap, key, (CdsMapItem*)item));
}
RTT_TEST_END

RTT_TEST_START(cds_map_size_should_be_3_after_3rd_insert)
{
    RTT_ASSERT(CdsMapSize(gMap) == 3);
}
RTT_TEST_END

RTT_TEST_START(cds_check_map_shape_after_inserting_3rd_item)
{
    TestItem* root = *((TestItem**)gMap);
    RTT_ASSERT(root != NULL);
    RTT_ASSERT(root->item.parent == NULL);
    char* key = (char*)(root->item.key);
    RTT_ASSERT(strcmp(key, "00000200") == 0);
    RTT_ASSERT(root->item.factor == 0);
    RTT_ASSERT(root->value == 200);

    TestItem* left = (TestItem*)(root->item.left);
    RTT_ASSERT(left != NULL);
    RTT_ASSERT(left->item.parent == (CdsMapItem*)root);
    RTT_ASSERT(left->item.left == NULL);
    RTT_ASSERT(left->item.right == NULL);
    key = (char*)(left->item.key);
    RTT_ASSERT(strcmp(key, "00000100") == 0);
    RTT_ASSERT(left->item.factor == 0);
    RTT_ASSERT(left->value == 100);

    TestItem* right = (TestItem*)(root->item.right);
    RTT_ASSERT(right != NULL);
    RTT_ASSERT(right->item.parent == (CdsMapItem*)root);
    RTT_ASSERT(right->item.left == NULL);
    RTT_ASSERT(right->item.right == NULL);
    key = (char*)(right->item.key);
    RTT_ASSERT(strcmp(key, "00000300") == 0);
    RTT_ASSERT(right->item.factor == 0);
    RTT_ASSERT(right->value == 300);
}
RTT_TEST_END

RTT_TEST_START(cds_should_destroy_map)
{
    CdsMapDestroy(gMap);
    RTT_ASSERT(gNumberOfItemsInExistence == 0);
    RTT_ASSERT(gNumberOfKeysInExistence == 0);
}
RTT_TEST_END

RTT_GROUP_END(TestCdsMap,
        cds_should_create_map,
        cds_should_get_map_name,
        cds_map_size_should_be_0_after_creation,
        cds_map_should_be_empty_after_creation,
        cds_map_should_not_be_full_after_creation,
        cds_map_capacity_should_be_7_after_creation,
        cds_map_insert_first_item,
        cds_map_size_should_be_1_after_inserting_first_item,
        cds_map_should_not_be_empty_after_inserting_first_item,
        cds_check_map_shape_after_inserting_first_item,
        cds_map_insert_2nd_item,
        cds_map_size_should_be_2_after_2nd_insert,
        cds_check_map_shape_after_inserting_2nd_item,
        cds_map_insert_3rd_item_and_perform_single_RR_rotation,
        cds_map_size_should_be_3_after_3rd_insert,
        cds_check_map_shape_after_inserting_3rd_item,
        cds_should_destroy_map);
