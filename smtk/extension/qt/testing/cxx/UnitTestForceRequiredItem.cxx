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
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/extension/qt/qtViewRegistrar.h"
#include "smtk/io/AttributeReader.h"
#include "smtk/io/Logger.h"
#include "smtk/plugin/Registry.h"
#include "smtk/view/Manager.h"
#include "smtk/view/Registrar.h"

#include "smtk/common/testing/cxx/helpers.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QWidget>

#include <cassert>
#include <string>

const char* templateString = "<SMTK_AttributeResource Version=\"4\">\n"
                             "  <Definitions>\n"
                             "    <AttDef Type=\"test\">\n"
                             "      <ItemDefinitions>\n"
                             "        <Double Name=\"ditem\" Optional=\"true\">\n"
                             "        </Double>\n"
                             "      </ItemDefinitions>\n"
                             "    </AttDef>\n"
                             "  </Definitions>\n"
                             "  <Views>\n"
                             "    <View Type=\"Instanced\" Name=\"Test\" TopLevel=\"true\">\n"
                             "      <InstancedAttributes>\n"
                             "        <Att Name=\"test\" Type=\"test\" />\n"
                             "      </InstancedAttributes>\n"
                             "    </View>\n"
                             "  </Views>\n"
                             "</SMTK_AttributeResource>\n";

int UnitTestForceRequiredItem(int argc, char** const argv)
{
  QApplication app(argc, argv);

  // Initialize attribute resource and load template string
  auto attResource = smtk::attribute::Resource::create();
  smtk::io::AttributeReader sbtReader;
  smtk::io::Logger logger;
  bool err = sbtReader.readContents(attResource, templateString, logger);
  test(!err, "Failed to read contents as an attribute resource.");
  assert(!err);

  // Create "test" attribute
  const std::string attName = "test";
  const std::string attType = "test";
  auto defn = attResource->findDefinition(attType);
  smtk::attribute::AttributePtr att = attResource->createAttribute(attName, attType);

  // Set forceRequired on double item
  auto dItem = att->findDouble("ditem");
  dItem->setForceRequired(true);

  // Initialize view and UI managers
  auto viewManager = smtk::view::Manager::create();
  auto viewRegistry = smtk::plugin::addToManagers<smtk::view::Registrar>(viewManager);
  auto qtViewRegistry = smtk::plugin::addToManagers<smtk::extension::qtViewRegistrar>(viewManager);
  smtk::extension::qtUIManager* uiManager = new smtk::extension::qtUIManager(attResource);
  uiManager->setViewManager(viewManager);

  // Instantiate widget as container for qtUIManager
  QWidget* widget = new QWidget();
  QVBoxLayout* layout = new QVBoxLayout();
  widget->setLayout(layout);

  // Create instanced view
  auto instancedView = attResource->findTopLevelView();
  uiManager->setSMTKView(instancedView, widget);

  return 0;
}
