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

#ifndef CDSCOMMON_h_
#define CDSCOMMON_h_

/** Common stuff for C data structures
 *
 * \defgroup cdscommon Common
 * \addtogroup cdscommon
 * @{
 */

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifdef CDS_ENABLE_MTRACE
#include <mcheck.h>
#endif



/*----------------+
 | Types & Macros |
 +----------------*/


/** Panic macro */
#define CDSPANIC \
    do { \
        fprintf(stderr, "CDS PANIC at %s:%d\n", __FILE__, __LINE__); \
        abort(); \
    } while (0)


/** Panic macro with message */
#define CDSPANIC_MSG(_fmt, ...) \
    do { \
        fprintf(stderr, "CDS PANIC: "); \
        fprintf(stderr, (_fmt), ## __VA_ARGS__); \
        fprintf(stderr, " (at %s:%d)\n", __FILE__, __LINE__); \
        abort(); \
    } while (0)


/** Assert macro */
#define CDSASSERT(_cond) \
    do { \
        if (!(_cond)) { \
            fprintf(stderr, "CDS ASSERT: %s (at %s:%d)\n", #_cond, \
                    __FILE__, __LINE__); \
            abort(); \
        } \
    } while (0)



/*------------------------------+
 | Public function declarations |
 +------------------------------*/


/** Replacement for `malloc()`
 *
 * \param size_B [in] The number of bytes to allocate; must be > 0
 *
 * \return The allocated memory, never NULL
 */
void* CdsMalloc(size_t size_B);


/** Replacement for `malloc()`, and initialise memory to zero
 *
 * \param size_B [in] The number of bytes to allocate; must be > 0
 *
 * \return The allocated memory, never NULL
 */
void* CdsMallocZ(size_t size_B);


/* @} */
#endif /* CDSCOMMON_h_ */
