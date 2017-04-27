//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtMeshItem - UI component for attribute RefItem
// .SECTION Description
// .SECTION See Also
// qtItem

#ifndef __smtk_extension_qtMeshItem_h
#define __smtk_extension_qtMeshItem_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtItem.h"
#include "smtk/mesh/MeshSet.h"

class qtMeshItemInternals;

namespace smtk
{
namespace extension
{
class qtBaseView;
class qtAttribute;

class SMTKQTEXT_EXPORT qtMeshItem : public qtItem
{
  Q_OBJECT

public:
  qtMeshItem(smtk::attribute::ItemPtr, QWidget* parent, qtBaseView* view,
    Qt::Orientation enVectorItemOrient = Qt::Horizontal);
  virtual ~qtMeshItem();

  virtual void setLabelVisible(bool);
  smtk::attribute::MeshItemPtr meshItem();

  bool add(const smtk::mesh::MeshSet& val);
  bool remove(const smtk::mesh::MeshSet& val);

public slots:
  void setOutputOptional(int);

protected slots:
  virtual void updateItemData();

protected:
  virtual void createWidget();
  virtual void loadAssociatedEntities();

private:
  qtMeshItemInternals* Internals;

}; // class

}; // namespace attribute
}; // namespace smtk

#endif
