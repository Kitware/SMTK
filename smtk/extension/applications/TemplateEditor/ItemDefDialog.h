//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __ItemDefDialog_h
#define __ItemDefDialog_h

#include <memory>

#include "smtk/PublicPointerDefs.h"

#include "InputDialog.h"

namespace Ui
{
class ItemDefDialog;
}

class HandlerItemDef;

/**
 * \brief Input dialog for a new ItemDefinition.
 *
 * This dialog can be used to visualize/edit properties of or create new
 * concrete ItemDefinitions. If an ItemDefinition has been set, then
 * the dialog will either SHOW or EDIT modes (depending on the active mode).
 * If no ItemDefinition is set, then it will try to create a new one.
 *
 * \sa smtk::attribute::ItemDefinition
 */
class ItemDefDialog : public InputDialog
{
  Q_OBJECT

public:
  ItemDefDialog(QWidget* parent = nullptr);
  ~ItemDefDialog();

  enum class EditMode : unsigned char
  {
    NEW,
    EDIT,
    SHOW
  };

  void setItemDef(smtk::attribute::ItemDefinitionPtr def);

  /**
   * Set instances (Group or Definition) against which name input will be
   * validated when defining a new ItemDefinition.
   */
  void setValidationInstances(
    smtk::attribute::ItemDefinitionPtr itemDef, smtk::attribute::DefinitionPtr attDef);

  /**
   * Applies changes and returns the edited or newly created ItemDef.
   */
  smtk::attribute::ItemDefinitionPtr getItemDef();

  void setEditMode(EditMode mode);

private slots:
  void onTypeChanged(const int type);

private:
  ItemDefDialog(const ItemDefDialog&) = delete;
  void operator=(const ItemDefDialog&) = delete;

  bool validate_impl() override;

  std::unique_ptr<Ui::ItemDefDialog> Ui;
  smtk::attribute::DefinitionPtr AttDef;
  smtk::attribute::GroupItemDefinitionPtr ParentGroup;
  std::shared_ptr<HandlerItemDef> Handler;
};
#endif //__ItemDefDialog_h
