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
 * Concrete classes handle UI logic for each of the different configurations
 * and create/modify ItemDefinitions.
 */
class HandlerItemDef
{
public:
  using ItemDefPtr = smtk::attribute::ItemDefinitionPtr;

  /**
   * Factory method. Maps attribute::Item::Type to corresponding Handler.
   */
  static std::shared_ptr<HandlerItemDef> create(const int type);

  bool initialize(ItemDefPtr itemDef, QWidget* parent);

  /**
   * Returns the current ItemDefinition. If no ItemDef was set, it creates
   * a new one of the type defined when initialized.
   */
  ItemDefPtr updateItemDef(const std::string& name);

protected:
  HandlerItemDef();
  ~HandlerItemDef();

  ItemDefPtr createItemDef(const std::string& name);

  ItemDefPtr ItemDef;

private:
  HandlerItemDef(const HandlerItemDef&) = delete;
  void operator=(const HandlerItemDef&) = delete;

  virtual ItemDefPtr updateItemDef_impl() = 0;
  virtual bool initialize_impl(QWidget* parent) = 0;
  virtual ItemDefPtr createItemDef_impl(const std::string& name) = 0;
};
#endif // __HandlerItemDef_h
