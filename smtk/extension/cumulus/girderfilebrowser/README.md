# Girder File Browser
A custom Qt file browser for browsing a girder server.

This is a stand-alone program that depends only on Qt, and it can be compiled and ran as such.

## Building
Dependencies:
  - Qt >= 5.5

To build, simply create a build directory, and then run `cmake <path/to/source/tree>`, and then `make`.
An executable will be placed in the build directory called `girderfilebrowser`.

## Authenticating the Girder File Browser
<img src="https://raw.githubusercontent.com/psavery/girderfilebrowser/master/images/currentLoginWindow.png" width="45%">

When the program is started, a login window will appear asking for the girder api url. This usually ends
with `/api/v1`. The username and password may then be entered. The password is sent using basic
http authentication, so it may be advisable to ensure that ssl is being used.

### Environment variables
Two environment variables are checked when the program starts: `GIRDER_API_URL` and `GIRDER_API_KEY`.
If `GIRDER_API_URL` exists, the api url on the login dialog will be pre-filled with its value.

If both `GIRDER_API_URL` and `GIRDER_API_KEY` exist, an automatic login via api key authentication
will be attempted. If successful, the girder file browser will immediately appear, and browsing
can begin. If api authentication fails, the login window will just appear.

## Running the Girder File Browser
<img src="https://raw.githubusercontent.com/psavery/girderfilebrowser/master/images/currentFileBrowser.png" width="35%">

The above image shows the current appearance of the file browser dialog.

The main file browsing window is in the middle. Here, folders (such as `More Data`) and files (such as `Entry`) will
be displayed. Note that although they appear as folders and files in the browser window, it may not always be
their corresponding girder type. For instance, all of these girder types may be displayed as a folder in the browser:
- folder
- user
- collection
- item

The folder icon, then, simply means that, whatever object it is, there are contents inside that may be viewed.

Rows with the file icon indicate a girder object whose contents may not be viewed. Such girder objects can be
either files or items (if the settings are such that item contents cannot be viewed).

Double-clicking or pressing the enter/return key on folder items will cause the girder file browser to enter
that folder.

In the top left corner of the browser window is the path to the root folder. Each of those root path buttons
may be pressed to enter the corresponding folder. If space runs out for the root path buttons, the list will
be truncated to include the most recent root path items, and the left arrow button will be highlighted to allow
a user to inspect previous root path items. They can then scroll back with the right arrow button.

In the top right corner of the browser window is an "Up" button. The up button moves up one folder in the root
path chain.

In the bottom right corner is a "Home" button. This button will, by default, take a user to their home directory.
If, however, a custom root is provided programmatically (so that /root is no longer the top root directory), the
home button will instead take a user to the custom root directory.

The "Choose" button in the bottom right corner prints to the terminal the girder object information of the chosen
row. It is intended primarily for external applications to use when having a user select a girder object.

The "Filter:" in the bottom left corner uses regex to remove all rows that do not contain the text in the box.
It is case-insensitive.

## Using the Girder File Browser in an External Program
One potential use of the girder file browser is to have a user choose a file
on a girder server for some purpose in an application. For instance, the application
could have a user choose an input file on a girder server.

An example of how the girder file browser is created and shown can be seen
[here](girderfilebrowser.cxx).

The only parts that are needed to start the girder file browser are:
- a QNetworkAccessManager object (there should be one per program)
- an api url
- a valid girder token for that api url.

The girder token can be obtained usually by password or api key authentication. They can be set on the
dialog object with `GirderFileBrowserDialog::setApiUrlAndGirderToken()`.

Optionally, in the Girder File Browser Dialog constructor, a custom root path can be passed as the second
constructor parameter. This second parameter is a `QMap<QString, QString>` which should have three entries:
`name`, `id`, and `type`. The behavior is undefined if any of these are specified incorrectly. Once the custom
root path is set, the girder file browser should never be able to go above that root path.

Items in girder can be treated a few different ways with the file browser. Fundamentally, they may be treated
as either folders or files. But there is also an option of treating items as folders but bumping the item's
containing file up one directory level if the item contains only one file and it has the same name.
Programmatically, the way items are treated in the girder file browser can be set with
`GirderFileBrowserDialog::setItemMode()`. The parameter is a `QString`, and it can be one of the following:
`Treat Items as Files` (default), `Treat Items as Folders`, or `Treat Items as Folders with File Bumping`. For
a slow internet, `Treat Items as Folders with File Bumping` may be noticeably slower because it must make an
additional api call for each item that is in the new directory.

Optionally, a list of choosable types may be set in the browser with
`GirderFileBrowserDialog::setChoosableTypes()`. This is a list of strings that resemble types that may
be chosen. This can be useful if the dialog is to be used to, for instance, have the user choose an
item on the girder file system. If `GirderFileBrowserDialog::setChoosableTypes()` is set, the only
types that will appear in the browser window are folder types and the choosable types. To obtain the
information about the object that the user chose, use Qt to connect to the signal
`GirderFileBrowserDialog::objectChosen()`, which will be emitted with a map of the following keys:
`name`, `id`, and `type`. The following is the list of choosable types by default: "root", "Users",
"Collections", "user", "collection", "folder", "item", and "file". Each choosable type that is set
must be a part of this list.

Once the Girder File Browser Dialog has been constructed and the api key and girder token are set,
`GirderFileBrowserDialog::begin()` should be called. This simply changes the folder to either the
default root folder or the custom root folder.

After `GirderFileBrowserDialog::begin()` has been called, `GirderFileBrowserDialog::show()` may be
called to display the dialog.
