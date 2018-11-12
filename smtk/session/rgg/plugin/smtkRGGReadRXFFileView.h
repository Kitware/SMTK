//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME smtkRGGReadRXFFileView - UI component for read RXF file
// .SECTION Description
// .SECTION See Also
// qtSection

#ifndef smtkRGGReadRXFFileView_h
#define smtkRGGReadRXFFileView_h

#include "smtk/extension/qt/qtBaseView.h"
#include "smtk/session/rgg/plugin/Exports.h"

class QColor;
class QComboBox;
class QIcon;
namespace smtk
{
namespace extension
{
class qtItem;
}
}
class smtkRGGReadRXFFileViewInternals;

class SMTKRGGSESSIONPLUGIN_EXPORT smtkRGGReadRXFFileView : public smtk::extension::qtBaseView
{
  Q_OBJECT

public:
  smtkRGGReadRXFFileView(const smtk::extension::ViewInfo& info);
  virtual ~smtkRGGReadRXFFileView();

  static smtk::extension::qtBaseView* createViewWidget(const smtk::extension::ViewInfo& info);

  bool displayItem(smtk::attribute::ItemPtr) override;

public slots:
  void updateUI() override {} // NB: Subclass implementation causes crashes.
  void requestModelEntityAssociation() override;
  void onShowCategory() override { this->updateAttributeData(); }
  void valueChanged(smtk::attribute::ItemPtr optype) override;

protected slots:
  virtual void requestOperation(const smtk::operation::OperationPtr& op);
  virtual void cancelOperation(const smtk::operation::OperationPtr&);
  virtual void clearSelection();

  // This slot is used to indicate that the underlying attribute
  // for the operation should be checked for validity
  virtual void attributeModified();
  void apply();

protected:
  void updateAttributeData() override;
  void createWidget() override;
  virtual void setInfoToBeDisplayed() override;

private:
  smtkRGGReadRXFFileViewInternals* Internals;
};

#endif // smtkRGGReadRXFFileView_h
