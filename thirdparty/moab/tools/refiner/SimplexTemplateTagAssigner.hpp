/*
 * MOAB, a Mesh-Oriented datABase, is a software component for creating,
 * storing and accessing finite element mesh data.
 * 
 * Copyright 2007 Sandia Corporation.  Under the terms of Contract
 * DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government
 * retains certain rights in this software.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 */

/**\class moab::SimplexTemplateTagAssigner
  *
  * This is an class that embodies the process of assigning tag
  * values to new vertices based on some pre-existing neighbors in a 
  * simplicial mesh.
  *
  * \author David Thompson
  * \author Philippe Pebay
  *
  * \date 28 December 2007
  */
#ifndef MOAB_SIMPEX_TEMPLATE_TAG_ASSIGNER_HPP
#define MOAB_SIMPEX_TEMPLATE_TAG_ASSIGNER_HPP

#include "moab/Compiler.hpp" // for MB_DLL_EXPORT
#include "moab/Types.hpp"

namespace moab {

class RefinerTagManager;
class SimplexTemplateRefiner;

class SimplexTemplateTagAssigner
{
public:
  SimplexTemplateTagAssigner( SimplexTemplateRefiner* );
  virtual ~SimplexTemplateTagAssigner();
  
  virtual void operator () ( const double* c0, const void* t0, EntityHandle h0,
                             const double* cm, void* tm,
                             const double* c1, const void* t1, EntityHandle h1 );
  virtual void operator () ( const void* t0,
                             const void* t1,
                             const void* t2,
                             void* tp );
  virtual void set_tag_manager( RefinerTagManager* tmgr );

protected:
  SimplexTemplateRefiner* mesh_refiner;
  RefinerTagManager* tag_manager;
};

} // namespace moab

#endif // MOAB_SIMPEX_TEMPLATE_TAG_ASSIGNER_HPP
