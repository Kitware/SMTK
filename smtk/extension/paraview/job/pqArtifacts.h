//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_job_pqArtifacts_h
#define smtk_extension_paraview_job_pqArtifacts_h

#include "smtk/SharedFromThis.h"
#include "smtk/extension/paraview/job/smtkPVJobExtModule.h"
#include "smtk/string/Token.h"

#include <QPointer>

#include <filesystem>
#include <memory>
#include <string>

class pqPipelineSource;
class pqDataRepresentation;
class vtkSMRepresentationProxy;

namespace smtk
{
namespace job
{
class Job;
}
} // namespace smtk

/** \brief A class to manage ParaView pipelines related to artifacts.
  *
  * Given (1) the (full, unambiguous) path to a file that can be read by ParaView;
  * (2) an application-specific tag for the data in the file; and
  * (3) the name of a reader to load the data in the file; return either a
  * pre-existing pipeline source or a newly-created pipeline source for the file.
  *
  * Artifacts are unique if the combination of their path, application-specific
  * tag, and reader are unique.
  *
  */
class SMTKPVJOBEXT_EXPORT pqArtifacts : public QObject
{
  Q_OBJECT
public:
  smtkSuperclassMacro(QObject);
  smtkTypenameMacroBase(pqArtifacts);

  pqArtifacts();
  ~pqArtifacts() override;

  static pqArtifacts* instance();

  pqPipelineSource* findOrCreate(
    const std::filesystem::path& path,
    smtk::string::Token tag,
    bool* didCreate = nullptr,
    bool findOnly = false,
    const std::string& readerGroup = std::string(),
    const std::string& readerName = std::string());

  /// Given a pipeline source, show or hide its representation in the currently-active view,
  /// returning the representation if found.
  static vtkSMRepresentationProxy* activeViewRepresentation(
    pqPipelineSource* source,
    bool visibility);

  /// Hide all representations of \a source in all views.
  static void hideRepresentations(pqPipelineSource* source);

private:
  class Internal;
  std::unique_ptr<Internal> m_p;
};

#endif // smtk_extension_paraview_job_pqArtifacts_h
