Develop SMTK with Git
=========================

This page documents how to develop SMTK through [Git][].
See the [README](README.md) for more information.

[Git]: http://git-scm.com

Git is an extremely powerful version control tool that supports many
different "workflows" for individual development and collaboration.
Here we document procedures used by the SMTK development community.
In the interest of simplicity and brevity we do *not* provide an
explanation of why we use this approach.

Setup
-----

Before you begin, perform initial setup:

1.  Register [GitLab Access][] to create an account and select a user name.

2.  [Fork SMTK][] into your user's namespace on GitLab.

3.  Follow the [download instructions](download.md#clone) to create a
    local clone of the main SMTK repository.  Optionally configure
    Git to [use SSH instead of HTTPS](download.md#use-ssh-instead-of-https).
    Then clone:

        $ git clone --recursive https://gitlab.kitware.com/cmb/smtk.git SMTK
        $ cd SMTK
    The main repository will be configured as your `origin` remote.

4.  Run the [developer setup script][] to prepare your SMTK work tree and
    create Git command aliases used below:

        $ ./utilities/SetupForDevelopment.sh
    This will prompt for your GitLab user name and configure a remote
    called `gitlab` to refer to it.

5.  (Optional but highly recommended.)
    [Register](https://open.cdash.org/register.php) with the SMTK project
    on Kitware's CDash instance to better know how your code performs in
    regression tests.  After registering and signing in, click on
    "All Dashboards" link in the upper left corner, scroll down and click
    "Subscribe to this project" on the right of SMTK.

[GitLab Access]: https://gitlab.kitware.com/users/sign_in
[Fork SMTK]: https://gitlab.kitware.com/cmb/smtk/forks/new
[developer setup script]: ../../utilities/SetupForDevelopment.sh

Workflow
--------

SMTK development uses a [branchy workflow][] based on topic branches.
Our collaboration workflow consists of three main steps:

1.  Local Development:
    * [Update](#update)
    * [Create a Topic](#create-a-topic)

2.  Code Review (requires [GitLab Access][]):
    * [Share a Topic](#share-a-topic)
    * [Create a Merge Request](#create-a-merge-request)
    * [Review a Merge Request](#review-a-merge-request)
    * [Revise a Topic](#revise-a-topic)

3.  Integrate Changes:
    * [Merge a Topic](#merge-a-topic) (requires permission in GitLab)
    * [Delete a Topic](#delete-a-topic)

[branchy workflow]: http://public.kitware.com/Wiki/Git/Workflow/Topic

Update
------

1.  Update your local `master` branch:

        $ git checkout master
        $ git pullall

2.  Optionally push `master` to your fork in GitLab:

        $ git push gitlab master

    to keep it in sync.

Create a Topic
--------------

All new work must be committed on topic branches.
Name topics like you might name functions: concise but precise.
A reader should have a general idea of the feature or fix to be developed given
just the branch name. Additionally, it is preferred to have an issue associated with
every topic. The issue can document the bug or feature to be developed. In such
cases, being your topic name with the issue number.

1.  To start a new topic branch:

        $ git fetch origin

    If there is an issue associated with the topic, assign the issue to yourself
    using the "**Assignee**" field, and add the
    `workflow:active-development` label to it.

2.  For new development, start the topic from `origin/master`:

        $ git checkout -b my-topic origin/master

    If subdmodules may have changed, the  run:

        $ git submodule update

3.  Edit files and create commits (repeat as needed):

        $ edit file1 file2 file3
        $ git add file1 file2 file3
        $ git commit

    Commit messages must contain a brief description as the first line
    and a more detailed description of what the commit contains. If
    the commit contains a new feature, the detailed message must
    describe the new feature and why it is needed. If the commit
    contains a bug fix, the detailed message must describe the bug
    behavior, its underlying cause, and the approach to fix it. If the
    bug is described in the bug tracker, the commit message must
    contain a reference to the bug number.

Share a Topic
-------------

When a topic is ready for review and possible inclusion, share it by pushing
to a fork of your repository in GitLab.  Be sure you have registered and
signed in for [GitLab Access][] and created your fork by visiting the main
[SMTK GitLab][] repository page and using the "Fork" button in the upper right.

[SMTK GitLab]: https://gitlab.kitware.com/cmb/smtk

1.  Checkout the topic if it is not your current branch:

        $ git checkout my-topic

2.  Check what commits will be pushed to your fork in GitLab:

        $ git prepush

3.  Push commits in your topic branch to your fork in GitLab:

        $ git push gitlab HEAD

    Notes:
    * If you are revising a previously pushed topic and have rewritten the
      topic history, add `-f` or `--force` to overwrite the destination.

    The output will include a link to the topic branch in your fork in GitLab
    and a link to a page for creating a Merge Request.

Create a Merge Request
----------------------

(If you already created a merge request for a given topic and have reached
this step after revising it, skip to the [next step](#review-a-merge-request).)

Visit your fork in GitLab, browse to the "**Merge Requests**" link on the
left, and use the "**New Merge Request**" button in the upper right to
reach the URL printed at the end of the [previous step](#share-a-topic).
It should be of the form:

    https://gitlab.kitware.com/<username>/smtk/merge_requests/new

Follow these steps:

1.  In the "**Source branch**" box select the `<username>/smtk` repository
    and the `my-topic` branch.

2.  In the "**Target branch**" box select the `cmb/smtk` repository and
    the `master` branch.  It should be the default.

3.  Use the "**Compare branches**" button to proceed to the next page
    and fill out the merge request creation form.

4.  In the "**Title**" field provide a one-line summary of the entire
    topic.  This will become the title of the Merge Request.

    Example Merge Request Title:

        Wrapping: Add OpenCascade 1.x support

5.  In the "**Description**" field provide a high-level description
    of the change the topic makes and any relevant information about
    how to try it.
    *   Use `@username` syntax to draw attention of specific developers.
        This syntax may be used anywhere outside literal text and code
        blocks.  Or, wait until the [next step](#review-a-merge-request)
        and add comments to draw attention of developers.
    *   Optionally use a fenced code block with type `message` to specify
        text to be included in the generated merge commit message when the
        topic is [merged](#merge-a-topic).

    Example Merge Request Description:

        This branch requires OpenCascade 1.x which is not generally available yet.
        Get OpenCascade 1.x from ... in order to try these changes.

        ```message
        Add support for OpenCascade 1.x to the model infrastructure.
        ```

        Cc: @user1 @user2

6.  The "**Assign to**", "**Milestone**", and "**Labels**" fields
    may be left blank.

7.  Use the "**Submit merge request**" button to create the merge request
    and visit its page.

Review a Merge Request
----------------------

Add comments mentioning specific developers using `@username` syntax to
draw their attention and have the topic reviewed.  After typing `@` and
some text, GitLab will offer completions for developers whose real names
or user names match.

Comments use [GitLab Flavored Markdown][] for formatting.  See GitLab
documentation on [Special GitLab References][] to add links to things
like merge requests and commits in other repositories.

When a merge request is ready for review, developers can use the
`triage:ready-for-review` to indicate the same to the reviewers. If reviewers
deem that it needs more work, they can add the `triage:merge-needs-work` label.
This can be repeated as many times as needed adding/removing labels as
appropriate.

If a merge request is waiting on dashboards, use the `triage:pending-dashboards`
label.

[GitLab Flavored Markdown]: https://gitlab.kitware.com/help/user/markdown.md
[Special GitLab References]: https://gitlab.kitware.com/help/user/markdown.md#special-gitlab-references

### Human Reviews ###

Reviewers may add comments providing feedback or to acknowledge their
approval.  Lines of specific forms will be extracted during
[merging](#merge-a-topic) and included as trailing lines of the
generated merge commit message:

The *leading* line of a comment may optionally be exactly one of the
following votes followed by nothing but whitespace before the end
of the line:

*   `-1` or :-1: (`:-1:`) means "The change is not ready for integration."
*   `+1` or :+1: (`:+1:`) means "I've reviewed the source changes and they look good."
*   `+2` means "I've reviewed and compiled the changes and they look good."
*   `+3` means "I have tested the change and verified it works."

**Note:** In the case of large commits, several reviewers may be involved.  In these cases each reviewer should document which section of the commit their comments pertain to.

The middle lines of a comment may be free-form [GitLab Flavored Markdown][].

Zero or more *trailing* lines of a comment may each contain exactly one
of the following votes followed by nothing but whitespace before the end
of the line:

*   `Rejected-by: me` means "The change is not ready for integration."
*   `Acked-by: me` means "I've reviewed the source changes and they look good."
*   `Reviewed-by: me` means "I've reviewed and compiled the changes and they look good."
*   `Tested-by: me` means "I have tested the change and verified it works."

Each `me` reference may instead be an `@username` reference or a full
`Real Name <user@domain>` reference to credit someone else for performing
the review.  References to `me` and `@username` will automatically be
transformed into a real name and email address according to the user's
GitLab account profile.

#### Fetching Changes ####

One may fetch the changes associated with a merge request by using
the `git fetch` command line shown at the top of the Merge Request
page.  It is of the form:

    $ git fetch https://gitlab.kitware.com/$username/cmb.git $branch

This updates the local `FETCH_HEAD` to refer to the branch.

There are a few options for checking out the changes in a work tree:

*   One may checkout the branch:

        $ git checkout FETCH_HEAD -b $branch
    or checkout the commit without creating a local branch:

        $ git checkout FETCH_HEAD

*   Or, one may cherry-pick the commits to minimize rebuild time:

        $ git cherry-pick ..FETCH_HEAD

### Robot Reviews ###

The "Kitware Robot" automatically performs basic checks on the commits
and adds a comment acknowledging or rejecting the topic.  This will be
repeated automatically whenever the topic is pushed to your fork again.
A re-check may be explicitly requested by adding a comment with a single
[*trailing* line](#trailing-lines):

    Do: check

A topic cannot be [merged](#merge-a-topic) until the automatic review
succeeds.

### Testing ###

CMB has a [buildbot](http://buildbot.net) instance watching for merge requests
to test.  A developer must issue a command to buildbot to enable builds:

    Do: test

The buildbot user (@buildbot) will respond with a comment linking to the CDash
results when it schedules builds.

The `Do: test` command accepts the following arguments:

  * `--oneshot`
        only build the *current* hash of the branch; updates will not be built
        using this command
  * `--stop`
        clear the list of commands for the merge request
  * `--superbuild`
        build the superbuilds related to the project
  * `--clear`
        clear previous commands before adding this command
  * `--regex-include <arg>` or `-i <arg>`
        only build on builders matching `<arg>` (a Python regular expression)
  * `--regex-exclude <arg>` or `-e <arg>`
        excludes builds on builders matching `<arg>` (a Python regular
        expression)

Multiple `Do: test` commands may be given. Upon each update to the branch,
buildbot will reconsider all of the active commands to determine which builders
to schedule.

Builder names always follow this pattern:

        project-host-os-libtype-buildtype+feature1+feature2

  * project: always `cmb` for cmb
  * host: the buildbot host
  * os: one of `windows`, `osx`, or `linux`
  * libtype: `shared` or `static`
  * buildtype: `release` or `debug`
  * feature: alphabetical list of features enabled for the build

For a list of all builders, see:

  * [smtk-expected](https://buildbot.kitware.com/builders?category=smtk-expected)
  * [smtk-superbuild](https://buildbot.kitware.com/builders?category=smtk-superbuild)
  * [smtk-experimental](https://buildbot.kitware.com/builders?category=smtk-experimental)

Revise a Topic
--------------

If a topic is approved during GitLab review, skip to the
[next step](#merge-a-topic).  Otherwise, revise the topic
and push it back to GitLab for another review as follows:

1.  Checkout the topic if it is not your current branch:

        $ git checkout my-topic

2.  To revise the 3<sup>rd</sup> commit back on the topic:

        $ git rebase -i HEAD~3

    (Substitute the correct number of commits back, as low as `1`.)
    Follow Git's interactive instructions.

3.  Return to the [above step](#share-a-topic) to share the revised topic.

Merge a Topic
-------------

After a topic has been reviewed and approved in a GitLab Merge Request,
authorized developers may add a comment of the form

    Do: merge

to ask that the change be merged into the upstream repository.  By
convention, do not request a merge if any `-1` or `Rejected-by:`
review comments have not been resolved and superseded by at least
`+1` or `Acked-by:` review comments from the same user.

Developers are encouraged to merge their own merge requests on review. However,
please do not merge unless you are available to address any dashboard issues that may
arise. Developers who repeatedly ignore dashboard issues following their merges may
loose developer privileges to the repository temporarily (or permanently)!

### Merge Success ###

If the merge succeeds the topic will appear in the upstream repository
`master` branch and the Merge Request will be closed automatically.
Any issues associated with the Merge Request will generally get closed
automatically. If not, the developer merging the changes should close such issues
and add a `workflow:customer-review` tag to the issue(s) addressed by the change.
Reporters and testers can then review the fix. Try to add enough information to
the Issue or the Merge Request to indicate how to test the functionality if not
obvious from the original Issue.

### Merge Failure ###

If the merge fails (likely due to a conflict), a comment will be added
describing the failure.  In the case of a conflict, fetch the latest
upstream history and rebase on it:

    $ git fetch origin
    $ git rebase origin/master

Return to the [above step](#share-a-topic) to share the revised topic.

Delete a Topic
--------------

After a topic has been merged upstream the Merge Request will be closed.
Now you may delete your copies of the branch.

1.  In the GitLab Merge Request page a "**Remove Source Branch**"
    button will appear.  Use it to delete the `my-topic` branch
    from your fork in GitLab.

2.  In your work tree checkout and update the `master` branch:

        $ git checkout master
        $ git pull

3.  Delete the local topic branch:

        $ git branch -d my-topic

    The `branch -d` command works only when the topic branch has been
    correctly merged.  Use `-D` instead of `-d` to force the deletion
    of an unmerged topic branch (warning - you could lose commits).

Contributing CMB, VTK or ParaView Changes
----------------------

If you have any CMB, VTK or ParaView changes, then you are required to get your changes
incorporated into CMB using [CMB's development workflow][], VTK using [VTK's development workflow][] and/or into ParaView using [ParaView's development workflow][].

[CMB's development workflow]: https://gitlab.kitware.com/cmb/cmb/tree/master/Documentation/dev
[VTK's development workflow]: https://gitlab.kitware.com/vtk/vtk/tree/master/Documentation/dev/git
[ParaView's development workflow]: https://gitlab.kitware.com/paraview/paraview/tree/master/Documentation/dev/git
