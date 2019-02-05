//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtAssociationView - View for modifying attribute association information
// .SECTION Description
// .SECTION See Also
// qtSection

#ifndef __smtk_extension_qtAssociationView_h
#define __smtk_extension_qtAssociationView_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtBaseView.h"

#include <QMap>
#include <QModelIndex>

class qtAssociationViewInternals;
class QTableWidgetItem;
class QKeyEvent;
class QStandardItem;
class QTableWidget;

namespace smtk
{
namespace extension
{
class SMTKQTEXT_EXPORT qtAssociationView : public qtBaseView
{
  Q_OBJECT

public:
  static qtBaseView* createViewWidget(const ViewInfo& info);
  qtAssociationView(const ViewInfo& info);
  virtual ~qtAssociationView();
  const QMap<QString, QList<smtk::attribute::DefinitionPtr> >& attDefinitionMap() const;

  bool isEmpty() const override;
  void displayAttributes();

public slots:
  void onShowCategory() override;
  void updateModelAssociation() override;
  void onAttributeChanged(int);

signals:
  void attAssociationChanged();

protected:
  void createWidget() override;
  virtual void getAllDefinitions();
  smtk::attribute::AttributePtr getAttributeFromIndex(int index);

private:
  qtAssociationViewInternals* Internals;

}; // class
}; // namespace attribute
}; // namespace smtk

#endif
