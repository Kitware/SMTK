//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_resource_Artifact_h
#define smtk_resource_Artifact_h

#include "smtk/common/URL.h" // For ivar and API.
#include "smtk/resource/Component.h"

namespace smtk
{
namespace resource
{

/**\brief An artifact is a component that represents external data.
  *
  * Artifacts are typically used to represent files that hold opaque
  * data relevant to a simulation. Examples include historical input
  * decks, job logs, job results files, meshes of simulation domains.
  *
  * Artifacts may be stored inside or outside of a resource (i.e., they
  * may be shared among multiple resources or they may be considered
  * owned by a single resource).
  *
  * Artifacts may be small or large.
  *
  * What characterizes artifacts is that they reference data external
  * to a resource. The data is referenced via an smtk::common::URL.
  * URLs may be absolute (and must be if the data is stored external to
  * the resource) or relative (and should be if the data is stored internal
  * to the resource). Any relative URL is considered relative to its parent
  * resource's location.
  *
  * Artifacts may also be characterized by a checksum and a timestamp.
  * This allows resources to determine whether an artifact has been
  * modified since the last time it was accessed.
  *
  * Note that this class is abstract; if you wish your resource to store
  * instances of Artifact, you will need to subclass it to provide
  *
  */
class SMTKCORE_EXPORT Artifact : public PersistentObject
{
public:
  using URL = smtk::common::URL;

  smtkTypeMacro(smtk::resource::Artifact);
  smtkSuperclassMacro(smtk::resource::Component);
  smtkSharedFromThisMacro(smtk::resource::PersistentObject);

  ~Artifact() override = default;

  // const smtk::common::UUID& id() const override { return m_id; }
  // void setId(const smtk::common::UUID& uid) override { m_id = uid; }

  /// Return the location of the artifact's data.
  const URL& location() const { return m_location; }

  /// Set the location of the artifact's data.
  bool setLocation(URL location);

  /// Indicate whether the artifact has a checksum provided by returning
  /// the checksum algorithm.
  ///
  /// If the returned token is invalid, no checksum is available.
  smtk::string::Token hasChecksum() const { return m_checksumAlgorithm; }

  /// Return the artifact's checksum (if it has one).
  std::vector<std::uint8_t> checksumData() const { return m_checksumData; }

  /// Set the artifact's current checksum.
  virtual bool setChecksum(smtk::string::Token algorithm, const std::vector<std::uint8_t>& value);

  /// Indicate whether the artifact has a timestamp provided by returning
  /// the timestamp format.
  ///
  /// If the returned token is invalid, no timestamp is available.
  smtk::string::Token hasTimestamp() const { return m_timestampFormat; }

  /// Return the artifact's timestamp (if it has one).
  std::vector<std::uint8_t> timestampData() const { return m_timestampData; }

  /// Set the artifact's current timestamp.
  virtual bool setTimestamp(smtk::string::Token format, const std::vector<std::uint8_t>& value);

  /// Return true if the artifact is still extant and false if expired.
  ///
  /// If an application determines an artifact is no longer accessible,
  /// it may call setExtant(). By default, artifacts are extant upon
  /// creation.
  bool extant() const { return m_extant; }

  /// Set whether an artifact exists at its location or not.
  ///
  /// Artifacts may not be extant initially (for example a log file may not
  /// exist before a simulation job has commenced) or finally (for example a
  /// log file may be removed after a certain time) or at other times in
  /// the lifecycle of the artifact.
  bool setExtant(bool isExtant);

protected:
  Artifact();

  smtk::common::UUID m_id;
  URL m_location;
  smtk::string::Token m_checksumAlgorithm;
  std::vector<std::uint8_t> m_checksumData;
  smtk::string::Token m_timestampFormat;
  std::vector<std::uint8_t> m_timestampData;
  bool m_extant{ true };
};
} // namespace resource
} // namespace smtk

#endif // smtk_resource_Artifact_h
