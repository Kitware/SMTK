//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKResource_h
#define smtk_extension_paraview_appcomponents_pqSMTKResource_h

#include "smtk/extension/paraview/appcomponents/smtkPQComponentsExtModule.h"

#include "pqPipelineSource.h"

#include "smtk/operation/Observer.h"

#include "smtk/PublicPointerDefs.h"

#include "smtk/extension/paraview/appcomponents/pqQtKeywordWrapping.h"

/**\brief A pqPipelineSource subclass for VTK algorithms that own SMTK resources.
  *
  * A single pqSMTKResource may hold **different** SMTK resources over its lifetime,
  * so there is **not** a permanent, one-to-one correspondence between a
  * pqSMTKResource and an smtk::resource::Resource.
  * Indeed, a pqSMTKResource may exist with a null SMTK resource pointer.
  * In general, a user-facing SMTK resource should never exist without
  * a pqSMTKResource counterpart, though.
  */
class SMTKPQCOMPONENTSEXT_EXPORT pqSMTKResource : public pqPipelineSource
{
  Q_OBJECT
  typedef pqPipelineSource Superclass;

public:
  pqSMTKResource(
    const QString& grp,
    const QString& name,
    vtkSMProxy* proxy,
    pqServer* server,
    QObject* parent = nullptr);
  ~pqSMTKResource() override;

  /**\brief Return the client-side smtk resource mirroring the server's original.
    *
    */
  smtk::resource::ResourcePtr getResource() const;

  /// Drop the resource in preparation for server/application exit.
  void dropResource();

Q_SIGNALS:
  /// This is called when the pqSMTKResource is assigned a new smtk::resource::ResourcePtr.
  void resourceModified(smtk::resource::ResourcePtr);

  /// Internal signal called from the subthread executing an operation to notify
  /// the primary thread that an operation has executed.
  void operationOccurred(QPrivateSignal);

protected Q_SLOTS:
  /**\brief Keep the pqSMTKResource and smtk::resource::Resource in sync.
    *
    * This is called when the ParaView pipeline source (vtkSMTKResourceSource) has had
    * its data modified. This can be the result of a filename change that
    * means the old SMTK resource should be removed from the manager and a new one
    * added. It may also be an unrelated change resulting in a modified resource, but
    * these changes should be tracked by the operation manager, not ParaView signals/slots.
    */
  virtual void synchronizeResource();

protected:
  smtk::resource::WeakResourcePtr m_lastResource;

  smtk::operation::Observers::Key m_key;
};
#endif // smtk_extension_paraview_appcomponents_pqSMTKResource_h
