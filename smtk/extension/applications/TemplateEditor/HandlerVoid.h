//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __HandlerVoid_h
#define __HandlerVoid_h
#include "HandlerItemDef.h"

/**
 * \brief Generates a custom UI for a VoidItemDefinition instance.
 */
class HandlerVoid : public HandlerItemDef
{
public:
  HandlerVoid();
  ~HandlerVoid();

private:
  HandlerVoid(const HandlerVoid&) = delete;
  void operator=(const HandlerVoid&) = delete;

  smtk::attribute::ItemDefinitionPtr createItemDef_impl(const std::string& name) override;
  smtk::attribute::ItemDefinitionPtr updateItemDef_impl() override;
  bool initialize_impl(smtk::attribute::ItemDefinitionPtr def, QWidget* parent) override;
};
#endif // __HandlerGroup_h
