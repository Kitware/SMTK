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
  "  \"kernel\": \"cgm\","
  "  \"engines\": ["
#if defined(HAVE_OCC)
  "    {"
  "      \"name\": \"OpenCascade\","
  "      \"filetypes\": ["
  "        \".brep (OpenCascade Boundary Representation)\""
  "        ,\".occ (OpenCascade Boundary Representation)\""
#if defined(HAVE_OCC_IGES)
  "        ,\".iges (Initial Graphics Exchange Specification)\""
  "        ,\".igs (Initial Graphics Exchange Specification)\""
#endif
#if defined(HAVE_OCC_STEP)
  "        ,\".step (Standard for the Exchange of Product model data)\""
  "        ,\".stp (Standard for the Exchange of Product model data)\""
#endif
#if defined(HAVE_OCC_STL)
  "        ,\".stl (STereoLithography file)\""
#endif
  "      ]"
  "    },"
#endif
  "    {"
  "      \"name\": \"Cholla\","
  "      \"filetypes\": ["
  "        \".off (Object File Format)\","
  "        \".cholla (Cholla facet file)\""
  "      ]"
  "    }"
  "  ]"
  "}";
