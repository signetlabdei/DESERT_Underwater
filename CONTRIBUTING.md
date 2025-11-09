# Contributing

We encourage contributions to `DESERT_Underwater` from the community.
There are some simple steps to follow.

First of all, fork the `DESERT_Underwater` repository on [Github](https://github.com/signetlabdei/DESERT_Underwater.git).
Clone your fork or add the remote if you already have a clone of the repository.

```console
$ git clone git@github.com:yourusername/DESERT_Underwater.git
```

or

```console
$ git remote add mine_remote git@github.com:yourusername/DESERT_Underwater.git
```

Then, create a branch for your improvements, bug fix, new modules, etc. 

```console
$ git checkout -b your-branch
```

To maintain consistency throughout the repository a coding style is enforced by the maintainers, which can be found in the [coding-rules document](coding_rules.md).

Make your changes and commit. Use a clear and descriptive commit message. To maintain a well documented trace of changes a committing style is also enforced by the maintainers.
A commit should follow this format:

```
name_of_module: Summary of the commit (less than 50 chars!!!)
<blank line>
Longer description of commit with explanation of bug fix, feature
why you chose certain implementations, etc. etc. (should be less than 70 chars
for each line)

Fix #Issue
```

One practical example

```console
$ git commit -m 'Module A: added this wonderful feature!' -m 'This feature allows to stop the time and fix wrong things. User can then restart the time. Fix #Issue'
```
      
Finally, push to your fork of the repository and then send a pull-request through Github.

```console
$ git push mine some-topic-branch
```

A community maintainer will review your pull request and merge it into the main repository or send you feedback.

Common useful rules to remember:
* Try to always work with a reference to an issue (bug, feature, ..) on github because it helps to better understand the history
* Never commit directly on master. Always work in a separate branch, also for trivial fixes/changes
* In your feature branch, try as much as possible to squash the commits before do the PR. The rule of thumb is:
  - If the feature brach introduces a new protocol/code, it should contain only one commit that add the first version of the code
  - One commit for each bug fix patch
  - This does not mean that you cannot commit often, it does mean that before do the PR you should squash the commit using `git rebase --i HEAD~<number-of-your-commits>`
