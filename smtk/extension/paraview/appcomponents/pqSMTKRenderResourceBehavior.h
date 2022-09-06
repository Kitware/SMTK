//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKRenderResourceBehavior_h
#define smtk_extension_paraview_appcomponents_pqSMTKRenderResourceBehavior_h

#include "smtk/extension/paraview/appcomponents/smtkPQComponentsExtModule.h"

#include "smtk/extension/paraview/appcomponents/pqSMTKResource.h"

#include "smtk/resource/Resource.h"

#include "smtk/extension/paraview/appcomponents/pqQtKeywordWrapping.h"

#include <QObject>

/// This behavior is responsible for the connections between the
/// addition/removal of a resource from a resource manager and the associated
/// creation/deletion of its pipeline.
///
/// Given an SMTK resource, the behavior constructs a pipeline source for the
/// resource and adds it to the active view. Resources created using ParaView's
/// File->Open do not need this functionality, as it occurs internally within
/// ParaView. As such, the creation of a pipeline is not connected to a resource
/// manager via observation. Other created resources (such as those loaded when
/// resolving links, resources created via the operation panel, etc.) do not
/// automatically go through ParaView's machinations; we duplicate ParaView's
/// source-handling process here.
///
/// The destruction of a pipeline source can be safely connected to a resource
/// manager's signal of a removed resource because we do not need to guard
/// against ParaView's pipeline removal logic.
class SMTKPQCOMPONENTSEXT_EXPORT pqSMTKRenderResourceBehavior : public QObject
{
  Q_OBJECT
  using Superclass = QObject;

public:
  static pqSMTKRenderResourceBehavior* instance(QObject* parent = nullptr);
  ~pqSMTKRenderResourceBehavior() override;

Q_SIGNALS:
  void pipelineSourceCreated(
    smtk::resource::Resource::Ptr smtkResource,
    pqSMTKResource* pipelineSource);
  void aboutToDestroyPipelineSource(
    smtk::resource::Resource::Ptr smtkResource,
    pqSMTKResource* pipelineSource);

public Q_SLOTS:
  pqSMTKResource* createPipelineSource(const smtk::resource::Resource::Ptr&);
  void destroyPipelineSource(const smtk::resource::Resource::Ptr&);
  void renderPipelineSource(pqSMTKResource* source);

protected:
  pqSMTKRenderResourceBehavior(QObject* parent = nullptr);

  class Internal;
  Internal* m_p;

private:
  Q_DISABLE_COPY(pqSMTKRenderResourceBehavior);
};

#endif // smtk_extension_paraview_appcomponents_pqSMTKRenderResourceBehavior_h
