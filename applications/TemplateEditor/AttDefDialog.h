//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __AttDefDialog_h
#define __AttDefDialog_h
#include <memory>

#include "smtk/PublicPointerDefs.h"

#include "DataModelContainers.h"
#include "InputDialog.h"

namespace Ui
{
class AttDefDialog;
}

/**
 * \brief Input dialog for a new AttributeDefinition.
 */
class AttDefDialog : public InputDialog
{
  Q_OBJECT

public:
  AttDefDialog(QWidget* parent = nullptr);
  ~AttDefDialog();

  void setBaseAttDef(smtk::attribute::DefinitionPtr def);

  const AttDefContainer& getInputValues();

private:
  AttDefDialog(const AttDefDialog&) = delete;
  void operator=(const AttDefDialog&) = delete;

  bool validate_impl() override;

  std::unique_ptr<Ui::AttDefDialog> Ui;
  AttDefContainer Container;
  smtk::attribute::DefinitionPtr BaseDef;
};
#endif //__AttDefDialog_h
