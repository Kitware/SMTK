//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkPythonExporter.h"

#include "PythonExportGridInfo2D.h"
#include "PythonExportGridInfo3D.h"
#include "vtkDiscreteModelWrapper.h"
#include <vtkDiscreteModel.h>
#include <vtkModelGeneratedGridRepresentation.h>
#include <vtkObjectFactory.h>
#include <vtkPythonInterpreter.h>

#include <vtksys/SystemTools.hxx>

#include <smtk/attribute/Attribute.h>
#include <smtk/attribute/FileItem.h>
#include <smtk/attribute/Item.h>
#include <smtk/attribute/Manager.h>
#include <smtk/model/Model.h>

#include <smtk/util/AttributeReader.h>
#include <smtk/util/ExportSpec.h>
#include <smtk/util/Logger.h>

#include <algorithm>
#include <sstream>

namespace
{
// Description:
// Given the attributes serialized into contents, deserialize that
// data into manager.
void DeserializeSMTK(const char* contents, smtk::attribute::Manager& manager)
{
  smtk::util::AttributeReader xmlr;
  smtk::util::Logger logger;
  xmlr.readContents(manager, contents, logger);
  std::vector<smtk::attribute::DefinitionPtr> definitions;
  manager.findBaseDefinitions(definitions);
}
}

vtkStandardNewMacro(vtkPythonExporter);

vtkPythonExporter::vtkPythonExporter()
{
  this->OperateSucceeded = 0;
  this->Script = 0;
  this->PythonPath = 0;
  this->PythonExecutable = 0;
}

vtkPythonExporter::~vtkPythonExporter()
{
  this->SetScript(0);
  this->SetPythonPath(0);
  this->SetPythonExecutable(0);
}

void vtkPythonExporter::Operate(
  vtkDiscreteModelWrapper* modelWrapper, const char* smtkContents, const char* exportContents)
{
  if (!this->AbleToOperate(modelWrapper))
  {
    this->OperateSucceeded = 0;
    return;
  }

  // create the attributes from smtkContents
  smtk::attribute::Manager simManager;
  smtk::model::ModelPtr modelPtr(new smtk::model::Model);
  simManager.setRefModel(modelPtr);
  DeserializeSMTK(smtkContents, simManager);

  smtk::attribute::Manager exportManager;
  DeserializeSMTK(exportContents, exportManager);

  this->Operate(modelWrapper->GetModel(), simManager, exportManager);
}

void vtkPythonExporter::Operate(vtkDiscreteModelWrapper* modelWrapper, const char* smtkContents)
{
  if (!this->AbleToOperate(modelWrapper))
  {
    this->OperateSucceeded = 0;
    return;
  }

  // create the attributes from smtkContents
  smtk::attribute::Manager manager;
  smtk::model::ModelPtr modelPtr(new smtk::model::Model);
  manager.setRefModel(modelPtr);
  DeserializeSMTK(smtkContents, manager);

  // Create empty export manager
  smtk::attribute::Manager exportManager;

  this->Operate(modelWrapper->GetModel(), manager, exportManager);
}

template <class IN>
std::string to_hex_address(IN* ptr)
{
  std::stringstream ss;
  ss << std::hex << ptr;
  std::string address;
  ss >> address;
  if (address[0] == '0' && (address[1] == 'x' || address[1] == 'X'))
  {
    address = address.substr(2);
  }
  return address;
}

void vtkPythonExporter::Operate(vtkDiscreteModel* model, smtk::attribute::Manager& manager,
  smtk::attribute::Manager& exportManager)
{
  // Check that we have a python script
  if (!this->GetScript() || strcmp(this->GetScript(), "") == 0)
  {
    vtkWarningMacro("Cannot export - no python script specified");
    this->OperateSucceeded = 0;
    return;
  }

  // Set python executable if defined
  if (this->PythonExecutable)
  {
    vtkPythonInterpreter::SetProgramName(this->PythonExecutable);
  }

  // Prepend the paths defined in PythonPath to sys.path
  if (this->PythonPath)
  {
    std::string pathscript;
    pathscript += "import sys\n";
    std::vector<vtksys::String> paths;
    paths = vtksys::SystemTools::SplitString(this->PythonPath, ';');
    for (size_t cc = 0; cc < paths.size(); cc++)
    {
      if (!paths[cc].empty())
      {
        pathscript += "if not ";
        pathscript += paths[cc];
        pathscript += " in sys.path:\n";
        pathscript += "  sys.path.insert(0, ";
        pathscript += paths[cc];
        pathscript += ")\n";

        vtkPythonInterpreter::RunSimpleString(pathscript.c_str());
      }
    }
  }
  std::string path = vtksys::SystemTools::GetFilenamePath(this->Script);
  if (!path.empty())
  {
    std::string pathscript;
    pathscript += "import sys\n";
    pathscript += "if not ";
    pathscript += '"' + path + '"';
    pathscript += " in sys.path:\n";
    pathscript += "  sys.path.insert(0, ";
    pathscript += '"' + path + '"';
    pathscript += ")\n";
    vtkPythonInterpreter::RunSimpleString(pathscript.c_str());
  }

  // Initialize GridInfo object
  smtk::model::ModelPtr smtkModel = manager.refModel();
  PythonExportGridInfo* gridInfoRaw = 0x0;
  if (2 == model->GetModelDimension())
  {
    gridInfoRaw = new PythonExportGridInfo2D(model);
  }
  else if (3 == model->GetModelDimension())
  {
    gridInfoRaw = new PythonExportGridInfo3D(model);
  }
  smtk::shared_ptr<PythonExportGridInfo> gridInfo(gridInfoRaw);
  smtkModel->setGridInfo(gridInfo);
  // Get filename from model
  std::string name = model->GetFileName();
  if (name == "")
  {
    // If empty string, try filename from model's mesh
    const DiscreteMesh& mesh = model->GetMesh();
    name = mesh.GetFileName();
  }
  smtkModel->setNativeModelName(name);

  // Initialize ExportSpec object
  smtk::util::ExportSpec spec;
  spec.setSimulationAttributes(&manager);
  spec.setExportAttributes(&exportManager);
  spec.setAnalysisGridInfo(gridInfo);

  std::string runscript;
  std::string script = vtksys::SystemTools::GetFilenameWithoutExtension(this->Script);

  // Call the function
  // reload the script if it has already been imported
  runscript += "import sys\n";
  runscript += "if sys.modules.has_key('" + script + "'):\n";
  runscript += "  reload(" + script + ")\n";
  runscript += "else:\n";
  runscript += "  import " + script + "\n";
  runscript += "import smtk\n";

  std::string spec_address = to_hex_address(&spec);

  runscript += "spec = smtk.util.ExportSpec._InternalConverterDoNotUse_('" + spec_address + "')\n";
  runscript += script + ".ExportCMB(spec)\n";
  //std::cout << "\nPython script:\n" << runscript << std::endl;
  vtkPythonInterpreter::RunSimpleString(runscript.c_str());

  this->OperateSucceeded = 1;
}

bool vtkPythonExporter::AbleToOperate(vtkDiscreteModelWrapper* modelWrapper)
{
  if (!modelWrapper)
  {
    vtkErrorMacro("Passed in a null model wrapper.");
    return false;
  }
  if (!this->Script)
  {
    vtkErrorMacro("No Python script.");
    return false;
  }
  return true;
}

void vtkPythonExporter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "OperateSucceeded: " << this->OperateSucceeded << endl;
  os << indent << "Script: " << (this->Script ? this->Script : "(NULL)") << endl;
  os << indent << "PythonPath: " << (this->PythonPath ? this->PythonPath : "(NULL)") << endl;
}
