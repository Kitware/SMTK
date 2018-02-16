//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/Exports.h"
#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/PublicPointerDefs.h"

#include <QDockWidget>

class pqServer;
class pqPipelineSource;

/**\brief A panel that displays SMTK resources available to the application/user.
  *
  */
class SMTKPQCOMPONENTSEXT_EXPORT pqSMTKAttributePanel : public QDockWidget
{
  Q_OBJECT
  typedef QDockWidget Superclass;

public:
  pqSMTKAttributePanel(QWidget* parent = nullptr);
  ~pqSMTKAttributePanel() override;

  smtk::extension::qtUIManager* attributeUIManager() const { return m_attrUIMgr; }

public slots:
  virtual bool displayPipelineSource(pqPipelineSource* psrc);
  virtual bool displayResource(smtk::attribute::CollectionPtr rsrc);
  virtual bool displayView(smtk::view::ViewPtr view);

protected:
  smtk::extension::qtUIManager* m_attrUIMgr;
  smtk::resource::ResourcePtr m_rsrc;
  int m_observer;
};
