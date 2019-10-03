## Test changes

- Removed one problematic test, `displayMultiBlockModel-simple`, that used an outdated input file and rendered differently on some dashboard systems.
- Added `knee.ex2` exodus test data, used by ModelBuilder tests for now.
- If we are running tests, don't display the "Your data is modified, save changes?" dialog on exit.
- Plugin (contract) tests now have a timeout of 600 seconds to allow more time for cloning and building a repository.
- fix the smtkAssignColorsView pallet choose dialog handling so it is testable with xml tests.
