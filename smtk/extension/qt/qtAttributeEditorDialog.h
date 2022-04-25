//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtAttributeEditorDialog - A Information Dialog for SMTK Operations
// .SECTION Description
// .SECTION Caveats
#ifndef _qtAttributeEditorDialog_h
#define _qtAttributeEditorDialog_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/extension/qt/Exports.h"

#include "smtk/extension/qt/qtUIManager.h"
#include <QtWidgets/QDialog>

namespace Ui
{
class qtAttributeEditorWidget;
}

namespace smtk
{
namespace extension
{

class qtInstancedView;
class qtAttribute;

/**\brief Provides a mechanism to edit an attribute using a dialog mechanism instead of a panel.
  *
  * The dialog uses a qtInstanceVuew internally though we hope to make this more general in the
  * future.  Note that canceling or ok'ing the dialog results in the attribute being modified  thoough
  * the value returned using exec() does indicate whether the user accepted or canceled the changes.
  * This limitation works with the initial use for the dialog which is creating new expression attributes
  * for Value Items without changing Views.  In this case, canceling means delete the newly created attribute
  * so there is no issue with having modifying the attribute previously.
  * There is an option to hide the cancel button which also gets around this limitation.
  */
class SMTKQTEXT_EXPORT qtAttributeEditorDialog : public QDialog
{
  Q_OBJECT

public:
  qtAttributeEditorDialog(
    const smtk::attribute::AttributePtr& attribute,
    smtk::extension::qtUIManager* uiManager,
    QWidget* Parent);
  ~qtAttributeEditorDialog() override;
  qtAttributeEditorDialog(const qtAttributeEditorDialog&) = delete;
  qtAttributeEditorDialog& operator=(const qtAttributeEditorDialog&) = delete;

  void hideCancel();
  void showCancel();

public Q_SLOTS:
  void attributeNameChanged();

private:
  const smtk::attribute::AttributePtr& m_attribute;
  QPointer<smtk::extension::qtUIManager> m_uiManager;
  Ui::qtAttributeEditorWidget* m_widget;
  smtk::extension::qtAttribute* m_qtAttribute;
  std::unique_ptr<qtInstancedView> m_instancedView;
  smtk::view::ConfigurationPtr m_instancedViewDef;
};
} // namespace extension
} // namespace smtk
#endif // _qtAttributeEditorDialog_h
