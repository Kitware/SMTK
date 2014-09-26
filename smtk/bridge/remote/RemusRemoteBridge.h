//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_bridge_remote_RemusRemoteBridge_h
#define __smtk_bridge_remote_RemusRemoteBridge_h

#include "smtk/bridge/remote/SMTKRemoteExports.h" // for export macro
#include "smtk/SharedPtr.h" // for export macro
#include "smtk/model/DefaultBridge.h"

#include "remus/client/Client.h" // for m_remusClient
#include "remus/common/MeshRegistrar.h" // for RemusModelBridgeType
#include "remus/worker/ServerConnection.h" // for m_remusConn

namespace smtk {
  namespace io { class ImportJSON; }
  namespace model { class RemoteOperator; }
  namespace bridge {
    namespace remote {

class RemusBridgeConnection; // A Remus client-server connection specifically for smtk::models.

/**\brief A subclass of the Remus mesh type for model bridges.
  *
  * This subclass adds a method to obtain an SMTK bridge name
  * which is different than its advertised input format.
  * Input formats advertised using Remus include compile-time
  * options specific to modeling kernels while bridges do not.
  * For example, it is important to Remus clients to distinguish
  * whether CGM was built with OpenCascade support or not.
  * However, SMTK only cares that the CGM bridge is named "cgm".
  */
struct RemusModelTypeBase : remus::meshtypes::MeshTypeBase
{
  /**\brief Return the name used to register the bridge class with SMTK.
    *
    * This is used to obtain a constructor function from
    * smtk::model::BRepModel::bridgeConstructor().
    */
  virtual std::string bridgeName() const = 0;

  /**\brief Prepare for construction of a bridge of this type.
    *
    * This method is invoked before the bridge constructor function
    * obtained from smtk::model::BRepModel::bridgeConstructor()
    * is invoked. It is used to change the default modeling kernel
    * for bridges to systems like CGM that can support different
    * kernels.
    */
  virtual void bridgePrep() const
    { /* Do nothing by default. */ }
};

/// Shorthand for the class objects used to obtain remus workers.
typedef boost::shared_ptr<RemusModelTypeBase> RemusModelBridgeType;

/**\brief Call this macro in your bridge's implementation file to register it as a remus worker.
  *
  * This macro declares a new struct derived from remus::meshtypes::MeshTypeBase
  * corresponding to the specific modeling kernel. This struct must have a
  * unique uint16_t integer, \a RemusId, associated with it. See remus/common/MeshTypes.h
  * for the basic integer types and be aware that other SMTK modeling kernels
  * must not collide. We recommend using the uint16_t hash of your bridge
  * name (plus 101 if it is lower, as Remus reserves 0--100; or plus 1 if it
  * collides with a pre-existing kernel).
  *
  * The \a BridgeName argument must be identical to what you pass to the
  * smtkImplementsModelingKernel macro.
  *
  * The \a BridgePrep is a function that should be invoked before the
  * the constructor function of the given \a BridgeName is called.
  * If you do not need a function invoked, then pass an empty value for the argument.
  * \a BridgePrep is used to set the default modeling kernel on several backends,
  * including CGM.
  *
  * The \a QualComp argument should include additional runtime requirements
  * on the component.
  */
#define smtkRegisterBridgeWithRemus(BridgeName, BridgePrep, CompString, QualComp) \
  struct smtk ##QualComp## RemusRemoteBridgeType : smtk::bridge::remote::RemusModelTypeBase \
    { \
    static boost::shared_ptr<remus::meshtypes::MeshTypeBase> create() \
      { return boost::shared_ptr<remus::meshtypes::MeshTypeBase>(new smtk ##QualComp## RemusRemoteBridgeType()); } \
    virtual std::string name() const { return CompString ; } \
    virtual std::string bridgeName() const { return BridgeName ; } \
    virtual void bridgePrep() const { (void)0; BridgePrep ; } \
    }; \
  static remus::common::MeshRegistrar smtk ##QualComp## RemusTag( \
    (smtk ##QualComp## RemusRemoteBridgeType()) ); \

/**\brief A bridge that forwards operation requests to a Remus worker.
  *
  * This bridge does no transcription because
  * it forwards all requests to a remote bridge (which is
  * backed with local storage).
  *
  * Instances of this bridge report their name() as "remote".
  * Its remoteName() will be the type of the bridge they provide
  * backing storage for.
  *
  * SMTK does not provide transport for forwarding requests;
  * instead it uses Remus for this.
  * For a protocol, it uses JSON-RPC v2.
  */
class SMTKREMOTE_EXPORT RemusRemoteBridge : public smtk::model::DefaultBridge
{
public:
  smtkTypeMacro(RemusRemoteBridge);
  smtkCreateMacro(RemusRemoteBridge);
  smtkSharedFromThisMacro(Bridge);
  smtkDeclareModelingKernel();

  virtual ~RemusRemoteBridge();

  Ptr setup(RemusBridgeConnection* remusServerConnection, remus::proto::JobRequirements& jreq);
  remus::proto::JobRequirements remusRequirements() const;

  static RemusModelBridgeType findAvailableType(
    const std::string& bridgeType);
  static smtk::model::StringList availableTypeNames();

protected:
  friend class model::RemoteOperator;
  friend class io::ImportJSON;

  RemusRemoteBridge();

  virtual smtk::model::BridgedInfoBits transcribeInternal(
        const smtk::model::Cursor& entity, smtk::model::BridgedInfoBits flags);

  virtual bool ableToOperateDelegate(smtk::model::RemoteOperatorPtr op);
  virtual smtk::model::OperatorResult operateDelegate(
                                            smtk::model::RemoteOperatorPtr op);

  RemusBridgeConnection* m_remusConn;
  smtk::shared_ptr<remus::client::Client> m_remusClient;
  std::string m_remusWorkerName;
  remus::proto::JobRequirements m_remusWorkerReqs;

  static void cleanupBridgeTypes();
};

    } // namespace remote
  } // namespace bridge
} // namespace smtk

#endif // __smtk_bridge_remote_RemusRemoteBridge_h
