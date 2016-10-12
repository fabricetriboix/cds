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

#include "cdscommon.h"
#include <stdlib.h>
#include <string.h>



/*---------------------------------+
 | Public function implementations |
 +---------------------------------*/


void* _CdsMalloc(size_t size_B, const char* file, int line)
{
    CDSASSERT(size_B > 0);
#ifdef CDS_WITH_FLLOC
    void* ptr = FllocMalloc(size_B, file, line);
#else
    (void)file;
    (void)line;
    void* ptr = malloc(size_B);
#endif
    CDSASSERT(ptr != NULL);
    return ptr;
}


void* _CdsMallocZ(size_t size_B, const char* file, int line)
{
    CDSASSERT(size_B > 0);
#ifdef CDS_WITH_FLLOC
    void* ptr = FllocMalloc(size_B, file, line);
#else
    (void)file;
    (void)line;
    void* ptr = malloc(size_B);
#endif
    CDSASSERT(ptr != NULL);
    memset(ptr, 0, size_B);
    return ptr;
}
