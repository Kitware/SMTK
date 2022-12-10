#include <iostream>
#include <string>

#include "input1_cpp.h"
#include "input1_sbt.h"
#include "input1_xml.h"

int TestEncode(int, char*[])
{
  std::string svg1 = input1_xml;
  std::string sbt = input1_sbt;

  const auto* substituted = R"foo(<?xml version="1.0" encoding="utf-8" ?>
<SMTK_AttributeResource Version="3">
  <Definitions>
    <AttDef Type="operation"/>

    <AttDef Type="dummy" BaseType="operation">
      <AssociationsDef LockType="Write" HoldReference="true" OnlyResources="true">
        <Accepts><Resource Name="smtk::resource::Resource"/></Accepts>
      </AssociationsDef>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
)foo";
  if (sbt != substituted)
  {
    std::cerr << "ERROR: SBT include-substitution mismatch:\n--- expected\n"
              << sbt << "---\nvs\n--- processed\n"
              << substituted << "---\n";
    return 1;
  }

  if (svg1 != input1_cpp())
  {
    std::cerr << "ERROR: XML mismatch for literal vs function:\n--- literal\n"
              << svg1 << "---\nvs\n--- function\n"
              << input1_cpp() << "---\n";
    return 1;
  }

  return 0;
}
