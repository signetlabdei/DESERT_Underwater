# Commit rules

* The commit should follow this format

```
name_of_module: Summary of the commit (less than 50 chars!!!)
<blank line>
Longer description of commit with explanation of bug fix, feature
why you chose certain implementations, etc. etc. (should be less than 70 chars
for each line)

Fix #Issue
```

One practical example

```
ms2c_evologics: Fix socket segfault

This fix a strange segfault that happens on
socket creation between DESERT and MS2C modem
from midnight to 1 am

Fix #1234
```

* Try to always work with a reference to an issue (bug, feature, ..) on github because it helps to better understand the history
* Never commit directly on master. Always work in a separate branch, also for trivial fixes/changes
* Commit on master are allowed only for version tagging
* Try as much as possible to have feedback from the community (even though you are sure about your code and you know you can safely merge it)
* In your feature branch, try as much as possible to squash the commits before do the PR. The rule of thumb is:
  - If the feature brach introduces a new protocol/code, it should contain only one commit that add the first version of the code
  - One commit for each bug fix patch
  - This does not mean that you cannot commit often, it does mean that before do the PR you should squash the commit using `git rebase --i HEAD~<number-of-your-commits>`
