//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKDisplayAttributeOnLoadBehavior_h
#define smtk_extension_paraview_appcomponents_pqSMTKDisplayAttributeOnLoadBehavior_h

#include "smtk/extension/paraview/appcomponents/Exports.h"

#include "smtk/PublicPointerDefs.h"

#include "smtk/model/EntityTypeBits.h"

#include "smtk/resource/Observer.h" // for EventType

#include <QObject>

#include <string>

class pqSMTKAttributePanel;

class vtkSMSMTKWrapperProxy;

class pqPipelineSource;
class pqServer;

/**\brief Make the SMTK attribute panel display an attribute whenever one is added.
  *
  * When any SMTK resource manager is updated to include a new attribute,
  * display that attribute.
  */
class SMTKPQCOMPONENTSEXT_EXPORT pqSMTKDisplayAttributeOnLoadBehavior : public QObject
{
  Q_OBJECT
  using Superclass = QObject;

public:
  pqSMTKDisplayAttributeOnLoadBehavior(QObject* parent = nullptr);
  ~pqSMTKDisplayAttributeOnLoadBehavior() override;

  /// This behavior is a singleton.
  static pqSMTKDisplayAttributeOnLoadBehavior* instance(QObject* parent = nullptr);

protected slots:
  virtual void observeResourcesOnServer(vtkSMSMTKWrapperProxy* mgr, pqServer* server);
  virtual void unobserveResourcesOnServer(vtkSMSMTKWrapperProxy* mgr, pqServer* server);
  virtual void handleResourceEvent(smtk::resource::ResourcePtr, smtk::resource::EventType);
  virtual void displayAttribute();

protected:
  std::map<smtk::resource::ManagerPtr, smtk::resource::Observers::Key> m_resourceManagerObservers;
  pqSMTKAttributePanel* m_panel;
  smtk::attribute::ResourcePtr m_attr;

private:
  Q_DISABLE_COPY(pqSMTKDisplayAttributeOnLoadBehavior);
};

#endif
