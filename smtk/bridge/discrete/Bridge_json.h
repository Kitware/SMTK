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
 * Normally this would be machine-generated from Bridge.json
 * using the smtk_bridge_json CMake macro, but this version
 * has been hand-edited to include preprocessor macros that
 * add and remove engines and filetypes as support for them
 * is known at compile time.
 */

static const char Bridge_json[] =
"{"
"  \"kernel\": \"discrete\","
"  \"engines\": ["
"    {"
"      \"name\": \"native\","
"      \"filetypes\": ["
"        \".cmb (Conceptual Model Builder)\""
"        ,\".vtk (Legacy vtk files)\""
#ifdef SMTK_BUILD_MOAB_READER
"        ,\".exo (Moab files)\""
#endif
"      ]"
"    }"
"  ]"
"}";
