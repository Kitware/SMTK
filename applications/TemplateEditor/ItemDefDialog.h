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
 * \brief Input dialog to show, edit or create new ItemDefinitions.
 *
 * This dialog can be used to visualize/edit properties of or create new
 * concrete ItemDefinitions. If an ItemDefinition has been set, then
 * the dialog will either SHOW or EDIT the ItemDef (depending on the active mode).
 * If no ItemDefinition is set, then it will try to create a new one.
 *
 * \sa smtk::attribute::ItemDefinition
 */
class ItemDefDialog : public InputDialog
{
  Q_OBJECT

public:
  ItemDefDialog(QWidget* parent = nullptr);
  ~ItemDefDialog() override;

  ItemDefDialog(const ItemDefDialog&) = delete;
  ItemDefDialog& operator=(const ItemDefDialog&) = delete;

  /**
   * Set the ItemDefinition to Show or Edit.
   * ///TODO Make the Dialog choose the EditMode automatically (no need
   * // to set it explicitly).
   */
  void setItemDef(smtk::attribute::ItemDefinitionPtr def);

  /**
   * Set instances (Group or Definition) against which name input will be
   * validated when defining a new ItemDefinition.
   */
  void setValidationInstances(
    smtk::attribute::ItemDefinitionPtr itemDef,
    smtk::attribute::DefinitionPtr attDef);

  /**
   * Apply changes and return the edited or newly created ItemDef.
   */
  smtk::attribute::ItemDefinitionPtr getItemDef();

  //@{
  /**
   * Show or edit an active ItemDefinition or create a new entity if none
   * was set.
   */
  enum class EditMode : unsigned char
  {
    NEW,
    EDIT,
    SHOW
  };

  void setEditMode(EditMode mode);
  //@}

private Q_SLOTS:
  /**
   * Update the UI depending on the currently selected concrete ItemDef type.
   * The Dialog uses HandlerItemDef instances to customize and create ItemDefinition
   * instances.
   *
   * \sa HandlerItemDef
   */
  void onTypeChanged(int type);

private:
  /**
   * Implements validation of critical fields for ItemDefinition instantiation
   * (e.g. Name, etc.).
   */
  bool validate_impl() override;

  /**
   * Arrange types in list of strings. Uses smtk::attribute::Item string types.
   * This assumes smtk::attribute::Item::Type is continuous and NUMBER_OF_TYPES
   * is the last type.
   */
  QStringList getTypeList();

  std::unique_ptr<Ui::ItemDefDialog> Ui;
  smtk::attribute::DefinitionPtr AttDef;
  smtk::attribute::GroupItemDefinitionPtr ParentGroup;
  std::shared_ptr<HandlerItemDef> Handler;
};
#endif //__ItemDefDialog_h
