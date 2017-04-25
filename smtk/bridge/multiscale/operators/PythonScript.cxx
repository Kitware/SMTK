//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/multiscale/operators/PythonScript.h"

#include "smtk/bridge/multiscale/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/model/Group.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"

#include "vtkPythonInterpreter.h"

#include "boost/filesystem.hpp"

using namespace smtk::model;
using namespace smtk::common;
using namespace boost::filesystem;

namespace smtk {
  namespace bridge {
    namespace multiscale {

std::string
PythonScript::listToArgList(std::vector<std::string>& tokens)
{
  return this->specToArgList(smtk::attribute::AttributePtr(),tokens);
}

std::string
PythonScript::specToArgList(smtk::attribute::AttributePtr spec)
{
  std::vector<std::string> nullList;
  return this->specToArgList(spec,nullList);
}

std::string
PythonScript::specToArgList(smtk::attribute::AttributePtr spec,
                                    std::vector<std::string>& additionalTokens)
{
  std::stringstream preamble;
  preamble << "import sys\n";
  preamble << "sys.argv = [\"pythonscript\"";
  if (spec)
  {
    for (std::size_t i=0; i<spec->numberOfItems(); i++)
    {
      smtk::attribute::ItemPtr item =
        smtk::dynamic_pointer_cast<smtk::attribute::Item>(spec->item(i));

      if (!item)
      {
        continue;
      }

      switch (item->type())
      {
      case smtk::attribute::Item::DOUBLE:
      case smtk::attribute::Item::INT:
      case smtk::attribute::Item::STRING:
        {
          smtk::attribute::ValueItemPtr valueItem =
            smtk::dynamic_pointer_cast<smtk::attribute::ValueItem>(item);

          if (valueItem && valueItem->name() != "assign names")
          {
            preamble << ",\"--" << valueItem->name() << "\"";
            for (std::size_t j=0; j<valueItem->numberOfValues();j++)
            {
              preamble << ",\"" << valueItem->valueAsString(j) << "\"";
            }
          }
        }
        break;
      case smtk::attribute::Item::FILE:
        {
          smtk::attribute::FileItemPtr fileItem =
            smtk::dynamic_pointer_cast<smtk::attribute::FileItem>(item);

          if (fileItem)
          {
            preamble << ",\"--" << fileItem->name() << "\"";
            for (std::size_t j=0; j<fileItem->numberOfValues();j++)
            {
              preamble << ",\"" << fileItem->valueAsString(j) << "\"";
            }
          }
        }
        break;
      case smtk::attribute::Item::VOID:
        {
          smtk::attribute::VoidItemPtr voidItem =
            smtk::dynamic_pointer_cast<smtk::attribute::VoidItem>(item);

          if (voidItem)
          {
            preamble << ",\"--" << voidItem->name() << "\"";
          }
        }
        break;
      default:
        break;
      }
    }
  }

  for (std::size_t i=0;i<additionalTokens.size();i++)
  {
    preamble << ",\"" << additionalTokens.at(i) << "\"";
  }

  preamble << "]\n";

  return preamble.str();
}

smtk::model::OperatorResult
PythonScript::executePythonScript(std::string preamble,
                                          std::string pythonScript)
{
  if (vtkPythonInterpreter::IsInitialized())
  {
    vtkPythonInterpreter::Finalize();
  }

  vtkPythonInterpreter::Initialize();
  vtkPythonInterpreter::RunSimpleString(preamble.c_str());
  std::stringstream pipeline;
  pipeline << "execfile(\"" << pythonScript << "\")";
  vtkPythonInterpreter::RunSimpleString(pipeline.str().c_str());
  vtkPythonInterpreter::Finalize();

  smtk::model::OperatorResult result = this->createResult(
    smtk::model::OPERATION_SUCCEEDED);
  return result;
}

    } // namespace multiscale
  } //namespace bridge
} // namespace smtk
