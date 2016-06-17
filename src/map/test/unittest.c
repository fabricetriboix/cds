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
    gMap = CdsMapCreate("MyMap", 9, testKeyCompare, NULL,
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

RTT_TEST_START(cds_map_capacity_should_be_9_after_creation)
{
    RTT_ASSERT(CdsMapCapacity(gMap) == 9);
}
RTT_TEST_END

RTT_TEST_START(cds_map_insert_1st_item)
{
    TestItem* item = testItemAlloc(100);
    char* key = testKeyCreate(100);
    RTT_ASSERT(CdsMapInsert(gMap, key, (CdsMapItem*)item));
}
RTT_TEST_END

RTT_TEST_START(cds_map_size_should_be_1_after_inserting_1st_item)
{
    RTT_ASSERT(CdsMapSize(gMap) == 1);
}
RTT_TEST_END

RTT_TEST_START(cds_map_should_not_be_empty_after_inserting_1st_item)
{
    RTT_ASSERT(!CdsMapIsEmpty(gMap));
}
RTT_TEST_END

RTT_TEST_START(cds_check_map_shape_after_inserting_1st_item)
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

RTT_TEST_START(cds_map_should_find_1st_item)
{
    TestItem* item = (TestItem*)CdsMapSearch(gMap, "00000100");
    RTT_ASSERT(item != NULL);
    RTT_ASSERT(item->value == 100);
}
RTT_TEST_END

RTT_TEST_START(cds_map_should_find_2nd_item)
{
    TestItem* item = (TestItem*)CdsMapSearch(gMap, "00000200");
    RTT_ASSERT(item != NULL);
    RTT_ASSERT(item->value == 200);
}
RTT_TEST_END

RTT_TEST_START(cds_map_should_find_3rd_item)
{
    TestItem* item = (TestItem*)CdsMapSearch(gMap, "00000300");
    RTT_ASSERT(item != NULL);
    RTT_ASSERT(item->value == 300);
}
RTT_TEST_END

RTT_TEST_START(cds_map_insert_4th_item)
{
    TestItem* item = testItemAlloc(50);
    char* key = testKeyCreate(50);
    RTT_ASSERT(CdsMapInsert(gMap, key, (CdsMapItem*)item));
}
RTT_TEST_END

RTT_TEST_START(cds_check_map_shape_after_inserting_4th_item)
{
    TestItem* root = *((TestItem**)gMap);
    RTT_ASSERT(root != NULL);
    RTT_ASSERT(root->item.parent == NULL);
    char* key = (char*)(root->item.key);
    RTT_ASSERT(strcmp(key, "00000200") == 0);
    RTT_ASSERT(root->item.factor == -1);
    RTT_ASSERT(root->value == 200);

    TestItem* subroot = (TestItem*)(root->item.left);
    RTT_ASSERT(subroot != NULL);
    RTT_ASSERT(subroot->item.parent == (CdsMapItem*)root);
    RTT_ASSERT(subroot->item.left != NULL);
    RTT_ASSERT(subroot->item.right == NULL);
    key = (char*)(subroot->item.key);
    RTT_ASSERT(strcmp(key, "00000100") == 0);
    RTT_ASSERT(subroot->item.factor == -1);
    RTT_ASSERT(subroot->value == 100);

    TestItem* left = (TestItem*)(subroot->item.left);
    RTT_ASSERT(left != NULL);
    RTT_ASSERT(left->item.parent == (CdsMapItem*)subroot);
    RTT_ASSERT(left->item.left == NULL);
    RTT_ASSERT(left->item.right == NULL);
    key = (char*)(left->item.key);
    RTT_ASSERT(strcmp(key, "00000050") == 0);
    RTT_ASSERT(left->item.factor == 0);
    RTT_ASSERT(left->value = 50);
}
RTT_TEST_END

RTT_TEST_START(cds_map_insert_5th_item_and_perform_single_LL_rotation)
{
    TestItem* item = testItemAlloc(25);
    char* key = testKeyCreate(25);
    RTT_ASSERT(CdsMapInsert(gMap, key, (CdsMapItem*)item));
}
RTT_TEST_END

RTT_TEST_START(cds_map_size_should_be_5_after_inserting_5th_item)
{
    RTT_ASSERT(CdsMapSize(gMap) == 5);
}
RTT_TEST_END

RTT_TEST_START(cds_map_should_not_be_empty_after_inserting_5th_item)
{
    RTT_ASSERT(!CdsMapIsEmpty(gMap));
}
RTT_TEST_END

RTT_TEST_START(cds_map_should_not_be_full_after_inserting_5th_item)
{
    RTT_ASSERT(!CdsMapIsFull(gMap));
}
RTT_TEST_END

/* Tree at this stage

                200
           50         300
         25  100
*/
RTT_TEST_START(cds_check_map_shape_after_inserting_5th_item)
{
    TestItem* root = *((TestItem**)gMap);
    RTT_ASSERT(root != NULL);
    RTT_ASSERT(root->item.parent == NULL);
    char* key = (char*)(root->item.key);
    RTT_ASSERT(strcmp(key, "00000200") == 0);
    RTT_ASSERT(root->item.factor == -1);
    RTT_ASSERT(root->value == 200);

    TestItem* subroot = (TestItem*)(root->item.left);
    RTT_ASSERT(subroot != NULL);
    RTT_ASSERT(subroot->item.parent == (CdsMapItem*)root);
    RTT_ASSERT(subroot->item.left != NULL);
    RTT_ASSERT(subroot->item.right != NULL);
    key = (char*)(subroot->item.key);
    RTT_ASSERT(strcmp(key, "00000050") == 0);
    RTT_ASSERT(subroot->item.factor == 0);
    RTT_ASSERT(subroot->value == 50);

    TestItem* left = (TestItem*)(subroot->item.left);
    RTT_ASSERT(left != NULL);
    RTT_ASSERT(left->item.parent == (CdsMapItem*)subroot);
    RTT_ASSERT(left->item.left == NULL);
    RTT_ASSERT(left->item.right == NULL);
    key = (char*)(left->item.key);
    RTT_ASSERT(strcmp(key, "00000025") == 0);
    RTT_ASSERT(left->item.factor == 0);
    RTT_ASSERT(left->value = 25);

    TestItem* right = (TestItem*)(subroot->item.right);
    RTT_ASSERT(right != NULL);
    RTT_ASSERT(right->item.parent == (CdsMapItem*)subroot);
    RTT_ASSERT(right->item.left == NULL);
    RTT_ASSERT(right->item.right == NULL);
    key = (char*)(right->item.key);
    RTT_ASSERT(strcmp(key, "00000100") == 0);
    RTT_ASSERT(right->item.factor == 0);
    RTT_ASSERT(right->value == 100);
}
RTT_TEST_END

RTT_TEST_START(cds_map_insert_6th_item_and_perform_double_LR_rotation)
{
    TestItem* item = testItemAlloc(150);
    char* key = testKeyCreate(150);
    RTT_ASSERT(CdsMapInsert(gMap, key, (CdsMapItem*)item));
}
RTT_TEST_END

/* Tree at this stage

             100
         50       200
       25      150   300
*/
RTT_TEST_START(cds_check_map_shape_after_inserting_6th_item)
{
    TestItem* root = *((TestItem**)gMap);
    RTT_ASSERT(root != NULL);
    RTT_ASSERT(root->item.parent == NULL);
    char* key = (char*)(root->item.key);
    RTT_ASSERT(strcmp(key, "00000100") == 0);
    RTT_ASSERT(root->item.factor == 0);
    RTT_ASSERT(root->value == 100);

    TestItem* left = (TestItem*)(root->item.left);
    RTT_ASSERT(left != NULL);
    RTT_ASSERT(left->item.parent == (CdsMapItem*)root);
    RTT_ASSERT(left->item.right == NULL);
    key = (char*)(left->item.key);
    RTT_ASSERT(strcmp(key, "00000050") == 0);
    RTT_ASSERT(left->item.factor == -1);
    RTT_ASSERT(left->value == 50);

    TestItem* leftleft = (TestItem*)(left->item.left);
    RTT_ASSERT(leftleft != NULL);
    RTT_ASSERT(leftleft->item.parent == (CdsMapItem*)left);
    RTT_ASSERT(leftleft->item.left == NULL);
    RTT_ASSERT(leftleft->item.right == NULL);
    key = (char*)(leftleft->item.key);
    RTT_ASSERT(strcmp(key, "00000025") == 0);
    RTT_ASSERT(leftleft->item.factor == 0);
    RTT_ASSERT(leftleft->value == 25);

    TestItem* right = (TestItem*)(root->item.right);
    RTT_ASSERT(right != NULL);
    RTT_ASSERT(right->item.parent == (CdsMapItem*)root);
    key = (char*)(right->item.key);
    RTT_ASSERT(strcmp(key, "00000200") == 0);
    RTT_ASSERT(right->item.factor == 0);
    RTT_ASSERT(right->value == 200);

    TestItem* rightleft = (TestItem*)(right->item.left);
    RTT_ASSERT(rightleft != NULL);
    RTT_ASSERT(rightleft->item.parent == (CdsMapItem*)right);
    RTT_ASSERT(rightleft->item.left == NULL);
    RTT_ASSERT(rightleft->item.right == NULL);
    key = (char*)(rightleft->item.key);
    RTT_ASSERT(strcmp(key, "00000150") == 0);
    RTT_ASSERT(rightleft->item.factor == 0);
    RTT_ASSERT(rightleft->value == 150);

    TestItem* rightright = (TestItem*)(right->item.right);
    RTT_ASSERT(rightright != NULL);
    RTT_ASSERT(rightright->item.parent == (CdsMapItem*)right);
    RTT_ASSERT(rightright->item.left == NULL);
    RTT_ASSERT(rightright->item.right == NULL);
    key = (char*)(rightright->item.key);
    RTT_ASSERT(strcmp(key, "00000300") == 0);
    RTT_ASSERT(rightright->item.factor == 0);
    RTT_ASSERT(rightright->value == 300);
}
RTT_TEST_END

RTT_TEST_START(cds_map_replace_item)
{
    TestItem* item = testItemAlloc(151);
    char* key = testKeyCreate(150);
    RTT_ASSERT(CdsMapInsert(gMap, key, (CdsMapItem*)item));
}
RTT_TEST_END

RTT_TEST_START(cds_map_size_should_not_change_after_replacing_item)
{
    RTT_ASSERT(CdsMapSize(gMap) == 6);
}
RTT_TEST_END

RTT_TEST_START(cds_map_find_replaced_item)
{
    TestItem* item = (TestItem*)CdsMapSearch(gMap, "00000150");
    RTT_ASSERT(item != NULL);
    RTT_ASSERT(item->value == 151);
}
RTT_TEST_END

RTT_TEST_START(cds_map_insert_3_items_and_perform_double_RL_rotation)
{
    TestItem* item = testItemAlloc(250);
    char* key = testKeyCreate(250);
    RTT_ASSERT(CdsMapInsert(gMap, key, (CdsMapItem*)item));

    item = testItemAlloc(350);
    key = testKeyCreate(350);
    RTT_ASSERT(CdsMapInsert(gMap, key, (CdsMapItem*)item));

    /* Tree at this stage

                 100
             50           200
           25        151        300
                             250   350
    */

    item = testItemAlloc(275);
    key = testKeyCreate(275);
    RTT_ASSERT(CdsMapInsert(gMap, key, (CdsMapItem*)item));
}
RTT_TEST_END

RTT_TEST_START(cds_map_size_should_be_9_after_inserting_3_items)
{
    RTT_ASSERT(CdsMapSize(gMap) == 9);
}
RTT_TEST_END

RTT_TEST_START(cds_map_should_not_be_empty_after_inserting_3_items)
{
    RTT_ASSERT(!CdsMapIsEmpty(gMap));
}
RTT_TEST_END

RTT_TEST_START(cds_map_should_be_full_after_inserting_3_items)
{
    RTT_ASSERT(CdsMapIsFull(gMap));
}
RTT_TEST_END

/* Tree at this stage

          100
      50             250
    25          200        300
             151        275   350

*/
RTT_TEST_START(cds_check_map_shape_after_inserting_3_items)
{
    TestItem* root = *((TestItem**)gMap);
    RTT_ASSERT(root != NULL);
    RTT_ASSERT(root->item.parent == NULL);
    char* key = (char*)(root->item.key);
    RTT_ASSERT(strcmp(key, "00000100") == 0);
    RTT_ASSERT(root->item.factor == 1);
    RTT_ASSERT(root->value == 100);

    TestItem* subroot = (TestItem*)(root->item.right);
    RTT_ASSERT(subroot != NULL);
    RTT_ASSERT(subroot->item.parent == (CdsMapItem*)root);
    key = (char*)(subroot->item.key);
    RTT_ASSERT(strcmp(key, "00000250") == 0);
    RTT_ASSERT(subroot->item.factor == 0);
    RTT_ASSERT(subroot->value == 250);

    TestItem* left = (TestItem*)(subroot->item.left);
    RTT_ASSERT(left != NULL);
    RTT_ASSERT(left->item.parent == (CdsMapItem*)subroot);
    RTT_ASSERT(left->item.right == NULL);
    key = (char*)(left->item.key);
    RTT_ASSERT(strcmp(key, "00000200") == 0);
    RTT_ASSERT(left->item.factor == -1);
    RTT_ASSERT(left->value == 200);

    TestItem* leftleft = (TestItem*)(left->item.left);
    RTT_ASSERT(leftleft != NULL);
    RTT_ASSERT(leftleft->item.parent == (CdsMapItem*)left);
    RTT_ASSERT(leftleft->item.left == NULL);
    RTT_ASSERT(leftleft->item.right == NULL);
    key = (char*)(leftleft->item.key);
    RTT_ASSERT(strcmp(key, "00000150") == 0);
    RTT_ASSERT(leftleft->item.factor == 0);
    RTT_ASSERT(leftleft->value == 151);

    TestItem* right = (TestItem*)(subroot->item.right);
    RTT_ASSERT(right != NULL);
    RTT_ASSERT(right->item.parent == (CdsMapItem*)subroot);
    key = (char*)(right->item.key);
    RTT_ASSERT(strcmp(key, "00000300") == 0);
    RTT_ASSERT(right->item.factor == 0);
    RTT_ASSERT(right->value == 300);

    TestItem* rightleft = (TestItem*)(right->item.left);
    RTT_ASSERT(rightleft != NULL);
    RTT_ASSERT(rightleft->item.parent == (CdsMapItem*)right);
    RTT_ASSERT(rightleft->item.left == NULL);
    RTT_ASSERT(rightleft->item.right == NULL);
    key = (char*)(rightleft->item.key);
    RTT_ASSERT(strcmp(key, "00000275") == 0);
    RTT_ASSERT(rightleft->item.factor == 0);
    RTT_ASSERT(rightleft->value == 275);

    TestItem* rightright = (TestItem*)(right->item.right);
    RTT_ASSERT(rightright != NULL);
    RTT_ASSERT(rightright->item.parent == (CdsMapItem*)right);
    RTT_ASSERT(rightright->item.left == NULL);
    RTT_ASSERT(rightright->item.right == NULL);
    key = (char*)(rightright->item.key);
    RTT_ASSERT(strcmp(key, "00000350") == 0);
    RTT_ASSERT(rightright->item.factor == 0);
    RTT_ASSERT(rightright->value == 350);
}
RTT_TEST_END

RTT_TEST_START(cds_map_should_remove_leaf_item)
{
    RTT_ASSERT(CdsMapRemove(gMap, "00000275"));
}
RTT_TEST_END

RTT_TEST_START(cds_map_size_should_be_8_after_removing_leaf_item)
{
    RTT_ASSERT(CdsMapSize(gMap) == 8);
}
RTT_TEST_END

RTT_TEST_START(cds_map_should_not_be_full_after_removing_leaf_item)
{
    RTT_ASSERT(!CdsMapIsFull(gMap));
}
RTT_TEST_END

/* Tree at this stage

          100
      50           250
    25          200   300
             151         350

*/
RTT_TEST_START(cds_check_map_shape_after_removing_leaf_item)
{
    TestItem* root = *((TestItem**)gMap);
    RTT_ASSERT(root != NULL);
    RTT_ASSERT(root->item.parent == NULL);
    char* key = (char*)(root->item.key);
    RTT_ASSERT(strcmp(key, "00000100") == 0);
    RTT_ASSERT(root->item.factor == 1);
    RTT_ASSERT(root->value == 100);

    TestItem* subroot = (TestItem*)(root->item.right);
    RTT_ASSERT(subroot != NULL);
    RTT_ASSERT(subroot->item.parent == (CdsMapItem*)root);
    key = (char*)(subroot->item.key);
    RTT_ASSERT(strcmp(key, "00000250") == 0);
    RTT_ASSERT(subroot->item.factor == 0);
    RTT_ASSERT(subroot->value == 250);

    TestItem* left = (TestItem*)(subroot->item.left);
    RTT_ASSERT(left != NULL);
    RTT_ASSERT(left->item.parent == (CdsMapItem*)subroot);
    RTT_ASSERT(left->item.right == NULL);
    key = (char*)(left->item.key);
    RTT_ASSERT(strcmp(key, "00000200") == 0);
    RTT_ASSERT(left->item.factor == -1);
    RTT_ASSERT(left->value == 200);

    TestItem* leftleft = (TestItem*)(left->item.left);
    RTT_ASSERT(leftleft != NULL);
    RTT_ASSERT(leftleft->item.parent == (CdsMapItem*)left);
    RTT_ASSERT(leftleft->item.left == NULL);
    RTT_ASSERT(leftleft->item.right == NULL);
    key = (char*)(leftleft->item.key);
    RTT_ASSERT(strcmp(key, "00000150") == 0);
    RTT_ASSERT(leftleft->item.factor == 0);
    RTT_ASSERT(leftleft->value == 151);

    TestItem* right = (TestItem*)(subroot->item.right);
    RTT_ASSERT(right != NULL);
    RTT_ASSERT(right->item.parent == (CdsMapItem*)subroot);
    RTT_ASSERT(right->item.left == NULL);
    key = (char*)(right->item.key);
    RTT_ASSERT(strcmp(key, "00000300") == 0);
    RTT_ASSERT(right->item.factor == 1);
    RTT_ASSERT(right->value == 300);

    TestItem* rightright = (TestItem*)(right->item.right);
    RTT_ASSERT(rightright != NULL);
    RTT_ASSERT(rightright->item.parent == (CdsMapItem*)right);
    RTT_ASSERT(rightright->item.left == NULL);
    RTT_ASSERT(rightright->item.right == NULL);
    key = (char*)(rightright->item.key);
    RTT_ASSERT(strcmp(key, "00000350") == 0);
    RTT_ASSERT(rightright->item.factor == 0);
    RTT_ASSERT(rightright->value == 350);
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
        cds_map_capacity_should_be_9_after_creation,
        cds_map_insert_1st_item,
        cds_map_size_should_be_1_after_inserting_1st_item,
        cds_map_should_not_be_empty_after_inserting_1st_item,
        cds_check_map_shape_after_inserting_1st_item,
        cds_map_insert_2nd_item,
        cds_map_size_should_be_2_after_2nd_insert,
        cds_check_map_shape_after_inserting_2nd_item,
        cds_map_insert_3rd_item_and_perform_single_RR_rotation,
        cds_map_size_should_be_3_after_3rd_insert,
        cds_check_map_shape_after_inserting_3rd_item,
        cds_map_should_find_1st_item,
        cds_map_should_find_2nd_item,
        cds_map_should_find_3rd_item,
        cds_map_insert_4th_item,
        cds_check_map_shape_after_inserting_4th_item,
        cds_map_insert_5th_item_and_perform_single_LL_rotation,
        cds_map_size_should_be_5_after_inserting_5th_item,
        cds_map_should_not_be_empty_after_inserting_5th_item,
        cds_map_should_not_be_full_after_inserting_5th_item,
        cds_check_map_shape_after_inserting_5th_item,
        cds_map_insert_6th_item_and_perform_double_LR_rotation,
        cds_check_map_shape_after_inserting_6th_item,
        cds_map_replace_item,
        cds_map_size_should_not_change_after_replacing_item,
        cds_map_find_replaced_item,
        cds_map_insert_3_items_and_perform_double_RL_rotation,
        cds_map_size_should_be_9_after_inserting_3_items,
        cds_map_should_not_be_empty_after_inserting_3_items,
        cds_map_should_be_full_after_inserting_3_items,
        cds_check_map_shape_after_inserting_3_items,
        cds_map_should_remove_leaf_item,
        cds_map_size_should_be_8_after_removing_leaf_item,
        cds_map_should_not_be_full_after_removing_leaf_item,
        cds_check_map_shape_after_removing_leaf_item,
        cds_should_destroy_map);
