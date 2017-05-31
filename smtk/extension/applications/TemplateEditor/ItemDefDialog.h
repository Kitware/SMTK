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

#include "AttributeProperties.h"
#include "InputDialog.h"

namespace Ui
{
class ItemDefDialog;
}

/**
 * \brief Input dialog for a new AttributeDefinition.
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

  void setAttDef(smtk::attribute::DefinitionPtr def);

  const ItemDefProperties& getInputValues();

  void setEditMode(EditMode mode);

protected slots:
  bool validate_impl() override;

private:
  ItemDefDialog(const ItemDefDialog&) = delete;
  void operator=(const ItemDefDialog&) = delete;

  std::unique_ptr<Ui::ItemDefDialog> Ui;
  ItemDefProperties Properties;
  smtk::attribute::ItemDefinitionPtr ItemDef;
  smtk::attribute::DefinitionPtr AttDef;
};
#endif //__ItemDefDialog_h
