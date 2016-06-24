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

#ifndef CDSMAP_PRIVATE_h_
#define CDSMAP_PRIVATE_h_



/*----------------+
 | Types & Macros |
 +----------------*/


/* Forward declaration */
struct CdsMap;


/* Map item */
struct CdsMapItem
{
    struct CdsMapItem* parent;
    struct CdsMapItem* left;
    struct CdsMapItem* right;
    void*              key;
    int8_t             factor;
    uint8_t            flags;
};



#endif /* CDSMAP_PRIVATE_h_ */
