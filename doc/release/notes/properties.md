## Introduction of Properties for Resources and Components

Resources and components now have access to Properties, a
dictionary-like container that can hold any copy-constructible
type. Currently enabled types include long, double, std::string, and
std::vectors of these three types. Resource and component properties
replace and extend the functionality of smtk::model's property system,
while maintaining much of smtk::model's property API. For more
information, see the Properties section of the Resources description
in the user guide.
