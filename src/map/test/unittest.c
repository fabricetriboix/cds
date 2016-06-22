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

RTT_TEST_START(cds_map_should_remove_item_with_RR_rotation)
{
    RTT_ASSERT(CdsMapRemove(gMap, "00000050"));
}
RTT_TEST_END

/* Tree at this stage

                250
      100           300
    25      200        350
         151
*/
RTT_TEST_START(cds_check_map_shape_after_removing_item_with_RR_rotation)
{
    TestItem* root = *((TestItem**)gMap);
    RTT_ASSERT(root != NULL);
    RTT_ASSERT(root->item.parent == NULL);
    char* key = (char*)(root->item.key);
    RTT_ASSERT(strcmp(key, "00000250") == 0);
    RTT_ASSERT(root->item.factor == -1);
    RTT_ASSERT(root->value == 250);

    TestItem* l = (TestItem*)(root->item.left);
    RTT_ASSERT(l != NULL);
    RTT_ASSERT(l->item.parent == (CdsMapItem*)root);
    key = (char*)(l->item.key);
    RTT_ASSERT(strcmp(key, "00000100") == 0);
    RTT_ASSERT(l->item.factor == 1);
    RTT_ASSERT(l->value == 100);

    TestItem* ll = (TestItem*)(l->item.left);
    RTT_ASSERT(ll != NULL);
    RTT_ASSERT(ll->item.parent == (CdsMapItem*)l);
    RTT_ASSERT(ll->item.left == NULL);
    RTT_ASSERT(ll->item.right == NULL);
    key = (char*)(ll->item.key);
    RTT_ASSERT(strcmp(key, "00000025") == 0);
    RTT_ASSERT(ll->item.factor == 0);
    RTT_ASSERT(ll->value == 25);

    TestItem* lr = (TestItem*)(l->item.right);
    RTT_ASSERT(lr != NULL);
    RTT_ASSERT(lr->item.parent == (CdsMapItem*)l);
    RTT_ASSERT(lr->item.right == NULL);
    key = (char*)(lr->item.key);
    RTT_ASSERT(strcmp(key, "00000200") == 0);
    RTT_ASSERT(lr->item.factor == -1);
    RTT_ASSERT(lr->value == 200);

    TestItem* lrl = (TestItem*)(lr->item.left);
    RTT_ASSERT(lrl != NULL);
    RTT_ASSERT(lrl->item.parent == (CdsMapItem*)lr);
    RTT_ASSERT(lrl->item.left == NULL);
    RTT_ASSERT(lrl->item.right == NULL);
    key = (char*)(lrl->item.key);
    RTT_ASSERT(strcmp(key, "00000150") == 0);
    RTT_ASSERT(lrl->item.factor == 0);
    RTT_ASSERT(lrl->value == 151);

    TestItem* r = (TestItem*)(root->item.right);
    RTT_ASSERT(r != NULL);
    RTT_ASSERT(r->item.parent == (CdsMapItem*)root);
    RTT_ASSERT(r->item.left == NULL);
    key = (char*)(r->item.key);
    RTT_ASSERT(strcmp(key, "00000300") == 0);
    RTT_ASSERT(r->item.factor == 1);
    RTT_ASSERT(r->value == 300);

    TestItem* rr = (TestItem*)(r->item.right);
    RTT_ASSERT(rr != NULL);
    RTT_ASSERT(rr->item.parent == (CdsMapItem*)r);
    RTT_ASSERT(rr->item.left == NULL);
    RTT_ASSERT(rr->item.right == NULL);
    key = (char*)(rr->item.key);
    RTT_ASSERT(strcmp(key, "00000350") == 0);
    RTT_ASSERT(rr->item.factor == 0);
    RTT_ASSERT(rr->value == 350);
}
RTT_TEST_END

RTT_TEST_START(cds_map_should_remove_item_with_LR_rotation)
{
    RTT_ASSERT(CdsMapRemove(gMap, "00000350"));
}
RTT_TEST_END

/* Tree at this stage

             200
       100       250
     25   151       300
*/
RTT_TEST_START(cds_check_map_shape_after_removing_item_with_LR_rotation)
{
    TestItem* root = *((TestItem**)gMap);
    RTT_ASSERT(root != NULL);
    RTT_ASSERT(root->item.parent == NULL);
    char* key = (char*)(root->item.key);
    RTT_ASSERT(strcmp(key, "00000200") == 0);
    RTT_ASSERT(root->item.factor == 0);
    RTT_ASSERT(root->value == 200);

    TestItem* l = (TestItem*)(root->item.left);
    RTT_ASSERT(l != NULL);
    RTT_ASSERT(l->item.parent == (CdsMapItem*)root);
    key = (char*)(l->item.key);
    RTT_ASSERT(strcmp(key, "00000100") == 0);
    RTT_ASSERT(l->item.factor == 0);
    RTT_ASSERT(l->value == 100);

    TestItem* ll = (TestItem*)(l->item.left);
    RTT_ASSERT(ll != NULL);
    RTT_ASSERT(ll->item.parent == (CdsMapItem*)l);
    RTT_ASSERT(ll->item.left == NULL);
    RTT_ASSERT(ll->item.right == NULL);
    key = (char*)(ll->item.key);
    RTT_ASSERT(strcmp(key, "00000025") == 0);
    RTT_ASSERT(ll->item.factor == 0);
    RTT_ASSERT(ll->value == 25);

    TestItem* lr = (TestItem*)(l->item.right);
    RTT_ASSERT(lr != NULL);
    RTT_ASSERT(lr->item.parent == (CdsMapItem*)l);
    RTT_ASSERT(lr->item.left == NULL);
    RTT_ASSERT(lr->item.right == NULL);
    key = (char*)(lr->item.key);
    RTT_ASSERT(strcmp(key, "00000150") == 0);
    RTT_ASSERT(lr->item.factor == 0);
    RTT_ASSERT(lr->value == 151);

    TestItem* r = (TestItem*)(root->item.right);
    RTT_ASSERT(r != NULL);
    RTT_ASSERT(r->item.parent == (CdsMapItem*)root);
    RTT_ASSERT(r->item.left == NULL);
    key = (char*)(r->item.key);
    RTT_ASSERT(strcmp(key, "00000250") == 0);
    RTT_ASSERT(r->item.factor == 1);
    RTT_ASSERT(r->value == 250);

    TestItem* rr = (TestItem*)(r->item.right);
    RTT_ASSERT(rr != NULL);
    RTT_ASSERT(rr->item.parent == (CdsMapItem*)r);
    RTT_ASSERT(rr->item.left == NULL);
    RTT_ASSERT(rr->item.right == NULL);
    key = (char*)(rr->item.key);
    RTT_ASSERT(strcmp(key, "00000300") == 0);
    RTT_ASSERT(rr->item.factor == 0);
    RTT_ASSERT(rr->value == 300);
}
RTT_TEST_END

RTT_TEST_START(cds_map_size_should_be_6)
{
    RTT_ASSERT(CdsMapSize(gMap) == 6);
}
RTT_TEST_END

RTT_TEST_START(cds_reshape_map_before_removal_with_LL_rotation)
{
    TestItem* item = testItemAlloc(12);
    char* key = testKeyCreate(12);
    RTT_ASSERT(CdsMapInsert(gMap, key, (CdsMapItem*)item));
}
RTT_TEST_END

RTT_TEST_START(cds_map_should_remove_item_with_LL_rotation)
{
    RTT_ASSERT(CdsMapRemove(gMap, "00000250"));
}
RTT_TEST_END

/* Tree at this stage

          100
       25       200
     12      151   300
*/
RTT_TEST_START(cds_check_map_shape_after_removing_item_with_LL_rotation)
{
    TestItem* root = *((TestItem**)gMap);
    RTT_ASSERT(root != NULL);
    RTT_ASSERT(root->item.parent == NULL);
    char* key = (char*)(root->item.key);
    RTT_ASSERT(strcmp(key, "00000100") == 0);
    RTT_ASSERT(root->item.factor == 0);
    RTT_ASSERT(root->value == 100);

    TestItem* l = (TestItem*)(root->item.left);
    RTT_ASSERT(l != NULL);
    RTT_ASSERT(l->item.right == NULL);
    RTT_ASSERT(l->item.parent == (CdsMapItem*)root);
    key = (char*)(l->item.key);
    RTT_ASSERT(strcmp(key, "00000025") == 0);
    RTT_ASSERT(l->item.factor == -1);
    RTT_ASSERT(l->value == 25);

    TestItem* ll = (TestItem*)(l->item.left);
    RTT_ASSERT(ll != NULL);
    RTT_ASSERT(ll->item.parent == (CdsMapItem*)l);
    RTT_ASSERT(ll->item.left == NULL);
    RTT_ASSERT(ll->item.right == NULL);
    key = (char*)(ll->item.key);
    RTT_ASSERT(strcmp(key, "00000012") == 0);
    RTT_ASSERT(ll->item.factor == 0);
    RTT_ASSERT(ll->value == 12);

    TestItem* r = (TestItem*)(root->item.right);
    RTT_ASSERT(r != NULL);
    RTT_ASSERT(r->item.parent == (CdsMapItem*)root);
    key = (char*)(r->item.key);
    RTT_ASSERT(strcmp(key, "00000200") == 0);
    RTT_ASSERT(r->item.factor == 0);
    RTT_ASSERT(r->value == 200);

    TestItem* rl = (TestItem*)(r->item.left);
    RTT_ASSERT(rl != NULL);
    RTT_ASSERT(rl->item.parent == (CdsMapItem*)r);
    RTT_ASSERT(rl->item.left == NULL);
    RTT_ASSERT(rl->item.right == NULL);
    key = (char*)(rl->item.key);
    RTT_ASSERT(strcmp(key, "00000150") == 0);
    RTT_ASSERT(rl->item.factor == 0);
    RTT_ASSERT(rl->value == 151);

    TestItem* rr = (TestItem*)(r->item.right);
    RTT_ASSERT(rr != NULL);
    RTT_ASSERT(rr->item.parent == (CdsMapItem*)r);
    RTT_ASSERT(rr->item.left == NULL);
    RTT_ASSERT(rr->item.right == NULL);
    key = (char*)(rr->item.key);
    RTT_ASSERT(strcmp(key, "00000300") == 0);
    RTT_ASSERT(rr->item.factor == 0);
    RTT_ASSERT(rr->value == 300);
}
RTT_TEST_END

RTT_TEST_START(cds_reshape_map_before_removal_with_RL_rotation)
{
    TestItem* item = testItemAlloc(125);
    char* key = testKeyCreate(125);
    RTT_ASSERT(CdsMapInsert(gMap, key, (CdsMapItem*)item));
}
RTT_TEST_END

/* Tree at this stage

          100
       25          200
     12         151   300
             125
*/
RTT_TEST_START(cds_map_should_remove_item_with_RL_rotation)
{
    RTT_ASSERT(CdsMapRemove(gMap, "00000012"));
}
RTT_TEST_END

/* Tree at this stage

           151
     100       200
   25   125       300
*/
RTT_TEST_START(cds_check_map_shape_after_removing_item_with_RL_rotation)
{
    TestItem* root = *((TestItem**)gMap);
    RTT_ASSERT(root != NULL);
    RTT_ASSERT(root->item.parent == NULL);
    char* key = (char*)(root->item.key);
    RTT_ASSERT(strcmp(key, "00000150") == 0);
    RTT_ASSERT(root->item.factor == 0);
    RTT_ASSERT(root->value == 151);

    TestItem* l = (TestItem*)(root->item.left);
    RTT_ASSERT(l != NULL);
    RTT_ASSERT(l->item.parent == (CdsMapItem*)root);
    key = (char*)(l->item.key);
    RTT_ASSERT(strcmp(key, "00000100") == 0);
    RTT_ASSERT(l->item.factor == 0);
    RTT_ASSERT(l->value == 100);

    TestItem* ll = (TestItem*)(l->item.left);
    RTT_ASSERT(ll != NULL);
    RTT_ASSERT(ll->item.parent == (CdsMapItem*)l);
    RTT_ASSERT(ll->item.left == NULL);
    RTT_ASSERT(ll->item.right == NULL);
    key = (char*)(ll->item.key);
    RTT_ASSERT(strcmp(key, "00000025") == 0);
    RTT_ASSERT(ll->item.factor == 0);
    RTT_ASSERT(ll->value == 25);

    TestItem* lr = (TestItem*)(l->item.right);
    RTT_ASSERT(lr != NULL);
    RTT_ASSERT(lr->item.parent == (CdsMapItem*)l);
    RTT_ASSERT(lr->item.left == NULL);
    RTT_ASSERT(lr->item.right == NULL);
    key = (char*)(lr->item.key);
    RTT_ASSERT(strcmp(key, "00000125") == 0);
    RTT_ASSERT(lr->item.factor == 0);
    RTT_ASSERT(lr->value = 125);

    TestItem* r = (TestItem*)(root->item.right);
    RTT_ASSERT(r != NULL);
    RTT_ASSERT(r->item.parent == (CdsMapItem*)root);
    key = (char*)(r->item.key);
    RTT_ASSERT(strcmp(key, "00000200") == 0);
    RTT_ASSERT(r->item.factor == 1);
    RTT_ASSERT(r->value == 200);

    TestItem* rr = (TestItem*)(r->item.right);
    RTT_ASSERT(rr != NULL);
    RTT_ASSERT(rr->item.parent == (CdsMapItem*)r);
    RTT_ASSERT(rr->item.left == NULL);
    RTT_ASSERT(rr->item.right == NULL);
    key = (char*)(rr->item.key);
    RTT_ASSERT(strcmp(key, "00000300") == 0);
    RTT_ASSERT(rr->item.factor == 0);
    RTT_ASSERT(rr->value == 300);
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
        cds_map_should_remove_item_with_RR_rotation,
        cds_check_map_shape_after_removing_item_with_RR_rotation,
        cds_map_should_remove_item_with_LR_rotation,
        cds_check_map_shape_after_removing_item_with_LR_rotation,
        cds_map_size_should_be_6,
        cds_reshape_map_before_removal_with_LL_rotation,
        cds_map_should_remove_item_with_LL_rotation,
        cds_check_map_shape_after_removing_item_with_LL_rotation,
        cds_reshape_map_before_removal_with_RL_rotation,
        cds_map_should_remove_item_with_RL_rotation,
        cds_check_map_shape_after_removing_item_with_RL_rotation,
        cds_should_destroy_map);


// Test insertions and deletions with more than one level of retracing
RTT_GROUP_START(TestCdsDeepMap, 0x00050002u, NULL, NULL)

RTT_TEST_START(cds_should_create_deep_map)
{
    RTT_ASSERT(gNumberOfItemsInExistence == 0);
    RTT_ASSERT(gNumberOfKeysInExistence == 0);
    gMap = CdsMapCreate(NULL, 0, testKeyCompare, NULL,
            testKeyUnref, testItemUnref);
    RTT_ASSERT(gMap != NULL);
}
RTT_TEST_END

// Fill 3 levels, and some of 4th level
RTT_TEST_START(cds_should_build_initial_deep_map)
{
    TestItem* item = testItemAlloc(100);
    char* key = testKeyCreate(100);
    RTT_ASSERT(CdsMapInsert(gMap, key, (CdsMapItem*)item));

    item = testItemAlloc(50);
    key = testKeyCreate(50);
    RTT_ASSERT(CdsMapInsert(gMap, key, (CdsMapItem*)item));

    item = testItemAlloc(150);
    key = testKeyCreate(150);
    RTT_ASSERT(CdsMapInsert(gMap, key, (CdsMapItem*)item));

    item = testItemAlloc(25);
    key = testKeyCreate(25);
    RTT_ASSERT(CdsMapInsert(gMap, key, (CdsMapItem*)item));

    item = testItemAlloc(75);
    key = testKeyCreate(75);
    RTT_ASSERT(CdsMapInsert(gMap, key, (CdsMapItem*)item));

    item = testItemAlloc(125);
    key = testKeyCreate(125);
    RTT_ASSERT(CdsMapInsert(gMap, key, (CdsMapItem*)item));

    item = testItemAlloc(200);
    key = testKeyCreate(200);
    RTT_ASSERT(CdsMapInsert(gMap, key, (CdsMapItem*)item));

    item = testItemAlloc(12);
    key = testKeyCreate(12);
    RTT_ASSERT(CdsMapInsert(gMap, key, (CdsMapItem*)item));

    item = testItemAlloc(33);
    key = testKeyCreate(33);
    RTT_ASSERT(CdsMapInsert(gMap, key, (CdsMapItem*)item));

    item = testItemAlloc(82);
    key = testKeyCreate(82);
    RTT_ASSERT(CdsMapInsert(gMap, key, (CdsMapItem*)item));
}
RTT_TEST_END

/* Tree at this stage

               100
         50            150
     25     75      125   200
   12  33     82
*/
RTT_TEST_START(cds_check_deep_map_inital_build)
{
    TestItem* root = *((TestItem**)gMap);
    RTT_ASSERT(root != NULL);
    RTT_ASSERT(root->item.parent == NULL);
    char* key = (char*)(root->item.key);
    RTT_ASSERT(strcmp(key, "00000100") == 0);
    RTT_ASSERT(root->item.factor == -1);
    RTT_ASSERT(root->value == 100);

    TestItem* l = (TestItem*)(root->item.left);
    RTT_ASSERT(l != NULL);
    RTT_ASSERT(l->item.parent == (CdsMapItem*)root);
    key = (char*)(l->item.key);
    RTT_ASSERT(strcmp(key, "00000050") == 0);
    RTT_ASSERT(l->item.factor == 0);
    RTT_ASSERT(l->value == 50);

    TestItem* ll = (TestItem*)(l->item.left);
    RTT_ASSERT(ll != NULL);
    RTT_ASSERT(ll->item.parent == (CdsMapItem*)l);
    key = (char*)(ll->item.key);
    RTT_ASSERT(strcmp(key, "00000025") == 0);
    RTT_ASSERT(ll->item.factor == 0);
    RTT_ASSERT(ll->value == 25);

    TestItem* lll = (TestItem*)(ll->item.left);
    RTT_ASSERT(lll != NULL);
    RTT_ASSERT(lll->item.parent == (CdsMapItem*)ll);
    RTT_ASSERT(lll->item.left == NULL);
    RTT_ASSERT(lll->item.right == NULL);
    key = (char*)(lll->item.key);
    RTT_ASSERT(strcmp(key, "00000012") == 0);
    RTT_ASSERT(lll->item.factor == 0);
    RTT_ASSERT(lll->value == 12);

    TestItem* llr = (TestItem*)(ll->item.right);
    RTT_ASSERT(llr != NULL);
    RTT_ASSERT(llr->item.parent == (CdsMapItem*)ll);
    RTT_ASSERT(llr->item.left == NULL);
    RTT_ASSERT(llr->item.right == NULL);
    key = (char*)(llr->item.key);
    RTT_ASSERT(strcmp(key, "00000033") == 0);

    TestItem* lr = (TestItem*)(l->item.right);
    RTT_ASSERT(lr != NULL);
    RTT_ASSERT(lr->item.parent == (CdsMapItem*)l);
    RTT_ASSERT(lr->item.left == NULL);
    key = (char*)(lr->item.key);
    RTT_ASSERT(strcmp(key, "00000075") == 0);
    RTT_ASSERT(lr->item.factor == 1);
    RTT_ASSERT(lr->value = 75);

    TestItem* lrr = (TestItem*)(lr->item.right);
    RTT_ASSERT(lrr != NULL);
    RTT_ASSERT(lrr->item.parent == (CdsMapItem*)lr);
    RTT_ASSERT(lrr->item.left == NULL);
    RTT_ASSERT(lrr->item.right == NULL);
    key = (char*)(lrr->item.key);
    RTT_ASSERT(strcmp(key, "00000082") == 0);
    RTT_ASSERT(lrr->item.factor == 0);
    RTT_ASSERT(lrr->value == 82);

    TestItem* r = (TestItem*)(root->item.right);
    RTT_ASSERT(r != NULL);
    RTT_ASSERT(r->item.parent == (CdsMapItem*)root);
    key = (char*)(r->item.key);
    RTT_ASSERT(strcmp(key, "00000150") == 0);
    RTT_ASSERT(r->item.factor == 0);
    RTT_ASSERT(r->value == 150);

    TestItem* rl = (TestItem*)(r->item.left);
    RTT_ASSERT(rl != NULL);
    RTT_ASSERT(rl->item.parent = (CdsMapItem*)r);
    RTT_ASSERT(rl->item.left == NULL);
    RTT_ASSERT(rl->item.right == NULL);
    key = (char*)(rl->item.key);
    RTT_ASSERT(strcmp(key, "00000125") == 0);
    RTT_ASSERT(rl->item.factor == 0);
    RTT_ASSERT(rl->value == 125);

    TestItem* rr = (TestItem*)(r->item.right);
    RTT_ASSERT(rr != NULL);
    RTT_ASSERT(rr->item.parent == (CdsMapItem*)r);
    key = (char*)(rr->item.key);
    RTT_ASSERT(strcmp(key, "00000200") == 0);
    RTT_ASSERT(rr->item.factor == 0);
    RTT_ASSERT(rr->value == 200);
}
RTT_TEST_END

RTT_TEST_START(cds_should_perform_LL_rotation_when_inserting_into_deep_map)
{
    TestItem* item = testItemAlloc(10);
    char* key = testKeyCreate(10);
    RTT_ASSERT(CdsMapInsert(gMap, key, (CdsMapItem*)item));
}
RTT_TEST_END

/* Tree at this stage

            50
       25         100
     12  33   75        150
   10           82   125   200
*/
RTT_TEST_START(cds_check_deep_map_after_insertion_with_LL_rotation)
{
    TestItem* root = *((TestItem**)gMap);
    RTT_ASSERT(root != NULL);
    RTT_ASSERT(root->item.parent == NULL);
    char* key = (char*)(root->item.key);
    RTT_ASSERT(strcmp(key, "00000050") == 0);
    RTT_ASSERT(root->item.factor == 0);
    RTT_ASSERT(root->value == 50);

    TestItem* l = (TestItem*)(root->item.left);
    RTT_ASSERT(l != NULL);
    RTT_ASSERT(l->item.parent == (CdsMapItem*)root);
    key = (char*)(l->item.key);
    RTT_ASSERT(strcmp(key, "00000025") == 0);
    RTT_ASSERT(l->item.factor == -1);
    RTT_ASSERT(l->value == 25);

    TestItem* ll = (TestItem*)(l->item.left);
    RTT_ASSERT(ll != NULL);
    RTT_ASSERT(ll->item.parent == (CdsMapItem*)l);
    RTT_ASSERT(ll->item.right == NULL);
    key = (char*)(ll->item.key);
    RTT_ASSERT(strcmp(key, "00000012") == 0);
    RTT_ASSERT(ll->item.factor == -1);
    RTT_ASSERT(ll->value == 12);

    TestItem* lll = (TestItem*)(ll->item.left);
    RTT_ASSERT(lll != NULL);
    RTT_ASSERT(lll->item.parent == (CdsMapItem*)ll);
    RTT_ASSERT(lll->item.left == NULL);
    RTT_ASSERT(lll->item.right == NULL);
    key = (char*)(lll->item.key);
    RTT_ASSERT(strcmp(key, "00000010") == 0);
    RTT_ASSERT(lll->item.factor == 0);
    RTT_ASSERT(lll->value == 10);

    TestItem* lr = (TestItem*)(l->item.right);
    RTT_ASSERT(lr != NULL);
    RTT_ASSERT(lr->item.parent == (CdsMapItem*)l);
    RTT_ASSERT(lr->item.left == NULL);
    RTT_ASSERT(lr->item.right == NULL);
    key = (char*)(lr->item.key);
    RTT_ASSERT(strcmp(key, "00000033") == 0);
    RTT_ASSERT(lr->item.factor == 0);
    RTT_ASSERT(lr->value = 33);

    TestItem* r = (TestItem*)(root->item.right);
    RTT_ASSERT(r != NULL);
    RTT_ASSERT(r->item.parent == (CdsMapItem*)root);
    key = (char*)(r->item.key);
    RTT_ASSERT(strcmp(key, "00000100") == 0);
    RTT_ASSERT(r->item.factor == 0);
    RTT_ASSERT(r->value == 100);

    TestItem* rl = (TestItem*)(r->item.left);
    RTT_ASSERT(rl != NULL);
    RTT_ASSERT(rl->item.parent = (CdsMapItem*)r);
    RTT_ASSERT(rl->item.left == NULL);
    key = (char*)(rl->item.key);
    RTT_ASSERT(strcmp(key, "00000075") == 0);
    RTT_ASSERT(rl->item.factor == 1);
    RTT_ASSERT(rl->value == 75);

    TestItem* rlr = (TestItem*)(rl->item.right);
    RTT_ASSERT(rlr != NULL);
    RTT_ASSERT(rlr->item.parent = (CdsMapItem*)rl);
    RTT_ASSERT(rlr->item.left == NULL);
    RTT_ASSERT(rlr->item.right == NULL);
    key = (char*)(rlr->item.key);
    RTT_ASSERT(strcmp(key, "00000082") == 0);
    RTT_ASSERT(rlr->item.factor == 0);
    RTT_ASSERT(rlr->value == 82);

    TestItem* rr = (TestItem*)(r->item.right);
    RTT_ASSERT(rr != NULL);
    RTT_ASSERT(rr->item.parent == (CdsMapItem*)r);
    key = (char*)(rr->item.key);
    RTT_ASSERT(strcmp(key, "00000150") == 0);
    RTT_ASSERT(rr->item.factor == 0);
    RTT_ASSERT(rr->value == 150);

    TestItem* rrl = (TestItem*)(rr->item.left);
    RTT_ASSERT(rrl != NULL);
    RTT_ASSERT(rrl->item.parent == (CdsMapItem*)rr);
    RTT_ASSERT(rrl->item.left == NULL);
    RTT_ASSERT(rrl->item.right == NULL);
    key = (char*)(rrl->item.key);
    RTT_ASSERT(strcmp(key, "00000125") == 0);
    RTT_ASSERT(rrl->item.factor == 0);
    RTT_ASSERT(rrl->value == 125);

    TestItem* rrr = (TestItem*)(rr->item.right);
    RTT_ASSERT(rrr != NULL);
    RTT_ASSERT(rrr->item.parent == (CdsMapItem*)rr);
    RTT_ASSERT(rrr->item.left == NULL);
    RTT_ASSERT(rrr->item.right == NULL);
    key = (char*)(rrr->item.key);
    RTT_ASSERT(strcmp(key, "00000200") == 0);
    RTT_ASSERT(rrr->item.factor == 0);
    RTT_ASSERT(rrr->value == 200);
}
RTT_TEST_END

RTT_TEST_START(cds_prepare_deep_map_for_insertion_with_RR_rotation)
{
    RTT_ASSERT(CdsMapRemove(gMap, "00000010"));
}
RTT_TEST_END

RTT_TEST_START(cds_should_perform_RR_rotation_when_inserting_into_deep_map)
{
    TestItem* item = testItemAlloc(250);
    char* key = testKeyCreate(250);
    RTT_ASSERT(CdsMapInsert(gMap, key, (CdsMapItem*)item));
}
RTT_TEST_END

/* Tree at this stage

                   100
             50           150
        25     75      125   200
      12  33     82             250
*/
RTT_TEST_START(cds_check_deep_map_after_insertion_with_RR_rotation)
{
    TestItem* root = *((TestItem**)gMap);
    RTT_ASSERT(root != NULL);
    RTT_ASSERT(root->item.parent == NULL);
    char* key = (char*)(root->item.key);
    RTT_ASSERT(strcmp(key, "00000100") == 0);
    RTT_ASSERT(root->item.factor == 0);
    RTT_ASSERT(root->value == 100);

    TestItem* l = (TestItem*)(root->item.left);
    RTT_ASSERT(l != NULL);
    RTT_ASSERT(l->item.parent == (CdsMapItem*)root);
    key = (char*)(l->item.key);
    RTT_ASSERT(strcmp(key, "00000050") == 0);
    RTT_ASSERT(l->item.factor == 0);
    RTT_ASSERT(l->value == 50);

    TestItem* ll = (TestItem*)(l->item.left);
    RTT_ASSERT(ll != NULL);
    RTT_ASSERT(ll->item.parent == (CdsMapItem*)l);
    key = (char*)(ll->item.key);
    RTT_ASSERT(strcmp(key, "00000025") == 0);
    RTT_ASSERT(ll->item.factor == 0);
    RTT_ASSERT(ll->value == 25);

    TestItem* lll = (TestItem*)(ll->item.left);
    RTT_ASSERT(lll != NULL);
    RTT_ASSERT(lll->item.parent == (CdsMapItem*)ll);
    RTT_ASSERT(lll->item.left == NULL);
    RTT_ASSERT(lll->item.right == NULL);
    key = (char*)(lll->item.key);
    RTT_ASSERT(strcmp(key, "00000012") == 0);
    RTT_ASSERT(lll->item.factor == 0);
    RTT_ASSERT(lll->value == 12);

    TestItem* llr = (TestItem*)(ll->item.right);
    RTT_ASSERT(llr != NULL);
    RTT_ASSERT(llr->item.parent == (CdsMapItem*)ll);
    RTT_ASSERT(llr->item.left == NULL);
    RTT_ASSERT(llr->item.right == NULL);
    key = (char*)(llr->item.key);
    RTT_ASSERT(strcmp(key, "00000033") == 0);
    RTT_ASSERT(llr->item.factor == 0);
    RTT_ASSERT(llr->value == 33);

    TestItem* lr = (TestItem*)(l->item.right);
    RTT_ASSERT(lr != NULL);
    RTT_ASSERT(lr->item.parent == (CdsMapItem*)l);
    RTT_ASSERT(lr->item.left == NULL);
    key = (char*)(lr->item.key);
    RTT_ASSERT(strcmp(key, "00000075") == 0);
    RTT_ASSERT(lr->item.factor == 1);
    RTT_ASSERT(lr->value = 75);

    TestItem* lrr = (TestItem*)(lr->item.right);
    RTT_ASSERT(lrr != NULL);
    RTT_ASSERT(lrr->item.parent == (CdsMapItem*)lr);
    RTT_ASSERT(lrr->item.left == NULL);
    RTT_ASSERT(lrr->item.right == NULL);
    key = (char*)(lrr->item.key);
    RTT_ASSERT(strcmp(key, "00000082") == 0);
    RTT_ASSERT(lrr->item.factor == 0);
    RTT_ASSERT(lrr->value == 82);

    TestItem* r = (TestItem*)(root->item.right);
    RTT_ASSERT(r != NULL);
    RTT_ASSERT(r->item.parent == (CdsMapItem*)root);
    key = (char*)(r->item.key);
    RTT_ASSERT(strcmp(key, "00000150") == 0);
    RTT_ASSERT(r->item.factor == 1);
    RTT_ASSERT(r->value == 150);

    TestItem* rl = (TestItem*)(r->item.left);
    RTT_ASSERT(rl != NULL);
    RTT_ASSERT(rl->item.parent = (CdsMapItem*)r);
    RTT_ASSERT(rl->item.left == NULL);
    RTT_ASSERT(rl->item.right == NULL);
    key = (char*)(rl->item.key);
    RTT_ASSERT(strcmp(key, "00000125") == 0);
    RTT_ASSERT(rl->item.factor == 0);
    RTT_ASSERT(rl->value == 125);

    TestItem* rr = (TestItem*)(r->item.right);
    RTT_ASSERT(rr != NULL);
    RTT_ASSERT(rr->item.parent == (CdsMapItem*)r);
    key = (char*)(rr->item.key);
    RTT_ASSERT(strcmp(key, "00000200") == 0);
    RTT_ASSERT(rr->item.factor == 1);
    RTT_ASSERT(rr->value == 200);

    TestItem* rrr = (TestItem*)(rr->item.right);
    RTT_ASSERT(rrr != NULL);
    RTT_ASSERT(rrr->item.parent == (CdsMapItem*)rr);
    RTT_ASSERT(rrr->item.left == NULL);
    RTT_ASSERT(rrr->item.right == NULL);
    key = (char*)(rrr->item.key);
    RTT_ASSERT(strcmp(key, "00000250") == 0);
    RTT_ASSERT(rrr->item.factor == 0);
    RTT_ASSERT(rrr->value == 250);
}
RTT_TEST_END

RTT_TEST_START(cds_prepare_deep_map_for_insertion_with_LR_rotation)
{
    RTT_ASSERT(CdsMapRemove(gMap, "00000250"));

    TestItem* item = testItemAlloc(70);
    char* key = testKeyCreate(70);
    RTT_ASSERT(CdsMapInsert(gMap, key, (CdsMapItem*)item));
}
RTT_TEST_END


/* Tree at this stage

                     100
             50            150
        25       75     125   200
      12  33   70  82
*/
RTT_TEST_START(cds_should_perform_LR_rotation_when_inserting_into_deep_map)
{
    TestItem* item = testItemAlloc(80);
    char* key = testKeyCreate(80);
    RTT_ASSERT(CdsMapInsert(gMap, key, (CdsMapItem*)item));
}
RTT_TEST_END

/* Tree at this stage

              75
         50         100
     25    70     82      150
   12  33       80     125   200
*/
RTT_TEST_START(cds_check_deep_map_after_insertion_with_LR_rotation)
{
    TestItem* root = *((TestItem**)gMap);
    RTT_ASSERT(root != NULL);
    RTT_ASSERT(root->item.parent == NULL);
    char* key = (char*)(root->item.key);
    RTT_ASSERT(strcmp(key, "00000075") == 0);
    RTT_ASSERT(root->item.factor == 0);
    RTT_ASSERT(root->value == 75);

    TestItem* l = (TestItem*)(root->item.left);
    RTT_ASSERT(l != NULL);
    RTT_ASSERT(l->item.parent == (CdsMapItem*)root);
    key = (char*)(l->item.key);
    RTT_ASSERT(strcmp(key, "00000050") == 0);
    RTT_ASSERT(l->item.factor == -1);
    RTT_ASSERT(l->value == 50);

    TestItem* ll = (TestItem*)(l->item.left);
    RTT_ASSERT(ll != NULL);
    RTT_ASSERT(ll->item.parent == (CdsMapItem*)l);
    key = (char*)(ll->item.key);
    RTT_ASSERT(strcmp(key, "00000025") == 0);
    RTT_ASSERT(ll->item.factor == 0);
    RTT_ASSERT(ll->value == 25);

    TestItem* lll = (TestItem*)(ll->item.left);
    RTT_ASSERT(lll != NULL);
    RTT_ASSERT(lll->item.parent == (CdsMapItem*)ll);
    RTT_ASSERT(lll->item.left == NULL);
    RTT_ASSERT(lll->item.right == NULL);
    key = (char*)(lll->item.key);
    RTT_ASSERT(strcmp(key, "00000012") == 0);
    RTT_ASSERT(lll->item.factor == 0);
    RTT_ASSERT(lll->value == 12);

    TestItem* llr = (TestItem*)(ll->item.right);
    RTT_ASSERT(llr != NULL);
    RTT_ASSERT(llr->item.parent == (CdsMapItem*)ll);
    RTT_ASSERT(llr->item.left == NULL);
    RTT_ASSERT(llr->item.right == NULL);
    key = (char*)(llr->item.key);
    RTT_ASSERT(strcmp(key, "00000033") == 0);
    RTT_ASSERT(llr->item.factor == 0);
    RTT_ASSERT(llr->value == 33);

    TestItem* lr = (TestItem*)(l->item.right);
    RTT_ASSERT(lr != NULL);
    RTT_ASSERT(lr->item.parent == (CdsMapItem*)l);
    RTT_ASSERT(lr->item.left == NULL);
    RTT_ASSERT(lr->item.right == NULL);
    key = (char*)(lr->item.key);
    RTT_ASSERT(strcmp(key, "00000070") == 0);
    RTT_ASSERT(lr->item.factor == 0);
    RTT_ASSERT(lr->value = 70);

    TestItem* r = (TestItem*)(root->item.right);
    RTT_ASSERT(r != NULL);
    RTT_ASSERT(r->item.parent == (CdsMapItem*)root);
    key = (char*)(r->item.key);
    RTT_ASSERT(strcmp(key, "00000100") == 0);
    RTT_ASSERT(r->item.factor == 0);
    RTT_ASSERT(r->value == 100);

    TestItem* rl = (TestItem*)(r->item.left);
    RTT_ASSERT(rl != NULL);
    RTT_ASSERT(rl->item.parent = (CdsMapItem*)r);
    RTT_ASSERT(rl->item.right == NULL);
    key = (char*)(rl->item.key);
    RTT_ASSERT(strcmp(key, "00000082") == 0);
    RTT_ASSERT(rl->item.factor == -1);
    RTT_ASSERT(rl->value == 82);

    TestItem* rll = (TestItem*)(rl->item.left);
    RTT_ASSERT(rll != NULL);
    RTT_ASSERT(rll->item.parent == (CdsMapItem*)rl);
    RTT_ASSERT(rll->item.left == NULL);
    RTT_ASSERT(rll->item.right == NULL);
    key = (char*)(rll->item.key);
    RTT_ASSERT(strcmp(key, "00000080") == 0);
    RTT_ASSERT(rll->item.factor == 0);
    RTT_ASSERT(rll->value == 80);

    TestItem* rr = (TestItem*)(r->item.right);
    RTT_ASSERT(rr != NULL);
    RTT_ASSERT(rr->item.parent == (CdsMapItem*)r);
    key = (char*)(rr->item.key);
    RTT_ASSERT(strcmp(key, "00000150") == 0);
    RTT_ASSERT(rr->item.factor == 0);
    RTT_ASSERT(rr->value == 150);

    TestItem* rrr = (TestItem*)(rr->item.right);
    RTT_ASSERT(rrr != NULL);
    RTT_ASSERT(rrr->item.parent == (CdsMapItem*)rr);
    RTT_ASSERT(rrr->item.left == NULL);
    RTT_ASSERT(rrr->item.right == NULL);
    key = (char*)(rrr->item.key);
    RTT_ASSERT(strcmp(key, "00000200") == 0);
    RTT_ASSERT(rrr->item.factor == 0);
    RTT_ASSERT(rrr->value == 200);
}
RTT_TEST_END

RTT_TEST_START(cds_prepare_deep_map_for_insertion_with_RL_rotation)
{
    RTT_ASSERT(CdsMapRemove(gMap, "00000012"));
    RTT_ASSERT(CdsMapRemove(gMap, "00000033"));

    TestItem* item = testItemAlloc(90);
    char* key = testKeyCreate(90);
    RTT_ASSERT(CdsMapInsert(gMap, key, (CdsMapItem*)item));
}
RTT_TEST_END

/* Tree at this stage

              75
         50           100
     25    70     82        150
                80  90   125   200
*/
RTT_TEST_START(cds_should_perform_RL_rotation_when_inserting_into_deep_map)
{
    TestItem* item = testItemAlloc(78);
    char* key = testKeyCreate(78);
    RTT_ASSERT(CdsMapInsert(gMap, key, (CdsMapItem*)item));
}
RTT_TEST_END

/* Tree at this stage

                82
          75        100
      50      80  90      150
    25  70  78         125   200
*/
RTT_TEST_START(cds_check_deep_map_after_insertion_with_RL_rotation)
{
    TestItem* root = *((TestItem**)gMap);
    RTT_ASSERT(root != NULL);
    RTT_ASSERT(root->item.parent == NULL);
    char* key = (char*)(root->item.key);
    RTT_ASSERT(strcmp(key, "00000082") == 0);
    RTT_ASSERT(root->item.factor == 0);
    RTT_ASSERT(root->value == 82);

    TestItem* l = (TestItem*)(root->item.left);
    RTT_ASSERT(l != NULL);
    RTT_ASSERT(l->item.parent == (CdsMapItem*)root);
    key = (char*)(l->item.key);
    RTT_ASSERT(strcmp(key, "00000075") == 0);
    RTT_ASSERT(l->item.factor == 0);
    RTT_ASSERT(l->value == 75);

    TestItem* ll = (TestItem*)(l->item.left);
    RTT_ASSERT(ll != NULL);
    RTT_ASSERT(ll->item.parent == (CdsMapItem*)l);
    key = (char*)(ll->item.key);
    RTT_ASSERT(strcmp(key, "00000050") == 0);
    RTT_ASSERT(ll->item.factor == 0);
    RTT_ASSERT(ll->value == 50);

    TestItem* lll = (TestItem*)(ll->item.left);
    RTT_ASSERT(lll != NULL);
    RTT_ASSERT(lll->item.parent == (CdsMapItem*)ll);
    RTT_ASSERT(lll->item.left == NULL);
    RTT_ASSERT(lll->item.right == NULL);
    key = (char*)(lll->item.key);
    RTT_ASSERT(strcmp(key, "00000025") == 0);
    RTT_ASSERT(lll->item.factor == 0);
    RTT_ASSERT(lll->value == 25);

    TestItem* llr = (TestItem*)(ll->item.right);
    RTT_ASSERT(llr != NULL);
    RTT_ASSERT(llr->item.parent == (CdsMapItem*)ll);
    RTT_ASSERT(llr->item.left == NULL);
    RTT_ASSERT(llr->item.right == NULL);
    key = (char*)(llr->item.key);
    RTT_ASSERT(strcmp(key, "00000070") == 0);
    RTT_ASSERT(llr->item.factor == 0);
    RTT_ASSERT(llr->value == 70);

    TestItem* lr = (TestItem*)(l->item.right);
    RTT_ASSERT(lr != NULL);
    RTT_ASSERT(lr->item.parent == (CdsMapItem*)l);
    RTT_ASSERT(lr->item.right == NULL);
    key = (char*)(lr->item.key);
    RTT_ASSERT(strcmp(key, "00000080") == 0);
    RTT_ASSERT(lr->item.factor == -1);
    RTT_ASSERT(lr->value = 80);

    TestItem* lrl = (TestItem*)(lr->item.left);
    RTT_ASSERT(lrl != NULL);
    RTT_ASSERT(lrl->item.parent == (CdsMapItem*)lr);
    RTT_ASSERT(lrl->item.left == NULL);
    RTT_ASSERT(lrl->item.right == NULL);
    key = (char*)(lrl->item.key);
    RTT_ASSERT(strcmp(key, "00000078") == 0);
    RTT_ASSERT(lrl->item.factor == 0);
    RTT_ASSERT(lrl->value == 78);

    TestItem* r = (TestItem*)(root->item.right);
    RTT_ASSERT(r != NULL);
    RTT_ASSERT(r->item.parent == (CdsMapItem*)root);
    key = (char*)(r->item.key);
    RTT_ASSERT(strcmp(key, "00000100") == 0);
    RTT_ASSERT(r->item.factor == 1);
    RTT_ASSERT(r->value == 100);

    TestItem* rl = (TestItem*)(r->item.left);
    RTT_ASSERT(rl != NULL);
    RTT_ASSERT(rl->item.parent = (CdsMapItem*)r);
    RTT_ASSERT(rl->item.left == NULL);
    RTT_ASSERT(rl->item.right == NULL);
    key = (char*)(rl->item.key);
    RTT_ASSERT(strcmp(key, "00000090") == 0);
    RTT_ASSERT(rl->item.factor == 0);
    RTT_ASSERT(rl->value == 90);

    TestItem* rr = (TestItem*)(r->item.right);
    RTT_ASSERT(rr != NULL);
    RTT_ASSERT(rr->item.parent == (CdsMapItem*)r);
    key = (char*)(rr->item.key);
    RTT_ASSERT(strcmp(key, "00000150") == 0);
    RTT_ASSERT(rr->item.factor == 0);
    RTT_ASSERT(rr->value == 150);

    TestItem* rrl = (TestItem*)(rr->item.left);
    RTT_ASSERT(rrl != NULL);
    RTT_ASSERT(rrl->item.parent == (CdsMapItem*)rr);
    RTT_ASSERT(rrl->item.left == NULL);
    RTT_ASSERT(rrl->item.right == NULL);
    key = (char*)(rrl->item.key);
    RTT_ASSERT(strcmp(key, "00000125") == 0);
    RTT_ASSERT(rrl->item.factor == 0);
    RTT_ASSERT(rrl->value == 125);

    TestItem* rrr = (TestItem*)(rr->item.right);
    RTT_ASSERT(rrr != NULL);
    RTT_ASSERT(rrr->item.parent == (CdsMapItem*)rr);
    RTT_ASSERT(rrr->item.left == NULL);
    RTT_ASSERT(rrr->item.right == NULL);
    key = (char*)(rrr->item.key);
    RTT_ASSERT(strcmp(key, "00000200") == 0);
    RTT_ASSERT(rrr->item.factor == 0);
    RTT_ASSERT(rrr->value == 200);
}
RTT_TEST_END

RTT_TEST_START(cds_prepare_deep_map_for_leaf_removal_with_RR_rotation)
{
    TestItem* item = testItemAlloc(95);
    char* key = testKeyCreate(95);
    RTT_ASSERT(CdsMapInsert(gMap, key, (CdsMapItem*)item));

    item = testItemAlloc(130);
    key = testKeyCreate(130);
    RTT_ASSERT(CdsMapInsert(gMap, key, (CdsMapItem*)item));

    item = testItemAlloc(250);
    key = testKeyCreate(250);
    RTT_ASSERT(CdsMapInsert(gMap, key, (CdsMapItem*)item));

    RTT_ASSERT(CdsMapRemove(gMap, "00000025"));
    RTT_ASSERT(CdsMapRemove(gMap, "00000070"));
}
RTT_TEST_END

/* Tree at this stage

            82
      75          100
    50    80  90           150
        78      95   125      200
                        130      250
*/
RTT_TEST_START(cds_check_deep_map_before_leaf_removal_with_RR_rotation)
{
    TestItem* root = *((TestItem**)gMap);
    RTT_ASSERT(root != NULL);
    RTT_ASSERT(root->item.parent == NULL);
    char* key = (char*)(root->item.key);
    RTT_ASSERT(strcmp(key, "00000082") == 0);
    RTT_ASSERT(root->item.factor == 1);
    RTT_ASSERT(root->value == 82);

    TestItem* l = (TestItem*)(root->item.left);
    RTT_ASSERT(l != NULL);
    RTT_ASSERT(l->item.parent == (CdsMapItem*)root);
    key = (char*)(l->item.key);
    RTT_ASSERT(strcmp(key, "00000075") == 0);
    RTT_ASSERT(l->item.factor == 1);
    RTT_ASSERT(l->value == 75);

    TestItem* ll = (TestItem*)(l->item.left);
    RTT_ASSERT(ll != NULL);
    RTT_ASSERT(ll->item.parent == (CdsMapItem*)l);
    RTT_ASSERT(ll->item.left == NULL);
    RTT_ASSERT(ll->item.right == NULL);
    key = (char*)(ll->item.key);
    RTT_ASSERT(strcmp(key, "00000050") == 0);
    RTT_ASSERT(ll->item.factor == 0);
    RTT_ASSERT(ll->value ==  50);

    TestItem* lr = (TestItem*)(l->item.right);
    RTT_ASSERT(lr != NULL);
    RTT_ASSERT(lr->item.parent == (CdsMapItem*)l);
    RTT_ASSERT(lr->item.right == NULL);
    key = (char*)(lr->item.key);
    RTT_ASSERT(strcmp(key, "00000080") == 0);
    RTT_ASSERT(lr->item.factor == -1);
    RTT_ASSERT(lr->value == 80);

    TestItem* lrl = (TestItem*)(lr->item.left);
    RTT_ASSERT(lrl != NULL);
    RTT_ASSERT(lrl->item.parent == (CdsMapItem*)lr);
    RTT_ASSERT(lrl->item.left == NULL);
    RTT_ASSERT(lrl->item.right == NULL);
    key = (char*)(lrl->item.key);
    RTT_ASSERT(strcmp(key, "00000078") == 0);
    RTT_ASSERT(lrl->item.factor == 0);
    RTT_ASSERT(lrl->value == 78);

    TestItem* r = (TestItem*)(root->item.right);
    RTT_ASSERT(r != NULL);
    RTT_ASSERT(r->item.parent == (CdsMapItem*)root);
    key = (char*)(r->item.key);
    RTT_ASSERT(strcmp(key, "00000100") == 0);
    RTT_ASSERT(r->item.factor == 1);
    RTT_ASSERT(r->value == 100);

    TestItem* rl = (TestItem*)(r->item.left);
    RTT_ASSERT(rl != NULL);
    RTT_ASSERT(rl->item.parent == (CdsMapItem*)r);
    RTT_ASSERT(rl->item.left == NULL);
    key = (char*)(rl->item.key);
    RTT_ASSERT(strcmp(key, "00000090") == 0);
    RTT_ASSERT(rl->item.factor == 1);
    RTT_ASSERT(rl->value == 90);

    TestItem* rlr = (TestItem*)(rl->item.right);
    RTT_ASSERT(rlr != NULL);
    RTT_ASSERT(rlr->item.parent == (CdsMapItem*)rl);
    RTT_ASSERT(rlr->item.left == NULL);
    RTT_ASSERT(rlr->item.right == NULL);
    key = (char*)(rlr->item.key);
    RTT_ASSERT(strcmp(key, "00000095") == 0);
    RTT_ASSERT(rlr->item.factor == 0);
    RTT_ASSERT(rlr->value == 95);

    TestItem* rr = (TestItem*)(r->item.right);
    RTT_ASSERT(rr != NULL);
    RTT_ASSERT(rr->item.parent == (CdsMapItem*)r);
    key = (char*)(rr->item.key);
    RTT_ASSERT(strcmp(key, "00000150") == 0);
    RTT_ASSERT(rr->item.factor == 0);
    RTT_ASSERT(rr->value == 150);

    TestItem* rrl = (TestItem*)(rr->item.left);
    RTT_ASSERT(rrl != NULL);
    RTT_ASSERT(rrl->item.parent == (CdsMapItem*)rr);
    RTT_ASSERT(rrl->item.left == NULL);
    key = (char*)(rrl->item.key);
    RTT_ASSERT(strcmp(key, "00000125") == 0);
    RTT_ASSERT(rrl->item.factor == 1);
    RTT_ASSERT(rrl->value == 125);

    TestItem* rrlr = (TestItem*)(rrl->item.right);
    RTT_ASSERT(rrlr != NULL);
    RTT_ASSERT(rrlr->item.parent == (CdsMapItem*)rrl);
    RTT_ASSERT(rrlr->item.left == NULL);
    RTT_ASSERT(rrlr->item.right == NULL);
    key = (char*)(rrlr->item.key);
    RTT_ASSERT(strcmp(key, "00000130") == 0);
    RTT_ASSERT(rrlr->item.factor == 0);
    RTT_ASSERT(rrlr->value == 130);

    TestItem* rrr = (TestItem*)(rr->item.right);
    RTT_ASSERT(rrr != NULL);
    RTT_ASSERT(rrr->item.parent == (CdsMapItem*)rr);
    RTT_ASSERT(rrr->item.left == NULL);
    key = (char*)(rrr->item.key);
    RTT_ASSERT(strcmp(key, "00000200") == 0);
    RTT_ASSERT(rrr->item.factor == 1);
    RTT_ASSERT(rrr->value == 200);

    TestItem* rrrr = (TestItem*)(rrr->item.right);
    RTT_ASSERT(rrrr != NULL);
    RTT_ASSERT(rrrr->item.parent == (CdsMapItem*)rrr);
    RTT_ASSERT(rrrr->item.left == NULL);
    RTT_ASSERT(rrrr->item.right == NULL);
    key = (char*)(rrrr->item.key);
    RTT_ASSERT(strcmp(key, "00000250") == 0);
    RTT_ASSERT(rrrr->item.factor == 0);
    RTT_ASSERT(rrrr->value == 250);
}
RTT_TEST_END

RTT_TEST_START(cds_should_perform_RR_rotation_when_removing_leaf_item)
{
    RTT_ASSERT(CdsMapRemove(gMap, "00000078"));
}
RTT_TEST_END

/* Tree at this stage

                 100
            82            150
        75    90    125      200
      50  80    95     130      250
*/
RTT_TEST_START(cds_check_deep_map_after_leaf_removal_with_RR_rotation)
{
    TestItem* root = *((TestItem**)gMap);
    RTT_ASSERT(root != NULL);
    RTT_ASSERT(root->item.parent == NULL);
    char* key = (char*)(root->item.key);
    RTT_ASSERT(strcmp(key, "00000100") == 0);
    RTT_ASSERT(root->item.factor == 0);
    RTT_ASSERT(root->value == 100);

    TestItem* l = (TestItem*)(root->item.left);
    RTT_ASSERT(l != NULL);
    RTT_ASSERT(l->item.parent == (CdsMapItem*)root);
    key = (char*)(l->item.key);
    RTT_ASSERT(strcmp(key, "00000082") == 0);
    RTT_ASSERT(l->item.factor == 0);
    RTT_ASSERT(l->value == 82);

    TestItem* ll = (TestItem*)(l->item.left);
    RTT_ASSERT(ll != NULL);
    RTT_ASSERT(ll->item.parent == (CdsMapItem*)l);
    key = (char*)(ll->item.key);
    RTT_ASSERT(strcmp(key, "00000075") == 0);
    RTT_ASSERT(ll->item.factor == 0);
    RTT_ASSERT(ll->value == 75);

    TestItem* lll = (TestItem*)(ll->item.left);
    RTT_ASSERT(lll != NULL);
    RTT_ASSERT(lll->item.parent == (CdsMapItem*)ll);
    RTT_ASSERT(lll->item.left == NULL);
    RTT_ASSERT(lll->item.right == NULL);
    key = (char*)(lll->item.key);
    RTT_ASSERT(strcmp(key, "00000050") == 0);
    RTT_ASSERT(lll->item.factor == 0);
    RTT_ASSERT(lll->value == 50);

    TestItem* llr = (TestItem*)(ll->item.right);
    RTT_ASSERT(llr != NULL);
    RTT_ASSERT(llr->item.parent == (CdsMapItem*)ll);
    RTT_ASSERT(llr->item.left == NULL);
    RTT_ASSERT(llr->item.right == NULL);
    key = (char*)(llr->item.key);
    RTT_ASSERT(strcmp(key, "00000080") == 0);
    RTT_ASSERT(llr->item.factor == 0);
    RTT_ASSERT(llr->value == 80);

    TestItem* lr = (TestItem*)(l->item.right);
    RTT_ASSERT(lr != NULL);
    RTT_ASSERT(lr->item.parent == (CdsMapItem*)l);
    RTT_ASSERT(lr->item.left == NULL);
    key = (char*)(lr->item.key);
    RTT_ASSERT(strcmp(key, "00000090") == 0);
    RTT_ASSERT(lr->item.factor == 1);
    RTT_ASSERT(lr->value = 90);

    TestItem* lrr = (TestItem*)(lr->item.right);
    RTT_ASSERT(lrr != NULL);
    RTT_ASSERT(lrr->item.parent == (CdsMapItem*)lr);
    RTT_ASSERT(lrr->item.left == NULL);
    RTT_ASSERT(lrr->item.right == NULL);
    key = (char*)(lrr->item.key);
    RTT_ASSERT(strcmp(key, "00000095") == 0);
    RTT_ASSERT(lrr->item.factor == 0);
    RTT_ASSERT(lrr->value == 95);

    TestItem* r = (TestItem*)(root->item.right);
    RTT_ASSERT(r != NULL);
    RTT_ASSERT(r->item.parent == (CdsMapItem*)root);
    key = (char*)(r->item.key);
    RTT_ASSERT(strcmp(key, "00000150") == 0);
    RTT_ASSERT(r->item.factor == 0);
    RTT_ASSERT(r->value == 150);

    TestItem* rl = (TestItem*)(r->item.left);
    RTT_ASSERT(rl != NULL);
    RTT_ASSERT(rl->item.parent == (CdsMapItem*)r);
    RTT_ASSERT(rl->item.left == NULL);
    key = (char*)(rl->item.key);
    RTT_ASSERT(strcmp(key, "00000125") == 0);
    RTT_ASSERT(rl->item.factor == 1);
    RTT_ASSERT(rl->value == 125);

    TestItem* rlr = (TestItem*)(rl->item.right);
    RTT_ASSERT(rlr != NULL);
    RTT_ASSERT(rlr->item.parent == (CdsMapItem*)rl);
    RTT_ASSERT(rlr->item.left == NULL);
    RTT_ASSERT(rlr->item.right == NULL);
    key = (char*)(rlr->item.key);
    RTT_ASSERT(strcmp(key, "00000130") == 0);
    RTT_ASSERT(rlr->item.factor == 0);
    RTT_ASSERT(rlr->value == 130);

    TestItem* rr = (TestItem*)(r->item.right);
    RTT_ASSERT(rr != NULL);
    RTT_ASSERT(rr->item.parent == (CdsMapItem*)r);
    RTT_ASSERT(rr->item.left == NULL);
    key = (char*)(rr->item.key);
    RTT_ASSERT(strcmp(key, "00000200") == 0);
    RTT_ASSERT(rr->item.factor == 1);
    RTT_ASSERT(rr->value == 200);

    TestItem* rrr = (TestItem*)(rr->item.right);
    RTT_ASSERT(rrr != NULL);
    RTT_ASSERT(rrr->item.parent == (CdsMapItem*)rr);
    RTT_ASSERT(rrr->item.left == NULL);
    RTT_ASSERT(rrr->item.right == NULL);
    key = (char*)(rrr->item.key);
    RTT_ASSERT(strcmp(key, "00000250") == 0);
    RTT_ASSERT(rrr->item.factor == 0);
    RTT_ASSERT(rrr->value == 250);
}
RTT_TEST_END

RTT_TEST_START(cds_prepare_deep_map_for_leaf_removal_with_LL_rotation)
{
    TestItem* item = testItemAlloc(81);
    char* key = testKeyCreate(81);
    RTT_ASSERT(CdsMapInsert(gMap, key, (CdsMapItem*)item));

    RTT_ASSERT(CdsMapRemove(gMap, "00000250"));
}
RTT_TEST_END

/* Tree at this stage

                   100
              82            150
        75      90    125      200
      50  80      95     130
            81
*/
RTT_TEST_START(cds_should_perform_LL_rotation_when_removing_leaf_item)
{
    RTT_ASSERT(CdsMapRemove(gMap, "00000125"));
}
RTT_TEST_END

/* Tree at this stage

            82
      75          100
    50  80    90        150
          81    95   130   200
*/
RTT_TEST_START(cds_check_deep_map_after_leaf_removal_with_LL_rotation)
{
    TestItem* root = *((TestItem**)gMap);
    RTT_ASSERT(root != NULL);
    RTT_ASSERT(root->item.parent == NULL);
    char* key = (char*)(root->item.key);
    RTT_ASSERT(strcmp(key, "00000082") == 0);
    RTT_ASSERT(root->item.factor == 0);
    RTT_ASSERT(root->value == 82);

    TestItem* l = (TestItem*)(root->item.left);
    RTT_ASSERT(l != NULL);
    RTT_ASSERT(l->item.parent == (CdsMapItem*)root);
    key = (char*)(l->item.key);
    RTT_ASSERT(strcmp(key, "00000075") == 0);
    RTT_ASSERT(l->item.factor == 1);
    RTT_ASSERT(l->value == 75);

    TestItem* ll = (TestItem*)(l->item.left);
    RTT_ASSERT(ll != NULL);
    RTT_ASSERT(ll->item.parent == (CdsMapItem*)l);
    RTT_ASSERT(ll->item.left == NULL);
    RTT_ASSERT(ll->item.right == NULL);
    key = (char*)(ll->item.key);
    RTT_ASSERT(strcmp(key, "00000050") == 0);
    RTT_ASSERT(ll->item.factor == 0);
    RTT_ASSERT(ll->value == 50);

    TestItem* lr = (TestItem*)(l->item.right);
    RTT_ASSERT(lr != NULL);
    RTT_ASSERT(lr->item.parent == (CdsMapItem*)l);
    RTT_ASSERT(lr->item.left == NULL);
    key = (char*)(lr->item.key);
    RTT_ASSERT(strcmp(key, "00000080") == 0);
    RTT_ASSERT(lr->item.factor == 1);
    RTT_ASSERT(lr->value == 80);

    TestItem* lrr = (TestItem*)(lr->item.right);
    RTT_ASSERT(lrr != NULL);
    RTT_ASSERT(lrr->item.parent == (CdsMapItem*)lr);
    RTT_ASSERT(lrr->item.left == NULL);
    RTT_ASSERT(lrr->item.right == NULL);
    key = (char*)(lrr->item.key);
    RTT_ASSERT(strcmp(key, "00000081") == 0);
    RTT_ASSERT(lrr->item.factor == 0);
    RTT_ASSERT(lrr->value == 81);

    TestItem* r = (TestItem*)(root->item.right);
    RTT_ASSERT(r != NULL);
    RTT_ASSERT(r->item.parent == (CdsMapItem*)root);
    key = (char*)(r->item.key);
    RTT_ASSERT(strcmp(key, "00000100") == 0);
    RTT_ASSERT(r->item.factor == 0);
    RTT_ASSERT(r->value == 100);

    TestItem* rl = (TestItem*)(r->item.left);
    RTT_ASSERT(rl != NULL);
    RTT_ASSERT(rl->item.parent == (CdsMapItem*)r);
    RTT_ASSERT(rl->item.left == NULL);
    key = (char*)(rl->item.key);
    RTT_ASSERT(strcmp(key, "00000090") == 0);
    RTT_ASSERT(rl->item.factor == 1);
    RTT_ASSERT(rl->value == 90);

    TestItem* rlr = (TestItem*)(rl->item.right);
    RTT_ASSERT(rlr != NULL);
    RTT_ASSERT(rlr->item.parent == (CdsMapItem*)rl);
    RTT_ASSERT(rlr->item.left == NULL);
    RTT_ASSERT(rlr->item.right == NULL);
    key = (char*)(rlr->item.key);
    RTT_ASSERT(strcmp(key, "00000095") == 0);
    RTT_ASSERT(rlr->item.factor == 0);
    RTT_ASSERT(rlr->value == 95);

    TestItem* rr = (TestItem*)(r->item.right);
    RTT_ASSERT(rr != NULL);
    RTT_ASSERT(rr->item.parent == (CdsMapItem*)r);
    key = (char*)(rr->item.key);
    RTT_ASSERT(strcmp(key, "00000150") == 0);
    RTT_ASSERT(rr->item.factor == 0);
    RTT_ASSERT(rr->value == 150);

    TestItem* rrl = (TestItem*)(rr->item.left);
    RTT_ASSERT(rrl != NULL);
    RTT_ASSERT(rrl->item.parent == (CdsMapItem*)rr);
    RTT_ASSERT(rrl->item.left == NULL);
    RTT_ASSERT(rrl->item.right == NULL);
    key = (char*)(rrl->item.key);
    RTT_ASSERT(strcmp(key, "00000130") == 0);
    RTT_ASSERT(rrl->item.factor == 0);
    RTT_ASSERT(rrl->value == 130);

    TestItem* rrr = (TestItem*)(rr->item.right);
    RTT_ASSERT(rrr != NULL);
    RTT_ASSERT(rrr->item.parent == (CdsMapItem*)rr);
    RTT_ASSERT(rrr->item.left == NULL);
    RTT_ASSERT(rrr->item.right == NULL);
    key = (char*)(rrr->item.key);
    RTT_ASSERT(strcmp(key, "00000200") == 0);
    RTT_ASSERT(rrr->item.factor == 0);
    RTT_ASSERT(rrr->value == 200);
}
RTT_TEST_END

RTT_TEST_START(cds_prepare_deep_map_for_leaf_removal_with_RL_rotation)
{
    TestItem* item = testItemAlloc(85);
    char* key = testKeyCreate(85);
    RTT_ASSERT(CdsMapInsert(gMap, key, (CdsMapItem*)item));

    item = testItemAlloc(87);
    key = testKeyCreate(87);
    RTT_ASSERT(CdsMapInsert(gMap, key, (CdsMapItem*)item));
}
RTT_TEST_END

/* Tree at this stage

            82
      75              100
    50  80        90        150
          81  85    95   130   200
                87
*/
RTT_TEST_START(cds_should_perform_RL_rotation_when_removing_leaf_item)
{
    RTT_ASSERT(CdsMapRemove(gMap, "00000081"));
}
RTT_TEST_END

/* Tree at this stage

              90
        82        100
    75    85    95      150
  50  80    87       130   200
*/
RTT_TEST_START(cds_check_deep_map_after_leaf_removal_with_RL_rotation)
{
    TestItem* root = *((TestItem**)gMap);
    RTT_ASSERT(root != NULL);
    RTT_ASSERT(root->item.parent == NULL);
    char* key = (char*)(root->item.key);
    RTT_ASSERT(strcmp(key, "00000090") == 0);
    RTT_ASSERT(root->item.factor == 0);
    RTT_ASSERT(root->value == 90);

    TestItem* l = (TestItem*)(root->item.left);
    RTT_ASSERT(l != NULL);
    RTT_ASSERT(l->item.parent == (CdsMapItem*)root);
    key = (char*)(l->item.key);
    RTT_ASSERT(strcmp(key, "00000082") == 0);
    RTT_ASSERT(l->item.factor == 0);
    RTT_ASSERT(l->value == 82);

    TestItem* ll = (TestItem*)(l->item.left);
    RTT_ASSERT(ll != NULL);
    RTT_ASSERT(ll->item.parent == (CdsMapItem*)l);
    key = (char*)(ll->item.key);
    RTT_ASSERT(strcmp(key, "00000075") == 0);
    RTT_ASSERT(ll->item.factor == 0);
    RTT_ASSERT(ll->value == 75);

    TestItem* lll = (TestItem*)(ll->item.left);
    RTT_ASSERT(lll != NULL);
    RTT_ASSERT(lll->item.parent == (CdsMapItem*)ll);
    RTT_ASSERT(lll->item.left == NULL);
    RTT_ASSERT(lll->item.right == NULL);
    key = (char*)(lll->item.key);
    RTT_ASSERT(strcmp(key, "00000050") == 0);
    RTT_ASSERT(lll->item.factor == 0);
    RTT_ASSERT(lll->value == 50);

    TestItem* llr = (TestItem*)(ll->item.right);
    RTT_ASSERT(llr != NULL);
    RTT_ASSERT(llr->item.parent == (CdsMapItem*)ll);
    RTT_ASSERT(llr->item.left == NULL);
    RTT_ASSERT(llr->item.right == NULL);
    key = (char*)(llr->item.key);
    RTT_ASSERT(strcmp(key, "00000080") == 0);
    RTT_ASSERT(llr->item.factor == 0);
    RTT_ASSERT(llr->value == 80);

    TestItem* lr = (TestItem*)(l->item.right);
    RTT_ASSERT(lr != NULL);
    RTT_ASSERT(lr->item.parent == (CdsMapItem*)l);
    RTT_ASSERT(lr->item.left == NULL);
    key = (char*)(lr->item.key);
    RTT_ASSERT(strcmp(key, "00000085") == 0);
    RTT_ASSERT(lr->item.factor == 1);
    RTT_ASSERT(lr->value == 85);

    TestItem* lrr = (TestItem*)(lr->item.right);
    RTT_ASSERT(lrr != NULL);
    RTT_ASSERT(lrr->item.parent == (CdsMapItem*)lr);
    RTT_ASSERT(lrr->item.left == NULL);
    RTT_ASSERT(lrr->item.right == NULL);
    key = (char*)(lrr->item.key);
    RTT_ASSERT(strcmp(key, "00000087") == 0);
    RTT_ASSERT(lrr->item.factor == 0);
    RTT_ASSERT(lrr->value == 87);

    TestItem* r = (TestItem*)(root->item.right);
    RTT_ASSERT(r != NULL);
    RTT_ASSERT(r->item.parent == (CdsMapItem*)root);
    key = (char*)(r->item.key);
    RTT_ASSERT(strcmp(key, "00000100") == 0);
    RTT_ASSERT(r->item.factor == 1);
    RTT_ASSERT(r->value == 100);

    TestItem* rl = (TestItem*)(r->item.left);
    RTT_ASSERT(rl != NULL);
    RTT_ASSERT(rl->item.parent == (CdsMapItem*)r);
    RTT_ASSERT(rl->item.left == NULL);
    RTT_ASSERT(rl->item.right == NULL);
    key = (char*)(rl->item.key);
    RTT_ASSERT(strcmp(key, "00000095") == 0);
    RTT_ASSERT(rl->item.factor == 0);
    RTT_ASSERT(rl->value == 95);

    TestItem* rr = (TestItem*)(r->item.right);
    RTT_ASSERT(rr != NULL);
    RTT_ASSERT(rr->item.parent == (CdsMapItem*)r);
    key = (char*)(rr->item.key);
    RTT_ASSERT(strcmp(key, "00000150") == 0);
    RTT_ASSERT(rr->item.factor == 0);
    RTT_ASSERT(rr->value == 150);

    TestItem* rrl = (TestItem*)(rr->item.left);
    RTT_ASSERT(rrl != NULL);
    RTT_ASSERT(rrl->item.parent == (CdsMapItem*)rr);
    RTT_ASSERT(rrl->item.left == NULL);
    RTT_ASSERT(rrl->item.right == NULL);
    key = (char*)(rrl->item.key);
    RTT_ASSERT(strcmp(key, "00000130") == 0);
    RTT_ASSERT(rrl->item.factor == 0);
    RTT_ASSERT(rrl->value == 130);

    TestItem* rrr = (TestItem*)(rr->item.right);
    RTT_ASSERT(rrr != NULL);
    RTT_ASSERT(rrr->item.parent == (CdsMapItem*)rr);
    RTT_ASSERT(rrr->item.left == NULL);
    RTT_ASSERT(rrr->item.right == NULL);
    key = (char*)(rrr->item.key);
    RTT_ASSERT(strcmp(key, "00000200") == 0);
    RTT_ASSERT(rrr->item.factor == 0);
    RTT_ASSERT(rrr->value == 200);
}
RTT_TEST_END

RTT_TEST_START(cds_prepare_deep_map_for_leaf_removal_with_LR_rotation)
{
    TestItem* item = testItemAlloc(83);
    char* key = testKeyCreate(83);
    RTT_ASSERT(CdsMapInsert(gMap, key, (CdsMapItem*)item));

    item = testItemAlloc(89);
    key = testKeyCreate(89);
    RTT_ASSERT(CdsMapInsert(gMap, key, (CdsMapItem*)item));

    RTT_ASSERT(CdsMapRemove(gMap, "00000200"));
}
RTT_TEST_END

/* Tree at this stage

                  90
        82            100
    75      85      95      150
  50  80  83  87         130
                89
*/
RTT_TEST_START(cds_should_perform_LR_rotation_when_removing_leaf_item)
{
    RTT_ASSERT(CdsMapRemove(gMap, "00000150"));
}
RTT_TEST_END

/* Tree at this stage

                85
         82           90
     75    83     87      100
   50  80           89  95   130
*/
RTT_TEST_START(cds_check_deep_map_after_leaf_removal_with_LR_rotation)
{
    TestItem* root = *((TestItem**)gMap);
    RTT_ASSERT(root != NULL);
    RTT_ASSERT(root->item.parent == NULL);
    char* key = (char*)(root->item.key);
    RTT_ASSERT(strcmp(key, "00000085") == 0);
    RTT_ASSERT(root->item.factor == 0);
    RTT_ASSERT(root->value == 85);

    TestItem* l = (TestItem*)(root->item.left);
    RTT_ASSERT(l != NULL);
    RTT_ASSERT(l->item.parent == (CdsMapItem*)root);
    key = (char*)(l->item.key);
    RTT_ASSERT(strcmp(key, "00000082") == 0);
    RTT_ASSERT(l->item.factor == -1);
    RTT_ASSERT(l->value == 82);

    TestItem* ll = (TestItem*)(l->item.left);
    RTT_ASSERT(ll != NULL);
    RTT_ASSERT(ll->item.parent == (CdsMapItem*)l);
    key = (char*)(ll->item.key);
    RTT_ASSERT(strcmp(key, "00000075") == 0);
    RTT_ASSERT(ll->item.factor == 0);
    RTT_ASSERT(ll->value == 75);

    TestItem* lll = (TestItem*)(ll->item.left);
    RTT_ASSERT(lll != NULL);
    RTT_ASSERT(lll->item.parent == (CdsMapItem*)ll);
    RTT_ASSERT(lll->item.left == NULL);
    RTT_ASSERT(lll->item.right == NULL);
    key = (char*)(lll->item.key);
    RTT_ASSERT(strcmp(key, "00000050") == 0);
    RTT_ASSERT(lll->item.factor == 0);
    RTT_ASSERT(lll->value == 50);

    TestItem* llr = (TestItem*)(ll->item.right);
    RTT_ASSERT(llr != NULL);
    RTT_ASSERT(llr->item.parent == (CdsMapItem*)ll);
    RTT_ASSERT(llr->item.left == NULL);
    RTT_ASSERT(llr->item.right == NULL);
    key = (char*)(llr->item.key);
    RTT_ASSERT(strcmp(key, "00000080") == 0);
    RTT_ASSERT(llr->item.factor == 0);
    RTT_ASSERT(llr->value == 80);

    TestItem* lr = (TestItem*)(l->item.right);
    RTT_ASSERT(lr != NULL);
    RTT_ASSERT(lr->item.parent == (CdsMapItem*)l);
    RTT_ASSERT(lr->item.left == NULL);
    RTT_ASSERT(lr->item.right == NULL);
    key = (char*)(lr->item.key);
    RTT_ASSERT(strcmp(key, "00000083") == 0);
    RTT_ASSERT(lr->item.factor == 0);
    RTT_ASSERT(lr->value == 83);

    TestItem* r = (TestItem*)(root->item.right);
    RTT_ASSERT(r != NULL);
    RTT_ASSERT(r->item.parent == (CdsMapItem*)root);
    key = (char*)(r->item.key);
    RTT_ASSERT(strcmp(key, "00000090") == 0);
    RTT_ASSERT(r->item.factor == 0);
    RTT_ASSERT(r->value == 90);

    TestItem* rl = (TestItem*)(r->item.left);
    RTT_ASSERT(rl != NULL);
    RTT_ASSERT(rl->item.parent == (CdsMapItem*)r);
    RTT_ASSERT(rl->item.left == NULL);
    key = (char*)(rl->item.key);
    RTT_ASSERT(strcmp(key, "00000087") == 0);
    RTT_ASSERT(rl->item.factor == 1);
    RTT_ASSERT(rl->value == 87);

    TestItem* rlr = (TestItem*)(rl->item.right);
    RTT_ASSERT(rlr != NULL);
    RTT_ASSERT(rlr->item.parent == (CdsMapItem*)rl);
    RTT_ASSERT(rlr->item.left == NULL);
    RTT_ASSERT(rlr->item.right == NULL);
    key = (char*)(rlr->item.key);
    RTT_ASSERT(strcmp(key, "00000089") == 0);
    RTT_ASSERT(rlr->item.factor == 0);
    RTT_ASSERT(rlr->value == 89);

    TestItem* rr = (TestItem*)(r->item.right);
    RTT_ASSERT(rr != NULL);
    RTT_ASSERT(rr->item.parent == (CdsMapItem*)r);
    key = (char*)(rr->item.key);
    RTT_ASSERT(strcmp(key, "00000100") == 0);
    RTT_ASSERT(rr->item.factor == 0);
    RTT_ASSERT(rr->value == 100);

    TestItem* rrl = (TestItem*)(rr->item.left);
    RTT_ASSERT(rrl != NULL);
    RTT_ASSERT(rrl->item.parent == (CdsMapItem*)rr);
    RTT_ASSERT(rrl->item.left == NULL);
    RTT_ASSERT(rrl->item.right == NULL);
    key = (char*)(rrl->item.key);
    RTT_ASSERT(strcmp(key, "00000095") == 0);
    RTT_ASSERT(rrl->item.factor == 0);
    RTT_ASSERT(rrl->value == 95);

    TestItem* rrr = (TestItem*)(rr->item.right);
    RTT_ASSERT(rrr != NULL);
    RTT_ASSERT(rrr->item.parent == (CdsMapItem*)rr);
    RTT_ASSERT(rrr->item.left == NULL);
    RTT_ASSERT(rrr->item.right == NULL);
    key = (char*)(rrr->item.key);
    RTT_ASSERT(strcmp(key, "00000130") == 0);
    RTT_ASSERT(rrr->item.factor == 0);
    RTT_ASSERT(rrr->value == 130);
}
RTT_TEST_END

RTT_TEST_START(cds_should_destroy_deep_map)
{
    CdsMapDestroy(gMap);
    RTT_ASSERT(gNumberOfItemsInExistence == 0);
    RTT_ASSERT(gNumberOfKeysInExistence == 0);
}
RTT_TEST_END

RTT_GROUP_END(TestCdsDeepMap,
        cds_should_create_deep_map,
        cds_should_build_initial_deep_map,
        cds_check_deep_map_inital_build,
        cds_should_perform_LL_rotation_when_inserting_into_deep_map,
        cds_check_deep_map_after_insertion_with_LL_rotation,
        cds_prepare_deep_map_for_insertion_with_RR_rotation,
        cds_should_perform_RR_rotation_when_inserting_into_deep_map,
        cds_check_deep_map_after_insertion_with_RR_rotation,
        cds_prepare_deep_map_for_insertion_with_LR_rotation,
        cds_should_perform_LR_rotation_when_inserting_into_deep_map,
        cds_check_deep_map_after_insertion_with_LR_rotation,
        cds_prepare_deep_map_for_insertion_with_RL_rotation,
        cds_should_perform_RL_rotation_when_inserting_into_deep_map,
        cds_check_deep_map_after_insertion_with_RL_rotation,
        cds_prepare_deep_map_for_leaf_removal_with_RR_rotation,
        cds_check_deep_map_before_leaf_removal_with_RR_rotation,
        cds_should_perform_RR_rotation_when_removing_leaf_item,
        cds_check_deep_map_after_leaf_removal_with_RR_rotation,
        cds_prepare_deep_map_for_leaf_removal_with_LL_rotation,
        cds_should_perform_LL_rotation_when_removing_leaf_item,
        cds_check_deep_map_after_leaf_removal_with_LL_rotation,
        cds_prepare_deep_map_for_leaf_removal_with_RL_rotation,
        cds_should_perform_RL_rotation_when_removing_leaf_item,
        cds_check_deep_map_after_leaf_removal_with_RL_rotation,
        cds_prepare_deep_map_for_leaf_removal_with_LR_rotation,
        cds_should_perform_LR_rotation_when_removing_leaf_item,
        cds_check_deep_map_after_leaf_removal_with_LR_rotation,
        cds_should_destroy_deep_map);
