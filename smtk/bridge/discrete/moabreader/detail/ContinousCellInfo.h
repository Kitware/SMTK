//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smoab_detail_ContinousCellInfo_h
#define __smoab_detail_ContinousCellInfo_h


namespace smoab { namespace detail {

struct ContinousCellInfo
{
  int type;
  int numVerts;
  int numUnusedVerts;
  int numCells;

};

} } //namespace smoab::detail

#endif
