//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/operators/ExportSMTKModel.h"

#include "smtk/model/ExportSMTKModel_xml.h"

namespace smtk
{
namespace model
{

ExportSMTKModel::ExportSMTKModel()
{
}

const char* ExportSMTKModel::xmlDescription() const
{
  return ExportSMTKModel_xml;
}

} //namespace model
} // namespace smtk
