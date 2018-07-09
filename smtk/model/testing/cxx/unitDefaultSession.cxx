//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/AutoInit.h"
#include "smtk/Options.h"

#include "smtk/io/LoadJSON.h"
#include "smtk/io/SaveJSON.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DirectoryItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/RefItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"
#include "smtk/common/UUIDGenerator.h"
#include "smtk/model/DefaultSession.h"
#include "smtk/model/Model.h"
#include "smtk/model/RemoteOperation.h"
#include "smtk/model/Resource.h"
#include "smtk/model/Session.h"
#include "smtk/model/SessionRef.h"

#include "smtk/common/testing/cxx/helpers.h"
#include "smtk/model/testing/cxx/helpers.h"

#include "cJSON.h"

#include "unitForwardingOperation_xml.h"

using namespace smtk::common;
using namespace smtk::model;
using namespace smtk::io;

using smtk::attribute::IntItem;

namespace
{

template <class T>
void printItem(typename T::Ptr item, const std::string& tname)
{
  std::cout << (item->name().empty() ? "(null)" : item->name()) << ":";
  std::size_t ne = item->numberOfValues();
  std::cout << " " << tname << " (" << ne << ") [";
  for (std::size_t e = 0; e < ne; ++e)
  {
    std::cout << " " << item->value(e);
  }
  std::cout << " ]";
}

template <>
void printItem<smtk::attribute::GroupItem>(
  smtk::attribute::GroupItem::Ptr item, const std::string& tname)
{
  std::cout << (item->name().empty() ? "(null)" : item->name()) << ":";
  std::cout << " " << tname << " ";
}

template <>
void printItem<smtk::attribute::VoidItem>(
  smtk::attribute::VoidItem::Ptr item, const std::string& tname)
{
  std::cout << (item->name().empty() ? "(null)" : item->name()) << ":";
  std::cout << " " << tname << " ";
}

} // anonymous namespace

#define smtkAttributeItemTypeCase(ENUM, CLASSNAME, ENTRYTYPE, ITEM, CODE)                          \
  case smtk::attribute::Item::ENUM:                                                                \
  {                                                                                                \
    smtk::attribute::CLASSNAME::Ptr typedItem =                                                    \
      smtk::dynamic_pointer_cast<smtk::attribute::CLASSNAME>(ITEM);                                \
    std::string enumName(#ENUM);                                                                   \
    typedef smtk::attribute::CLASSNAME ItemTType;                                                  \
    if (typedItem)                                                                                 \
    {                                                                                              \
      CODE;                                                                                        \
    }                                                                                              \
  }                                                                                                \
  break

#define smtkAttributeItemTypeSwitch(CODE)                                                          \
  smtkAttributeItemTypeCase(IntType, IntItem, int, item, CODE);                                    \
  smtkAttributeItemTypeCase(DoubleType, DoubleItem, double, item, CODE);                           \
  smtkAttributeItemTypeCase(StringType, StringItem, std::string, item, CODE);                      \
  smtkAttributeItemTypeCase(FileType, StringItem, std::string, item, CODE);                        \
  smtkAttributeItemTypeCase(DirectoryType, StringItem, std::string, item, CODE);                   \
  smtkAttributeItemTypeCase(AttributeRefType, RefItem, smtk::attribute::AttributePtr, item, CODE); \
  smtkAttributeItemTypeCase(ModelEntityType, ModelEntityItem, smtk::model::EntityRef, item, CODE); \
  smtkAttributeItemTypeCase(GroupType, GroupItem, smtk::attribute::ItemPtr, item, CODE);           \
  smtkAttributeItemTypeCase(VoidType, VoidItem, void, item, CODE);                                 \
  default:                                                                                         \
    break;

static void printParams(smtk::attribute::AttributePtr attr, const std::string& msg)
{
  std::cout << msg << "\n";
  if (!attr)
    return;
  std::size_t ni = attr->numberOfItems();
  for (std::size_t i = 0; i < ni; ++i)
  {
    std::cout << "  ";
    smtk::attribute::ItemPtr item = attr->item(static_cast<int>(i));
    switch (item->type())
    {
      smtkAttributeItemTypeSwitch(printItem<ItemTType>(typedItem, enumName));
    }
    std::cout << "\n";
  }
}

// Implement a forwarding session that does no communication.
// Instead, it just copies operator parameters _to_ a "remote"
// session and results back _from_ the "remote" session.
class TestForwardingSession : public smtk::model::DefaultSession
{
public:
  smtkTypeMacro(TestForwardingSession);             // Provides typedefs for Ptr, SelfType, ...
  smtkSuperclassMacro(smtk::model::DefaultSession); // Provides typedefs for Superclass, ...
  smtkCreateMacro(TestForwardingSession);           // Provides static create() method
  smtkSharedFromThisMacro(Session);                 // Provides shared_from_this() method
  smtkDeclareModelingKernel();                      // Declares name() and utility methods/members.

  smtk::model::Resource::Ptr remoteModel;
  smtk::model::Session::Ptr remoteSession;

  void addSomeRemoteDanglers(const UUIDs& danglers)
  {
    UUIDs::const_iterator it;
    for (it = danglers.begin(); it != danglers.end(); ++it)
    {
      this->remoteSession->declareDanglingEntity(
        smtk::model::EntityRef(this->remoteModel, *it), smtk::model::SESSION_NOTHING);
    }
  }

  bool checkLocalDanglers(const UUIDs& danglers, smtk::model::ResourcePtr modelResource)
  {
    UUIDs::const_iterator it;
    for (it = danglers.begin(); it != danglers.end(); ++it)
    {
      smtk::model::DanglingEntities::const_iterator cit =
        m_dangling.find(smtk::model::EntityRef(modelResource, *it));
      if (cit == m_dangling.end())
        return false;
    }
    return true;
  }

protected:
  TestForwardingSession()
  {
    this->initializeOperationCollection(TestForwardingSession::s_operators);
  }

  SessionInfoBits transcribeInternal(
    const EntityRef& entity, SessionInfoBits flags, int depth = -1) override
  {
    return remoteSession->transcribe(EntityRef(remoteModel, entity.entity()), flags, false, depth);
  }

  bool ableToOperateDelegate(RemoteOperationPtr oper) override
  {
    OperationPtr remOp = remoteSession->op(oper->name());
    remOp->setSpecification(
      remoteSession->operatorCollection()->copyAttribute(oper->specification()));
    return remOp->ableToOperate();
  }

  OperationResult operateDelegate(RemoteOperationPtr localOp) override
  {
    printParams(localOp->specification(), "local input");
    OperationPtr remOp = remoteSession->op(localOp->name());
    remOp->setSpecification(
      remoteSession->operatorCollection()->copyAttribute(localOp->specification()));
    OperationResult remResult = remOp->operate();
    OperationResult localResult = this->operatorCollection()->copyAttribute(remResult);

    // Kill remote operator and result.
    remOp->session()->operatorCollection()->removeAttribute(remOp->specification());
    remResult->collection()->removeAttribute(remResult);

    printParams(localResult, "local output");
    return localResult;
  }
};
smtkImplementsModelingKernel(
  /* no export symbol */, forwarding, "{\"kernel\":\"test-forwarding\", \"engines\":[]}",
  SessionHasNoStaticSetup, TestForwardingSession,
  false /* forwarding session should not inherit local operators */
  );

class TestForwardingOperation : public Operation
{
public:
  smtkTypeMacro(TestForwardingOperation);
  smtkCreateMacro(TestForwardingOperation);
  smtkSharedFromThisMacro(Operation);
  smtkDeclareModelOperation();

  bool ableToOperate() override
  {
    if (!this->specification()->isValid())
      return false;

    if (!this->specification()->find("addToCount")->isEnabled())
    {
      return false;
    }

    return true;
  }

protected:
  TestForwardingOperation() {}

  OperationResult operateInternal() override
  {
    printParams(this->specification(), "actual input");

    OperationResult result = this->createResult(smtk::operation::Operation::OPERATION_SUCCEEDED);
    this->s_state += this->specification()->findInt("addToCount")->value();
    result->findInt("state")->setValue(this->s_state);

    printParams(result, "actual output");
    return result;
  }

  // Note that any state to be preserved between
  // invocations of the operator must be kept in
  // smtk::model::Resource or class static. This is
  // because the operator is created by the session
  // each time it is requested.
  static Integer s_state;
};
smtkImplementsModelOperation(
  /* no export symbol */, TestForwardingOperation, forwarding, "forwarding operator",
  unitForwardingOperation_xml,
  smtk::model::DefaultSession); //TestForwardingSession);

Integer TestForwardingOperation::s_state = 1;

void printSessionOperationNames(const SessionRef& br, const std::string& msg)
{
  StringList opNames = br.operatorNames();
  StringList::const_iterator it;
  std::cout << "Session \"" << br.name() << "\" [" << br.session()->className() << ", " << msg
            << "] operators:\n";
  for (it = opNames.begin(); it != opNames.end(); ++it)
  {
    smtk::operation::OperationPtr op = br.op(*it);
    std::cout << "  " << *it << " [" << op->className() << "]"
              << "\n";
  }
  std::cout << "\n";
}

// Test remote bridging: create 2 model::Resource instances,
// add a native operator to resource A's "native" session,
// serialize the session into a DefaultSession instance
// that backs it into resource B, and invoke the remote version
// of the operator attached to the DefaultSession on resource B.
// Check that the operation was invoked on resource A and that
// the OperationResult from both operations are identical (by
// having the native operator cache its result parameters).
int main()
{
  int status = 0;

  try
  {

    // Create the resources
    smtk::model::Resource::Ptr remoteResource = smtk::model::Resource::create();
    smtk::model::Resource::Ptr localResource = smtk::model::Resource::create();

    // The default session of the "remote" resource:
    SessionRef remoteSess = remoteResource->createSession("native");
    Session::Ptr remoteSession = remoteSess.session();
    remoteSess.setName("remote session");
    printSessionOperationNames(remoteSess, "remote");

    // Now we want to mirror the remote resource locally.
    // Serialize the "remote" session:
    cJSON* sessJSON = cJSON_CreateObject();
    SaveJSON::forResourceSession(remoteSession->sessionId(), sessJSON, remoteResource);
    // ... and import the session locally to a new session object.
    TestForwardingSession::Ptr localSession = TestForwardingSession::create();
    localSession->remoteSession = remoteSession;
    localSession->remoteModel = remoteResource;
    localResource->registerSession(localSession);
    SessionRef localSess(localResource, localSession->sessionId());
    test(localSession->operatorNames().size() == 0,
      "Forwarding session should have no operators by default.");
    printSessionOperationNames(localSess, "local, pre-import");
    LoadJSON::ofRemoteSession(sessJSON->child, localSession, localResource);
    printSessionOperationNames(localSess, "local, post-import");
    test(localSession->operatorNames().size() == remoteSession->operatorNames().size(),
      "Forwarding session operator count should match remote session after import.");

    // Run the local operator.
    // Examine the remote version to verify the operation was forwarded.
    OperationPtr localOp = localSession->op("forwarding operator");
    test(smtk::dynamic_pointer_cast<smtk::model::RemoteOperation>(localOp) ? true : false,
      "Local forwarding operator was not an instance of RemoteOperation.");
    test(localOp->className() == "smtk::model::RemoteOperation",
      "Local operator did not return expected className() \"smtk::model::RemoteOperation\"");
    localOp->ensureSpecification();
    test(!!localOp->specification(),
      "Local operator should have a default specification, not a null pointer.");
    localOp->findInt("addToCount")->setValue(1);
    localOp->findAs<IntItem>("addToCount")->setValue(2);
    test(localOp->specification()->findInt("addToCount")->value() == 2,
      "Setting a valid parameter had no effect.");
    OperationResult localResult = localOp->operate();

    test(
      localResult->findInt("outcome")->value() == smtk::operation::Operation::OPERATION_SUCCEEDED,
      "Operation should have succeeded.");
    test(localResult->findInt("state")->value() == 3, "Operation should have yielded state == 3.");
    localOp->eraseResult(localResult);

    std::cout << "\n---\n\n";

    // Rerun the local operator.
    localOp->specification()->findInt("addToCount")->setValue(8);
    localResult = localOp->operate();

    test(
      localResult->findInt("outcome")->value() == smtk::operation::Operation::OPERATION_SUCCEEDED,
      "Operation should have succeeded.");
    test(
      localResult->findInt("state")->value() == 11, "Operation should have yielded state == 11.");

    // Rerun the local operator but with an "improper" input (disabling a parameter that really
    // shouldn't be optional is a surrogate for situations where an operator may be unable to run)
    // This also tests the setParameterEnabled method.
    test(localOp->specification()->find("addToCount")->isOptional(),
      "Expected optional \"addToCount\".");

    localOp->specification()->findInt("addToCount")->setIsEnabled(false);
    test(!localOp->specification()->find("addToCount")->isEnabled(),
      "Did not disable \"addToCount\".");
    localResult = localOp->operate();

    test(localResult->findInt("outcome")->value() == smtk::operation::Operation::UNABLE_TO_OPERATE,
      "Operation should have been unable to execute.");

    // Test transferring remote dangling entity list to local session.
    // (As a preentityref to a full implementation of TestForwardingSession::transcribeInternal)
    UUIDs danglers;
    for (int i = 0; i < 8; ++i)
      danglers.insert(UUIDGenerator::instance().random());
    localSession->addSomeRemoteDanglers(danglers);
    cJSON* jsonDanglers = cJSON_CreateObject();
    smtk::io::SaveJSON::forDanglingEntities(
      remoteSession->sessionId(), jsonDanglers, remoteResource);
    //std::cout << "\n\n\n" << cJSON_Print(jsonDanglers) << "\n\n\n";
    smtk::io::LoadJSON::ofDanglingEntities(jsonDanglers, localResource);
    test(localSession->checkLocalDanglers(danglers, localResource),
      "All generated danglers should have been serialized.");
  }
  catch (const std::string& msg)
  {
    (void)msg; // Ignore the message; it's already been printed.
    std::cerr << "Exiting...\n";
    status = -1;
  }

  return status;
}
