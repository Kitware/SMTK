//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/session/opencascade/operators/Import.h"
#include "smtk/session/opencascade/Resource.h"
#include "smtk/session/opencascade/Session.h"
#include "smtk/session/opencascade/Shape.h"

#include "smtk/operation/MarkGeometry.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/resource/Manager.h"

#include "smtk/io/Logger.h"

#include "smtk/common/Paths.h"

#include "smtk/session/opencascade/Import_xml.h"

#include "BRepTools.hxx"
#include "BRep_Builder.hxx"
#include "IGESCAFControl_Reader.hxx"
#include "STEPCAFControl_Reader.hxx"
#include "TopoDS_Compound.hxx"
#include "TopoDS_Iterator.hxx"
#include "TopoDS_Shape.hxx"
#include "Transfer_TransientProcess.hxx"
#include "XCAFDoc_DocumentTool.hxx"
#include "XCAFDoc_ShapeTool.hxx"
#include "XSControl_TransferReader.hxx"
#include "XSControl_WorkSession.hxx"

#include <algorithm>
#include <cctype>
#include <mutex>
#include <string>

namespace
{

void clearSession(const ::opencascade::handle<XSControl_WorkSession>& session)
{
  if (session.IsNull())
  {
    return;
  }

  ::opencascade::handle<Transfer_TransientProcess> mapReader =
    session->TransferReader()->TransientProcess();
  if (!mapReader.IsNull())
  {
    mapReader->Clear();
  }

  ::opencascade::handle<XSControl_TransferReader> transferReader = session->TransferReader();
  if (!transferReader.IsNull())
  {
    transferReader->Clear(1);
  }
}
}

namespace smtk
{
namespace session
{
namespace opencascade
{

template <typename T>
using handle = ::opencascade::handle<T>;

Import::Result Import::operateInternal()
{
  smtk::session::opencascade::Resource::Ptr resource;
  smtk::session::opencascade::Session::Ptr session;

  auto result = this->createResult(Import::Outcome::FAILED);

  this->prepareResourceAndSession(result, resource, session);

  smtk::attribute::FileItem::Ptr filenameItem = this->parameters()->findFile("filename");
  std::string filename = filenameItem->value(0);
  if (resource->location().empty())
  {
    resource->setLocation(filename);
  }
  std::string potentialName = smtk::common::Paths::stem(filename);
  if (resource->name().empty() && !potentialName.empty())
  {
    resource->setName(potentialName);
  }

  std::string filetype;
  std::string extension = smtk::common::Paths::extension(filename);
  std::transform(filetype.begin(), filetype.end(), filetype.begin(), ::tolower);
  if (extension == ".step" || extension == ".stp")
  {
    filetype = "step";
  }
  else if (extension == ".iges" || extension == ".igs")
  {
    filetype = "iges";
  }
  else
  {
    filetype = "occ";
  }

  auto created = result->findComponent("created");

  BRep_Builder builder;
  TopoDS_Shape& shape = resource->compound();
  if (filetype == "occ")
  {
    auto ok = BRepTools::Read(shape, filename.c_str(), builder);
    if (!ok)
    {
      return result; // Failure
    }
  }
  else if (filetype == "step")
  {
    STEPCAFControl_Reader reader;
    Handle(XSControl_WorkSession) wsess = reader.Reader().WS();

    try
    {
      if (!reader.ReadFile(filename.c_str()))
      {
        clearSession(wsess);
        return result;
      }

      if (!reader.Transfer(session->document()))
      {
        clearSession(wsess);
        return result;
      }

      clearSession(wsess);
    }
    catch (Standard_Failure e)
    {
      std::string msg = e.GetMessageString();
      smtkErrorMacro(
        this->log(), "Exception raised during STEP import: " << msg << " (" << filename << ")");
      return result;
    }

    auto shapeTool = XCAFDoc_DocumentTool::ShapeTool(session->document()->Main());
    TDF_LabelSequence labels;
    shapeTool->GetFreeShapes(labels);
    TopoDS_Compound compound;
    builder.MakeCompound(compound);
    for (Standard_Integer labelIter = 1; labelIter <= labels.Length(); ++labelIter)
    {
      TopoDS_Shape child;
      const TDF_Label& childLabel = labels.Value(labelIter);
      if (XCAFDoc_ShapeTool::GetShape(childLabel, child))
      {
        builder.Add(compound, child);
      }
    }
    shape = compound;
  }
  else if (filetype == "iges")
  {
    IGESCAFControl_Reader reader;
    Handle(XSControl_WorkSession) wsess = reader.WS();

    try
    {
      if (!reader.ReadFile(filename.c_str()))
      {
        clearSession(wsess);
        return result;
      }

      if (!reader.Transfer(session->document()))
      {
        clearSession(wsess);
        return result;
      }

      clearSession(wsess);
    }
    catch (Standard_Failure e)
    {
      std::string msg = e.GetMessageString();
      smtkErrorMacro(
        this->log(), "Exception raised during IGES import: " << msg << " (" << filename << ")");
      return result;
    }

    auto shapeTool = XCAFDoc_DocumentTool::ShapeTool(session->document()->Main());
    TDF_LabelSequence labels;
    shapeTool->GetFreeShapes(labels);
    TopoDS_Compound compound;
    builder.MakeCompound(compound);
    for (Standard_Integer labelIter = 1; labelIter <= labels.Length(); ++labelIter)
    {
      TopoDS_Shape child;
      const TDF_Label& childLabel = labels.Value(labelIter);
      if (XCAFDoc_ShapeTool::GetShape(childLabel, child))
      {
        builder.Add(compound, child);
      }
    }
    shape = compound;
  }
  else
  {
    smtkErrorMacro(this->log(), "Unknown file type for \"" << filename << "\".");
    return result;
  }

  auto topNode = this->createNode(shape, resource.get(), true);
  // std::cout
  //   << "  Added " << " " << topNode->id() << " " << topNode->name()
  //   << " " << TopAbs::ShapeTypeToString(shape.ShapeType()) << "\n";

  // NB: This visits shared children multiple times (e.g., vertex bounding 2+ edges)
  this->iterateChildren(*topNode, result);

  result->findInt("outcome")->setValue(static_cast<int>(Import::Outcome::SUCCEEDED));
  return result;
}

const char* Import::xmlDescription() const
{
  return Import_xml;
}

} // namespace opencascade
} // namespace session
} // namespace smtk
