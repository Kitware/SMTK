//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/operation/XMLOperation.h"

#include "smtk/attribute/Resource.h"

#include "smtk/io/AttributeReader.h"

namespace smtk
{
namespace operation
{

XMLOperation::XMLOperation()
  : Operation()
{
}

XMLOperation::~XMLOperation()
{
}

smtk::operation::XMLOperation::Specification XMLOperation::createSpecification()
{
  Specification spec = smtk::attribute::Resource::create();
  smtk::io::AttributeReader reader;
  reader.readContents(spec, this->xmlDescription(), this->log());

  return spec;
}

} // operation namespace
} // smtk namespace
