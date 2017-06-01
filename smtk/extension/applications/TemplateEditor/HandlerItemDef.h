//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __HandlerItemDef_h
#define __HandlerItemDef_h
#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/Item.h"

class QWidget;

/**
 * \brief Generates a custom UI for each concrete ItemDef.
 *
 * Concrete classes handles UI logic for each of the different configurations
 * and create/modify ItemDefinitions.
 */
class HandlerItemDef
{
public:
  HandlerItemDef();
  ~HandlerItemDef();

  bool initialize(smtk::attribute::ItemDefinitionPtr itemDef, QWidget* parent);

  /**
   * Returns the current ItemDefinition. If no ItemDef was set, it creates
   * a new one of the type defined when initialized.
   */
  smtk::attribute::ItemDefinitionPtr updateItemDef(const std::string& name);

  smtk::attribute::ItemDefinitionPtr createItemDef(const std::string& name);

protected:
  smtk::attribute::ItemDefinitionPtr ItemDef;

private:
  HandlerItemDef(const HandlerItemDef&) = delete;
  void operator=(const HandlerItemDef&) = delete;

  virtual smtk::attribute::ItemDefinitionPtr updateItemDef_impl() = 0;
  virtual bool initialize_impl(smtk::attribute::ItemDefinitionPtr def, QWidget* parent) = 0;
  virtual smtk::attribute::ItemDefinitionPtr createItemDef_impl(const std::string& name) = 0;
};
#endif // __HandlerItemDef_h
