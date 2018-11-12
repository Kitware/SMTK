//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/session/rgg/operators/ReadRXFFile.h"

#include "smtk/session/rgg/Session.h"

#include "smtk/io/Logger.h"

#include "smtk/model/Model.h"
#include "smtk/model/Resource.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/session/rgg/ReadRXFFile_xml.h"
#include "smtk/session/rgg/operators/CreateModel.h"

#define PUGIXML_HEADER_ONLY
#include "pugixml/src/pugixml.cpp"

#include "smtk/session/rgg/operators/ReadRXFFileHelper.h"

#include <fstream>

using namespace smtk::model;
using namespace smtk::session::rgg::XMLAttribute;

namespace smtk
{
namespace session
{
namespace rgg
{

bool ReadRXFFile::ableToOperate()
{
  if (!this->parameters()->isValid())
  {
    return false;
  }

  std::string filename = this->parameters()->findFile("filename")->value();
  if (filename.empty())
  {
    return false;
  }
  return true;
}

ReadRXFFile::Result ReadRXFFile::operateInternal()
{
  Result result = this->createResult(smtk::operation::Operation::Outcome::FAILED);
  EntityRefArray entities = this->parameters()->associatedModelEntities<EntityRefArray>();
  if (entities.empty() || !entities[0].isModel())
  {
    smtkErrorMacro(this->log(), "An invalid model is provided for read rxf file op");
    return result;
  }

  // Verify the file
  smtk::model::EntityRef model = entities[0];

  smtk::attribute::FileItemPtr filenameItem = this->parameters()->findFile("filename");

  std::string filename = filenameItem->value();
  if (filename.empty())
  {
    smtkErrorMacro(this->log(), "A filename must be provided.\n");
    return result;
  }

  std::ifstream file(filename.c_str());
  if (!file.good())
  {
    smtkErrorMacro(this->log(), "Could not open file \"" << filename << "\".\n");
    return result;
  }

  std::string data((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));

  if (data.empty())
  {
    smtkErrorMacro(this->log(), "No XML objects in file\"" << filename << "\".\n");
    return result;
  }

  // Start parsing
  pugi::xml_document document;
  pugi::xml_parse_result presult = document.load_buffer(data.c_str(), data.size());
  if (presult.status != pugi::status_ok)
  {
    smtkErrorMacro(this->log(), "Cannot parse XML objects in file\"" << filename << "\".\n");
    return result;
  }
  pugi::xml_node rootElement = document.child(CORE_TAG.c_str());

  // Read materials and store the info on the model. Without doing so, smtk
  // is not able to assign the right material color
  {
    pugi::xml_node materialNode = rootElement.child(MATERIALS_TAG.c_str());
    if (!ReadRXFFileHelper::parseMaterial(materialNode, model))
    {
      smtkErrorMacro(this->log(), "Encounter errors when parsing materials.");
    }
  }

  // Read ducts and core properties (Geometry type, duct heights and pitch)
  // For now the reader should read ducts first so that
  // It can set the geometry type and size parameters.
  // FIXME: It's a hack in CMB5. In CMB6, we should allow user to directly
  // open RXF file without calling "create  model" op first. 'read rxf file' op
  // should be responsible for calling 'create model' op.
  // Newly created ducts and their sub parts
  smtk::model::EntityRefArray ducts;
  {
    if (!ReadRXFFileHelper::parseDuctsAndCoreProperty(rootElement, model, ducts))
    {
      smtkErrorMacro(this->log(), "Encounter errors when parsing ducts and core properties.");
    }
  }

  // Read pins
  pugi::xml_node pinsNode = rootElement.child(PINS_TAG.c_str());
  smtk::model::EntityRefArray pins;
  {
    if (!ReadRXFFileHelper::parsePins(pinsNode, model, pins))
    {
      smtkErrorMacro(this->log(), "Encounter errors when parsing pins.");
    }
  }

  // Create labelToPin and nameToDuct maps
  std::map<std::string, smtk::model::EntityRef> labelToPin;
  std::map<std::string, smtk::model::EntityRef> nameToDuct;
  for (auto pin : pins)
  {
    if (pin.hasStringProperty("label"))
    {
      labelToPin[pin.stringProperty("label")[0]] = pin;
    }
  }
  for (auto duct : ducts)
  {
    if (duct.hasFloatProperty("z values")) // Duct subparts does not have z values
    {
      nameToDuct[duct.name()] = duct;
    }
  }

  // Read assemblies
  // Since instances needed to be marked as "tess_changed",
  // we need to use a second array here
  smtk::model::EntityRefArray assys, assyInstances;
  {
    if (!ReadRXFFileHelper::parseAssemblies(
          rootElement, model, assys, assyInstances, labelToPin, nameToDuct))
    {
      smtkErrorMacro(this->log(), "Encounter errors when parsing assemblies.");
    }
  }

  // Parse defaults. It might override duct thickness, z origin and height
  {
    if (!ReadRXFFileHelper::parseDefaults(rootElement, model))
    {
      smtkErrorMacro(this->log(), "Encounter errors when parsing default properties.");
    }
  }

  // Read core
  smtk::model::EntityRefArray coreInstances;
  {
    std::map<std::string, smtk::model::EntityRef> labelToAssy;
    for (auto& assy : assys)
    {
      if (assy.hasStringProperty("label"))
      {
        labelToAssy[assy.stringProperty("label")[0]] = assy;
      }
    }
    if (!ReadRXFFileHelper::parseCore(rootElement, model, coreInstances, labelToAssy))
    {
      smtkErrorMacro(this->log(), "Encounter errors when parsing the core.");
    }
  }
  smtk::model::EntityRefArray coreArray =
    model.resource()->findEntitiesByProperty("rggType", SMTK_SESSION_RGG_CORE);
  if (coreArray.size() == 0)
  {
    smtkErrorMacro(this->log(), "An invalid core is provided for read rxf file op");
    return result;
  }

  result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  smtk::attribute::ComponentItem::Ptr createdItem = result->findComponent("created");
  for (auto& c : ducts)
  {
    createdItem->appendValue(c.component());
  }
  for (auto& c : pins)
  {
    createdItem->appendValue(c.component());
  }
  for (auto& c : assys)
  {
    createdItem->appendValue(c.component());
  }
  for (auto& c : assyInstances)
  {
    createdItem->appendValue(c.component());
  }
  for (auto& c : coreInstances)
  {
    createdItem->appendValue(c.component());
  }

  smtk::attribute::ComponentItem::Ptr modifiedItem = result->findComponent("modified");
  modifiedItem->appendValue(model.component());
  modifiedItem->appendValue(coreArray[0].component());

  return result;
}

const char* ReadRXFFile::xmlDescription() const
{
  return ReadRXFFile_xml;
}
} // namespace rgg
} //namespace session
} // namespace smtk
