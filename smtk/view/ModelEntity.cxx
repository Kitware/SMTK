//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "smtk/view/ModelEntity.h"
using namespace smtk::view;

//----------------------------------------------------------------------------
ModelEntity::ModelEntity(const std::string &myTitle):
  Base(myTitle), m_modelEntityMask(0)
{
}

//----------------------------------------------------------------------------
ModelEntity::~ModelEntity()
{
}
//----------------------------------------------------------------------------
Base::Type ModelEntity::type() const
{
  return MODEL_ENTITY;
}
