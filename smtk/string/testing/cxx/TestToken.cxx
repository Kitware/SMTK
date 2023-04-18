//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/string/Token.h"
#include "smtk/string/json/jsonToken.h"

#include "smtk/common/testing/cxx/helpers.h"

#include <iostream>
#include <unordered_set>

int TestToken(int, char*[])
{
  using namespace smtk::string::literals; // for ""_token
  using json = nlohmann::json;

  // Test that construction from a std::string works.
  std::string dab = "bad";
  smtk::string::Token bad = dab;

  // Test that construction from a string literal works.
  smtk::string::Token tmp = "tmp";

  // Test that construction from our string literal operator works.
  auto foo = "foo"_token;
  auto bar = "bar"_token;
  auto oof = "foo"_token;

  std::cout << "Testing comparison operators for:\n";
  std::cout << "  " << bad.data() << " "
            << "0x" << std::hex << bad.id() << std::dec << "\n";
  std::cout << "  " << tmp.data() << " "
            << "0x" << std::hex << tmp.id() << std::dec << "\n";
  std::cout << "  " << foo.data() << " "
            << "0x" << std::hex << foo.id() << std::dec << "\n";
  std::cout << "  " << oof.data() << " "
            << "0x" << std::hex << oof.id() << std::dec << "\n";
  std::cout << "  " << bar.data() << " "
            << "0x" << std::hex << bar.id() << std::dec << "\n";

  // Test comparison operators
  test(foo == oof, "String comparison incorrect.");
  test(bar != foo, "String comparison incorrect.");
  test(bar <= foo, "String lexical order must be preserved.");
  test(foo >= bar, "String lexical order must be preserved.");
  test(bar < foo, "String lexical order must be preserved.");
  test(foo > bar, "String lexical order must be preserved.");

  // Test string-literal comparison operators
  test("foo" == oof, "String comparison incorrect.");
  test("bar" != foo, "String comparison incorrect.");
  test("bar" <= foo, "String lexical order must be preserved.");
  test("foo" >= bar, "String lexical order must be preserved.");
  test("bar" < foo, "String lexical order must be preserved.");
  test("foo" > bar, "String lexical order must be preserved.");

  test(foo == "foo", "String comparison incorrect.");
  test(bar != "foo", "String comparison incorrect.");
  test(bar <= "foo", "String lexical order must be preserved.");
  test(foo >= "bar", "String lexical order must be preserved.");
  test(bar < "foo", "String lexical order must be preserved.");
  test(foo > "bar", "String lexical order must be preserved.");

  // Test hash functor (verify that unordered containers are supported).
  std::unordered_set<smtk::string::Token> set;
  set.insert("foo"_token);
  set.insert(foo);
  set.insert(bar);
  set.insert("baz");
  set.insert(bad);
  set.insert(tmp);
  test(set.size() == 5, "Expected set to have 5 members.");

  // Test construction from a hash.
  smtk::string::Token foo2 = smtk::string::Token::fromHash(foo.id());
  (void)foo2;

  // Test that attempted construction from a non-existent hash throws.
  try
  {
    smtk::string::Token foo3 = smtk::string::Token::fromHash(5551212);
    (void)foo3;
    test(false, "Expected to throw an exception for a non-existent hash.");
  }
  catch (std::invalid_argument& e)
  {
    // Do nothing.
    std::string message = e.what();
    test(message == "Hash does not exist in database.", "Wrong exception.");
  }

  // Test json serialization/deserialization.
  json j = foo;
  bar = j;
  test(bar.data() == foo.data(), "Expected JSON assignment to work.");

  // Test constexpr-ness (i.e., that tokens can be used in switch statements).
  // Really, the test here is at compile time; it ensures that no one
  // breaks anything constexpr as needed for the switch() use-case:
  std::cout << "Testing switch statement with hash cases: ";
  bool ok = false;
  switch (foo.id())
  {
    case "foo"_hash:
      ok = true;
      break;
    case "bar"_hash:
      ok = false;
      break;
    default:
      ok = false;
      break;
  }
  std::cout << (ok ? "pass" : "fail") << "\n";
  test(ok, "Expected switch statement to work.");

  // Test that set<Token> works as expected (with slow alphanumeric sorting).
  std::cout << "Testing slow alphanumeric sorting for set<Token>:\n";
  std::set<smtk::string::Token> candies;
  std::vector<std::string> expected{
    { "gumdrop", "mike&ike", "pixie_stick", "tootsie_roll", "twinkie" }
  };
  // Insert in reverse order just to be sure sorting occurs:
  candies.insert(expected.rbegin(), expected.rend());
  auto expectedCandy = expected.begin();
  for (const auto& candy : candies)
  {
    std::cout << "  " << std::hex << candy.id() << std::dec << ":  " << candy.data() << "\n";
    test(candy.data() == *expectedCandy, "Unexpected order for sorted tokens.");
    ++expectedCandy;
  }

  smtk::string::Token naughty(smtk::string::Token::Invalid);
  smtk::string::Token uninitialized;
  test(!naughty.valid(), "Improper validity check.");
  test(!uninitialized.valid(), "Uninitialized tokens should be invalid.");

  return 0;
}
