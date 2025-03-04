//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/plugin/Registry.h"

#include "smtk/common/testing/cxx/helpers.h"

#include <set>

namespace
{
class Manager_1
{
public:
  std::set<std::string> managed;
};

class Manager_2
{
public:
  std::set<std::string> managed;
};

class Manager_3
{
public:
  std::set<std::string> managed;
};

class Registrar_1
{
public:
  static constexpr const char* const type_name = "Registrar 1";

  [[nodiscard]] bool registerTo(const std::shared_ptr<Manager_1>& /*m*/) const;
  [[nodiscard]] bool unregisterFrom(const std::shared_ptr<Manager_1>& /*m*/) const;
  [[nodiscard]] bool registerTo(const std::shared_ptr<Manager_2>& /*m*/) const;
  void unregisterFrom(const std::shared_ptr<Manager_2>& /*m*/) const;
  void registerTo(const std::shared_ptr<Manager_3>& /*m*/) const;
  void unregisterFrom(const std::shared_ptr<Manager_3>& /*m*/) const;
};

bool Registrar_1::registerTo(const std::shared_ptr<Manager_1>& m) const
{
  auto inserted = m->managed.insert(type_name);
  test(inserted.second, "Double registering a registrar with a manager");
  return true;
}

bool Registrar_1::unregisterFrom(const std::shared_ptr<Manager_1>& m) const
{
  auto it = m->managed.find(type_name);
  test(it != m->managed.end(), "unregistering an unkown registrar from a manager");
  m->managed.erase(it);
  return true;
}

bool Registrar_1::registerTo(const std::shared_ptr<Manager_2>& m) const
{
  auto inserted = m->managed.insert(type_name);
  test(inserted.second, "Double registering a registrar with a manager");
  return true;
}

void Registrar_1::unregisterFrom(const std::shared_ptr<Manager_2>& m) const
{
  auto it = m->managed.find(type_name);
  test(it != m->managed.end(), "unregistering an unkown registrar from a manager");
  m->managed.erase(it);
}

void Registrar_1::registerTo(const std::shared_ptr<Manager_3>& m) const
{
  auto inserted = m->managed.insert(type_name);
  test(inserted.second, "Double registering a registrar with a manager");
}

void Registrar_1::unregisterFrom(const std::shared_ptr<Manager_3>& m) const
{
  auto it = m->managed.find(type_name);
  test(it != m->managed.end(), "unregistering an unkown registrar from a manager");
  m->managed.erase(it);
}

class Registrar_2
{
public:
  static constexpr const char* const type_name = "Registrar 2";

  static int registerTo(const std::shared_ptr<Manager_1>& /*m*/);
  static bool unregisterFrom(const std::shared_ptr<Manager_1>& /*m*/);
  static bool registerTo(const std::shared_ptr<Manager_2>& /*m*/);
  static void unregisterFrom(const std::shared_ptr<Manager_2>& /*m*/);
};

int Registrar_2::registerTo(const std::shared_ptr<Manager_1>& m)
{
  auto inserted = m->managed.insert(type_name);
  test(inserted.second, "Double registering a registrar with a manager");
  return 4;
}

bool Registrar_2::unregisterFrom(const std::shared_ptr<Manager_1>& m)
{
  auto it = m->managed.find(type_name);
  test(it != m->managed.end(), "unregistering an unkown registrar from a manager");
  m->managed.erase(it);
  return true;
}

bool Registrar_2::registerTo(const std::shared_ptr<Manager_2>& m)
{
  auto inserted = m->managed.insert(type_name);
  test(inserted.second, "Double registering a registrar with a manager");
  return true;
}

void Registrar_2::unregisterFrom(const std::shared_ptr<Manager_2>& m)
{
  auto it = m->managed.find(type_name);
  test(it != m->managed.end(), "unregistering an unkown registrar from a manager");
  m->managed.erase(it);
}
} // namespace

int UnitTestRegistry(int /*unused*/, char** const /*unused*/)
{
  auto manager_1 = std::make_shared<Manager_1>();
  auto manager_2 = std::make_shared<Manager_2>();
  auto manager_3 = std::make_shared<Manager_3>();
  test(
    manager_1->managed.empty() && manager_2->managed.empty() && manager_3->managed.empty(),
    "New managers should not be managing anything");
  {
    smtk::plugin::Registry<Registrar_1, Manager_1, Manager_2, Manager_3> registry_1(
      manager_1, manager_2, manager_3);
    test(
      manager_1->managed.size() == 1 && manager_2->managed.size() == 1 &&
        manager_3->managed.size() == 1,
      "Managers should be managing one thing");
  }
  test(
    manager_1->managed.empty() && manager_2->managed.empty() && manager_3->managed.empty(),
    "Cleared managers should not be managing anything");
  smtk::plugin::Registry<Registrar_1, Manager_1, Manager_2, Manager_3> registry_2(
    manager_1, manager_2, manager_3);
  test(
    manager_1->managed.size() == 1 && manager_2->managed.size() == 1 &&
      manager_3->managed.size() == 1,
    "Managers should be managing one thing again");

  auto manager_22 = std::make_shared<Manager_2>();
  auto manager_33 = std::make_shared<Manager_3>();
  {
    smtk::plugin::Registry<Registrar_2, Manager_1, Manager_2, Manager_3> registry_3(
      manager_1, manager_22, manager_33);

    test(manager_1->managed.size() == 2, "Manager_1 should be managing two things");
    test(manager_2->managed.size() == 1, "Manager_2 should be managing one thing");
    test(manager_22->managed.size() == 1, "Manager_22 should be managing one thing");
    test(manager_3->managed.size() == 1, "Manager_3 should be managing one thing");
    test(manager_33->managed.empty(), "Manager_3 should not be managing anything");
  }

  return 0;
}
