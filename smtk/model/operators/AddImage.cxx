//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/operators/AddImage.h"

#include "smtk/model/AddImage_xml.h"

smtkImplementsModelOperator(
  SMTKCORE_EXPORT,
  smtk::model::AddImage,
  add_image,
  "add image",
  AddImage_xml,
  smtk::model::Session);
