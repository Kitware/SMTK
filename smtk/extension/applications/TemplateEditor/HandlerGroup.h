//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __HandlerGroup_h
#define __HandlerGroup_h
#include "HandlerItemDef.h"

namespace Ui
{
class ItemDefGroupForm;
}

/**
 * \brief Generates a custom UI for a GroupItemDefinition instance.
 */
class HandlerGroup : public HandlerItemDef
{
public:
  HandlerGroup();
  ~HandlerGroup();

private:
  HandlerGroup(const HandlerGroup&) = delete;
  void operator=(const HandlerGroup&) = delete;

  smtk::attribute::ItemDefinitionPtr createItemDef_impl(const std::string& name) override;
  smtk::attribute::ItemDefinitionPtr updateItemDef_impl() override;
  bool initialize_impl(smtk::attribute::ItemDefinitionPtr def, QWidget* parent) override;

  std::unique_ptr<Ui::ItemDefGroupForm> Ui;
};
#endif // __HandlerGroup_h
