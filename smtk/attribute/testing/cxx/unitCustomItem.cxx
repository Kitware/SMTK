//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/CustomItem.h"
#include "smtk/attribute/CustomItemDefinition.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/DoubleItemDefinition.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/attribute/json/jsonItem.h"
#include "smtk/attribute/json/jsonItemDefinition.h"

#include "smtk/attribute/ItemDefinitionManager.h"
#include "smtk/attribute/Registrar.h"
#include "smtk/attribute/operators/Export.h"
#include "smtk/attribute/operators/Import.h"
#include "smtk/attribute/operators/Read.h"
#include "smtk/attribute/operators/Write.h"

#include "smtk/resource/Manager.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/Registrar.h"

#include "smtk/plugin/Registry.h"

#include "smtk/io/AttributeReader.h"
#include "smtk/io/AttributeWriter.h"
#include "smtk/io/Logger.h"

#include <complex>
#include <fstream>
#include <iostream>
#include <utility>

#include "smtk/common/testing/cxx/helpers.h"

#include "nlohmann/json.hpp"

#define PUGIXML_HEADER_ONLY
// NOLINTNEXTLINE(bugprone-suspicious-include)
#include "pugixml/src/pugixml.cpp"

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
using namespace boost::filesystem;

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

namespace
{
void cleanup(const std::string& file_path)
{
  //first verify the file exists
  ::boost::filesystem::path path(file_path);
  if (::boost::filesystem::is_regular_file(path))
  {
    //remove the file_path if it exists.
    ::boost::filesystem::remove(path);
  }
}
} // namespace

namespace
{
class MyItem : public smtk::attribute::CustomItem<MyItem>
{
  friend class smtk::attribute::CustomItemDefinition<MyItem>;

public:
  smtkTypeMacro(MyItem);
  ~MyItem() override = default;

  const std::complex<double>& value() const { return m_value; }
  bool setValue(const std::complex<double>& v);

protected:
  MyItem(smtk::attribute::Attribute* owningAttribute, int itemPosition)
    : smtk::attribute::CustomItem<MyItem>(owningAttribute, itemPosition)
  {
  }

  MyItem(smtk::attribute::Item* owningItem, int myPosition, int mySubGroupPosition)
    : smtk::attribute::CustomItem<MyItem>(owningItem, myPosition, mySubGroupPosition)
  {
  }

  bool isValidInternal(bool useCategories, const std::set<std::string>& categories) const override
  {
    (void)useCategories;
    (void)categories;
    return true;
  }

  const MyItem& operator>>(nlohmann::json& json) const override
  {
    std::shared_ptr<smtk::attribute::Item> tmp(const_cast<MyItem*>(this), [](MyItem*) {});
    smtk::attribute::to_json(json, tmp);

    json["Real"] = m_value.real();
    json["Imag"] = m_value.imag();

    return *this;
  }
  MyItem& operator<<(const nlohmann::json& json) override
  {
    std::shared_ptr<smtk::attribute::Item> tmp(this, [](MyItem*) {});
    smtk::attribute::from_json(json, tmp);
    m_value = std::complex<double>(json.at("Real").get<double>(), json.at("Imag").get<double>());

    return *this;
  }

  const MyItem& operator>>(pugi::xml_node& node) const override
  {
    node.append_attribute("Real").set_value(m_value.real());
    node.append_attribute("Imag").set_value(m_value.imag());

    return *this;
  }
  MyItem& operator<<(const pugi::xml_node& node) override
  {
    m_value =
      std::complex<double>(node.attribute("Real").as_double(), node.attribute("Imag").as_double());

    return *this;
  }

private:
  std::complex<double> m_value;
};

class MyItemDefinition : public smtk::attribute::CustomItemDefinition<MyItem>
{
public:
  smtkTypeMacro(MyItemDefinition);

  static Ptr New(const std::string& myName) { return std::make_shared<MyItemDefinition>(myName); }

  ~MyItemDefinition() override = default;

  MyItemDefinition(const std::string& myName)
    : CustomItemDefinition(myName)
    , m_phaseRange(0., 2. * M_PI)
  {
  }

  MyItemDefinition(MyItemDefinition&&) = default;

  [[nodiscard]] bool isValueValid(const std::complex<double>& val) const
  {
    double phase = std::arg(val);
    return (phase >= m_phaseRange.first && phase <= m_phaseRange.second);
  }

  [[nodiscard]] const std::pair<double, double>& phaseRange() const { return m_phaseRange; }
  void setPhaseRange(const std::pair<double, double>& range) { m_phaseRange = range; }

  smtk::attribute::ItemDefinition::Ptr createCopy(
    smtk::attribute::ItemDefinition::CopyInfo& info) const override;

  const MyItemDefinition& operator>>(nlohmann::json& json) const override
  {
    std::shared_ptr<smtk::attribute::ItemDefinition> tmp(
      const_cast<MyItemDefinition*>(this), [](MyItemDefinition*) {});
    smtk::attribute::to_json(json, tmp);

    json["PhaseMin"] = m_phaseRange.first;
    json["PhaseMax"] = m_phaseRange.second;

    return *this;
  }
  MyItemDefinition& operator<<(const nlohmann::json& json) override
  {
    std::shared_ptr<smtk::attribute::ItemDefinition> tmp(this, [](MyItemDefinition*) {});
    smtk::attribute::from_json(json, tmp);
    this->setPhaseRange(std::make_pair(json.at("PhaseMin"), json.at("PhaseMax")));

    return *this;
  }

  const MyItemDefinition& operator>>(pugi::xml_node& node) const override
  {
    node.append_attribute("PhaseMin").set_value(m_phaseRange.first);
    node.append_attribute("PhaseMax").set_value(m_phaseRange.second);

    return *this;
  }
  MyItemDefinition& operator<<(const pugi::xml_node& node) override
  {
    m_phaseRange = std::make_pair(
      node.attribute("PhaseMin").as_double(), node.attribute("PhaseMax").as_double());

    return *this;
  }

private:
  std::pair<double, double> m_phaseRange;
};

smtk::attribute::ItemDefinition::Ptr MyItemDefinition::createCopy(
  smtk::attribute::ItemDefinition::CopyInfo& info) const
{
  (void)info;

  MyItemDefinition::Ptr instance = std::make_shared<MyItemDefinition>(this->name());
  instance->setPhaseRange(this->phaseRange());
  ItemDefinition::copyTo(instance);
  return instance;
}

bool MyItem::setValue(const std::complex<double>& v)
{
  const MyItemDefinition* def = static_cast<const MyItemDefinition*>(this->definition().get());
  if (def->isValueValid(v))
  {
    m_value = v;
    return true;
  }
  return false;
}

typedef std::tuple<MyItemDefinition> CustomItemDefinitionsList;
} // namespace

int unitCustomItem(int /*unused*/, char* /*unused*/[])
{
  auto managers = smtk::common::Managers::create();

  // Construct smtk managers
  auto resourceRegistry = smtk::plugin::addToManagers<smtk::resource::Registrar>(managers);
  auto operationRegistry = smtk::plugin::addToManagers<smtk::operation::Registrar>(managers);
  auto attributeRegistry = smtk::plugin::addToManagers<smtk::attribute::Registrar>(managers);

  // access smtk managers
  auto resourceManager = managers->get<smtk::resource::Manager::Ptr>();
  auto operationManager = managers->get<smtk::operation::Manager::Ptr>();
  auto itemDefinitionManager = managers->get<smtk::attribute::ItemDefinitionManager::Ptr>();

  // Initialize smtk managers
  auto attributeOpRegistry =
    smtk::plugin::addToManagers<smtk::attribute::Registrar>(operationManager);
  {
    operationManager->registerResourceManager(resourceManager);
    itemDefinitionManager->registerDefinitions<CustomItemDefinitionsList>();
  }

  // 1. Construct an attribute with a custom item

  smtk::attribute::Resource::Ptr resource = smtk::attribute::Resource::create();
  resource->customItemDefinitionFactory().registerType<MyItemDefinition>();

  smtk::attribute::Definition::Ptr def = resource->createDefinition("testDef");

  MyItemDefinition::Ptr itemDef = MyItemDefinition::New("myItem");
  itemDef->setPhaseRange(std::make_pair(M_PI / 2., 3. * M_PI / 2.));
  def->addItemDefinition(itemDef);

  smtk::attribute::DoubleItemDefinition::Ptr doubleItemDef =
    smtk::attribute::DoubleItemDefinition::New("myDoubleItem");
  def->addItemDefinition(doubleItemDef);

  smtk::attribute::Attribute::Ptr attribute = resource->createAttribute(def);

  MyItem::Ptr myItem = attribute->findAs<MyItem>("myItem");
  myItem->setValue(std::complex<double>(0., 1.));

  std::string write_root(SMTK_SCRATCH_DIR);
  std::string unique_str = smtk::common::UUID::random().toString();

  // 2. Write the resource out to a new .sbi file (to ensure it is of the
  //    latest format)

  std::string sbi1FileName;
  {
    std::stringstream s;
    s << write_root << "/"
      << "originalResource_" << unique_str << ".sbi";
    sbi1FileName = s.str();

    smtk::attribute::Export::Ptr exporter = operationManager->create<smtk::attribute::Export>();
    exporter->parameters()->associate(resource);
    exporter->parameters()->findFile("filename")->setValue(sbi1FileName);

    auto result = exporter->operate();
    if (
      result->findInt("outcome")->value() !=
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
    {
      std::cerr << "Export operator failed\n";
      std::cerr << exporter->log().convertToString(true) << "\n";
      return -2;
    }
  }

  // 3. Import the resource in from the .sbi file

  {
    smtk::attribute::Import::Ptr importer = operationManager->create<smtk::attribute::Import>();
    importer->parameters()->findFile("filename")->setValue(sbi1FileName);

    auto result = importer->operate();
    if (
      result->findInt("outcome")->value() !=
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
    {
      std::cerr << "Import operator failed\n";
      std::cerr << importer->log().convertToString(true) << "\n";
      return -2;
    }

    resource = std::dynamic_pointer_cast<smtk::attribute::Resource>(
      result->findResource("resourcesCreated")->value());
  }

  // 4. Write the resource out to a .smtk file

  std::string smtkFileName;
  {
    std::stringstream s;
    s << write_root << "/"
      << "jsonResource_" << unique_str << ".smtk";
    smtkFileName = s.str();
    resource->setLocation(smtkFileName);

    smtk::attribute::Write::Ptr writeOp = operationManager->create<smtk::attribute::Write>();
    writeOp->parameters()->associate(resource);
    auto result = writeOp->operate();

    if (
      result->findInt("outcome")->value() !=
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
    {
      std::cerr << "Write operation failed\n";
      std::cerr << writeOp->log().convertToString();
      return -2;
    }
  }

  // 5. Read the .smtk file into a new resource

  smtk::attribute::Resource::Ptr copiedResource;
  {
    smtk::attribute::Read::Ptr readOp = operationManager->create<smtk::attribute::Read>();
    readOp->parameters()->findFile("filename")->setValue(smtkFileName);
    auto result = readOp->operate();

    if (
      result->findInt("outcome")->value() !=
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
    {
      std::cerr << "Read operation failed\n";
      std::cerr << readOp->log().convertToString();
      cleanup(smtkFileName);
      return -2;
    }

    copiedResource = std::dynamic_pointer_cast<smtk::attribute::Resource>(
      result->findResource("resourcesCreated")->value());

    // Because both the original and the copied resources are managed by the
    // same manager, the copied resource will be forced to have a new id. Let's
    // remove the copied resource from the manager and set the UUID to the value
    // of the original resource.
    resourceManager->remove(copiedResource);
    copiedResource->setId(resource->id());
  }

  // 6. Write the new resource to a .sbi file

  std::string sbi2FileName;
  {
    // Removing the copied resource location in order to match the original .sbi
    copiedResource->setLocation("");
    // Resolving links to update Surrogate instances
    copiedResource->links().resolve(copiedResource);

    std::stringstream s;
    s << write_root << "/"
      << "jsonResource_" << unique_str << ".sbi";
    sbi2FileName = s.str();

    smtk::io::Logger logger;
    smtk::io::AttributeWriter writer;
    if (writer.write(copiedResource, sbi2FileName, logger))
    {
      std::cerr << "Encountered Errors while writing " << sbi2FileName << "\n";
      std::cerr << logger.convertToString();
      cleanup(sbi2FileName);
      return -2;
    }
  }

  // 7. Compare the .sbi files generated in steps 2 and 6

  int status = 0;
  {
    std::ifstream f1(sbi1FileName, std::ifstream::binary | std::ifstream::ate);
    std::ifstream f2(sbi2FileName, std::ifstream::binary | std::ifstream::ate);

    if (f1.fail() || f2.fail())
    {
      std::cerr << "Could not read in original or generated file\n\n";
      std::cerr << "generated smtk file: " << smtkFileName << "\n";
      std::cerr << "generated sbi1 file: " << sbi1FileName << "\n";
      std::cerr << "generated sbi2 file: " << sbi2FileName << "\n";
      status = -1;
    }

    if (f1.tellg() != f2.tellg())
    {
      std::cerr << "Original and generated file sizes are different\n\n";
      std::cerr << "generated smtk file: " << smtkFileName << "\n";
      std::cerr << "generated sbi1 file: " << sbi1FileName << "\n";
      std::cerr << "generated sbi2 file: " << sbi2FileName << "\n";
      status = -1;
    }

    f1.seekg(0, std::ifstream::beg);
    f2.seekg(0, std::ifstream::beg);
    if (!std::equal(
          std::istreambuf_iterator<char>(f1.rdbuf()),
          std::istreambuf_iterator<char>(),
          std::istreambuf_iterator<char>(f2.rdbuf())))
    {
      std::cerr << "Original and generated files are different\n\n";
      std::cerr << "generated smtk file: " << smtkFileName << "\n";
      std::cerr << "generated sbi1 file: " << sbi1FileName << "\n";
      std::cerr << "generated sbi2 file: " << sbi2FileName << "\n";
      status = -1;
    }
  }

  // Clean up all generated files
  cleanup(smtkFileName);
  cleanup(sbi1FileName);
  cleanup(sbi2FileName);
  if (status != 0)
  {
    return status;
  }

  itemDefinitionManager->unregisterDefinitions<CustomItemDefinitionsList>();

  return 0;
}
