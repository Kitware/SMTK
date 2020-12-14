## Evaluators

### Motivation

Evaluators are an addition to SMTK's Attribute system that are a mechanism to
process an Attribute that is an expression. `ValueItem` already supported
linkage of Attributes to a ValueItem by means of the `<ExpressionType>` SBT tag,
however Evaluators provide a means to transform the Items in those expression
Attributes into meaningful results inside SMTK, as opposed to consumers of an
Attribute Resource. These results can be shown to the user in Qt components and
serialized by consuming application code.

This document outlines changes and additions to support this feature.

### Evaluator

`smtk::attribute::Evaluator` is the abstract base class of Evaluators.
Subclasses must implement the method `evaluate()`, which processes the
Attribute.

Implementations of `evaluate()` which depend on trees of expressions must manage
the state of dependent symbols (i.e. Attributes) by using the API provided in
`smtk::attribute::SymbolDependencyStorage`. SymbolDependencyStorage provides
methods to maintain and check for expressions which depend on the result of
one another, preventing infinite recursion in evaluate().

### EvaluatorFactory

Evaluator are created per Attribute through `EvaluatorFactory`. Evaluators types
are registered via template with an `std::string` alias, and multiple definition
can be used for a single Evaluator, as long as they conform to the Items needed
by the Evaluator. A Definition can only be used with one type of Evaluator at a
time, else EvaluatorFactory could create multiple Evaluators for Attributes of
that Definition type.

`EvaluatorFactory::createEvaluator(Attribute)` returns a `std::unique_ptr` to an
Evaluator. Any consumer with access to an EvaluatorFactory can create an
Evaluator as long as it is evaluatable.

### Evaluator Registration

Plugins can register evaluator via a Registrar as is done by default for
`smtk::attribute::InfixExpressionEvaluator` in smtk::attribute::Registrar with
smtk::attribute::EvaluatorManager. This means an smtk::attribute::Resource
created with an smtk::resource::Manager will have their EvaluatorFactory seeded
with the Evaluator type and std::string alias specified in the Registrar.

### Serialization/deserialization

The designer of an SBT can specify the Evaluator to be used for a Definition in
the following way, as in the case of InfixExpressionEvaluator.

```xml
    <Definitions>
      <AttDef Type="infixExpression" Label="Expression">
        <ItemDefinitions>
          <String Name="expression" Extensible="true"/>
        </ItemDefinitions>
      </AttDef>
    </Definitions>

    <Evaluators>
      <Evaluator Name="InfixExpressionEvaluator">
        <Definition Type="infixExpression">
        </Definition>
      </Evaluator>
    </Evaluators>
```

This means that an Evaluator registered with alias `InfixExpressionEvaluator`
will be created by EvaluatorFactory for Definitions of type `infixExpression`.
The Definition infixExpression has a StringItem called `expression` because
`InfixExpressionEvaluator` expects an infix math expression in string form.

### Changes to smtk::attribute::Resource and smtk::attribute::ValueItemTempate

Attribute Resources are now constructed with an EvaluatorFactory to obtain
Evaluators for Attributes in the Resource.

`ValueItemTemplate` has been modified to attempt to `evaluate()` expressions set
on the Item in `value(std::size_t element)`, and serialize their result to
`std::string` in `valueAsString()`.

### Infix Expression Parsing

A tao PEGTL grammar and structures for computng infix expressions have been
added to smtk::common in `InfixExpressionGrammarImpl.h` and
`InfixExpressionEvalution.h`, respectively. These are wrapped together by
`InfixExpressionGrammar`.

### UI Additions Specific to Infix Expressions

A new qtItem has been added to the smtk::extension namespace.
`qtInfixExpressionEditor` shows the result of an infix expression as the user
changed a QLineEdit. This is linked to the underlying StringItem of the infix
expression. It is created by specifying an `ItemView` with Type
`InfixExpression`.

`qtInputsItems` has been modified to show the result of an evalutable expression
displayed in a read-only QLineEdit when the Item is in expression mode and the
expression Attribute is evaluatable.
