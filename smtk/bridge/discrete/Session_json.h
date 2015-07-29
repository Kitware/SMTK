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
/*
 * Normally this would be machine-generated from Session.json
 * using the smtk_session_json CMake macro, but this version
 * has been hand-edited to include preprocessor macros that
 * add and remove engines and filetypes as support for them
 * is known at compile time.
 */

static const char Session_json[] =
"{"
"  \"kernel\": \"discrete\","
"  \"engines\": ["
"    {"
"      \"name\": \"native\","
"      \"filetypes\": ["
"        \".cmb (CMB Models)\""
"        ,\".vtk (Legacy VTK Files)\""
"        ,\".2dm (CMB Mesh Files)\""
"        ,\".3dm (CMB Mesh Files)\""
"        ,\".stl (STL Files)\""
#ifdef SMTK_ENABLE_MOAB_SUPPORT
"        ,\".h5m (Moab Files)\""
"        ,\".sat (Moab Files)\""
"        ,\".brep (Moab Files)\""
"        ,\".stp (Moab Files)\""
"        ,\".cub (Moab Files)\""
"        ,\".exo (Moab Files)\""
#endif
#ifdef SMTK_ENABLE_REMUS_SUPPORT
"        ,\".poly (Poly Files)\""
"        ,\".smesh (Surface Meshes)\""
"        ,\".map (Map Files)\""
"        ,\".shp (Shape Files)\""
#endif
"      ]"
"    }"
"  ]"
"}";
