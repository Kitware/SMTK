//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================


#ifndef __smtk_mesh_json_Readers_h
#define __smtk_mesh_json_Readers_h

#include "smtk/PublicPointerDefs.h"

#ifndef SHIBOKEN_SKIP
#  include "cJSON.h"
#endif // SHIBOKEN_SKIP

namespace smtk {
namespace mesh {
namespace json
{

smtk::mesh::CollectionPtr import(cJSON* child,
                                 const smtk::mesh::ManagerPtr& manager);

//Merge the entire json data stream to the collection creating a lightweight
//collection view, which uses the json backend interface
bool import(cJSON* json, const smtk::mesh::CollectionPtr& c);

}
}
}

#endif
