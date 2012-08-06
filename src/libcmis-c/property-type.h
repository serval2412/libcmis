/* libcmis
 * Version: MPL 1.1 / GPLv2+ / LGPLv2+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2011 SUSE <cbosdonnat@suse.com>
 *
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPLv2+"), or
 * the GNU Lesser General Public License Version 2 or later (the "LGPLv2+"),
 * in which case the provisions of the GPLv2+ or the LGPLv2+ are applicable
 * instead of those above.
 */
#ifndef _LIBCMIS_PROPERTY_TYPE_H_
#define _LIBCMIS_PROPERTY_TYPE_H_

typedef struct libcmis_property_type* libcmis_PropertyTypePtr;

enum libcmis_property_type_Type
{
    libcmis_String,
    libcmis_Integer,
    libcmis_Decimal,
    libcmis_Bool,
    libcmis_DateTime
};

void libcmis_property_type_free( libcmis_PropertyTypePtr type );

const char* libcmis_property_type_getId( libcmis_PropertyTypePtr type );
const char* libcmis_property_type_getLocalName( libcmis_PropertyTypePtr type );
const char* libcmis_property_type_getLocalNamespace( libcmis_PropertyTypePtr type );
const char* libcmis_property_type_getDisplayName( libcmis_PropertyTypePtr type );
const char* libcmis_property_type_getQueryName( libcmis_PropertyTypePtr type );

libcmis_property_type_Type libcmis_property_type_getType( libcmis_PropertyTypePtr type );
const char* libcmis_property_type_getXmlType( libcmis_PropertyTypePtr type );

bool libcmis_property_type_isMultiValued( libcmis_PropertyTypePtr type );
bool libcmis_property_type_isUpdatable( libcmis_PropertyTypePtr type );
bool libcmis_property_type_isInherited( libcmis_PropertyTypePtr type );
bool libcmis_property_type_isRequired( libcmis_PropertyTypePtr type );
bool libcmis_property_type_isQueryable( libcmis_PropertyTypePtr type );
bool libcmis_property_type_isOrderable( libcmis_PropertyTypePtr type );
bool libcmis_property_type_isOpenChoice( libcmis_PropertyTypePtr type );

#endif