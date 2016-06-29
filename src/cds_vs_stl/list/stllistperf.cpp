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

#include <memory>
#include <list>
#include <cstdio>


class MyItem
{
public :
    MyItem(long long v)
    {
        value = v;
    }

    long long value;
};


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

    std::list<std::shared_ptr<MyItem>> list;

    printf("Inserting %lld items at the front\n", count / 2);
    for (long long i = 0; i < (count / 2); i++) {
        list.push_front(std::make_shared<MyItem>(i));
    }

    printf("Inserting %lld items at the back\n", count / 2);
    for (long long i = (count / 2); i < count; i++) {
        list.push_back(std::make_shared<MyItem>(i));
    }

    printf("Walking through the list\n");
    for (auto& item : list) {
        volatile long long x = item->value;
        (void)x;
    }

    printf("Popping %lld items from the front\n", count / 2);
    for (long long i = 0; i < (count / 2); i++) {
        list.pop_front();
    }

    printf("Popping %lld items from the back\n", count / 2);
    for (long long i = (count / 2); i < count; i++) {
        list.pop_back();
    }

    if (list.size() != 0) {
        fprintf(stderr, "ERROR: List size should be 0 after removing all items "
                "(currently it is %lld)\n", (long long)list.size());
        exit(1);
    }
    return 0;
}
