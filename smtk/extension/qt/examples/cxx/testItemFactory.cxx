//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtAttributeItemWidgetFactory.h"
#include "smtk/extension/qt/qtAttributeRefItem.h"
#include "smtk/extension/qt/qtBaseView.h"
#include "smtk/extension/qt/qtFileItem.h"
#include "smtk/extension/qt/qtFileItem.h"
#include "smtk/extension/qt/qtGroupItem.h"
#include "smtk/extension/qt/qtInputsItem.h"
#include "smtk/extension/qt/qtMeshSelectionItem.h"
#include "smtk/extension/qt/qtModelEntityItem.h"
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/extension/qt/qtVoidItem.h"

#include "smtk/attribute/DirectoryItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/MeshSelectionItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/RefItem.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/StringItemDefinition.h"
#include "smtk/attribute/System.h"

#include "smtk/common/View.h"

#include "smtk/common/testing/cxx/helpers.h"

#include "smtk/PublicPointerDefs.h"

#include <QApplication>
#include <QWidget>

#include <iostream>

using namespace smtk::attribute;
using namespace smtk::extension;

static int numDeleted = 0;

// A "dummy" class to verify that creating/deleting factories works as required.
class testItemWidgetFactory : public qtAttributeItemWidgetFactory
{
public:
  virtual ~testItemWidgetFactory()
    {
    ++numDeleted;
    }

  virtual qtItem* createValueItemWidget(ValueItemPtr item, QWidget* p, qtBaseView* bview, Qt::Orientation orient)
    {
    // TODO: Need to create an attribute view and see that this gets called.
    return new qtInputsItem(smtk::dynamic_pointer_cast<ValueItem>(item), p, bview, orient);
    }

};

AttributePtr createAttribForTest(System& system)
{
  DefinitionPtr def = system.createDefinition("test def");
  StringItemDefinitionPtr stringChild =
    def->addItemDefinition<StringItemDefinitionPtr>("test string");

  AttributePtr att = system.createAttribute("testAtt", "test def");
  double color[] = {3,24,12,6};
  att->setColor(color);
  att->setAppliesToBoundaryNodes(true);
  att->setAppliesToInteriorNodes(true);
  return att;
}

int testLifecycle()
{
  qtAttribute::setItemWidgetFactory(
    new testItemWidgetFactory());
  test(numDeleted == 0, "Bad initial value for numDeleted.");
  qtAttribute::setItemWidgetFactory(
    new testItemWidgetFactory());
  test(numDeleted == 1, "Expected to delete the old test factory.");
  qtAttribute::setItemWidgetFactory(NULL);
  test(numDeleted == 2, "Expected to delete the new test factory.");

  // This should not crash even though the factory is null
  // (because a new default factory should be created on demand).
  smtk::attribute::System system;
  AttributePtr att = createAttribForTest(system);
  qtUIManager* mgr = new qtUIManager(system);
  QWidget* w = new QWidget;
  smtk::extension::ViewInfo vinfo(smtk::common::View::New("base", "test view"),
                                  w, mgr);
  qtBaseView* v = new qtBaseView(vinfo);
  qtAttribute* qatt = new qtAttribute(att, w, v);

  delete qatt;
  delete w;
  return 0;
}

int main(int argc, char* argv[])
{
  QApplication app(argc, argv);
  (void)argc;
  (void)argv;
  // Verify that widget factory lifecycle management is done properly.
  return testLifecycle();
}
