# Workflow

Divide major tasks into more concrete sub-tasks (new features, refactoring, bug-fixes, etc) and create a github issue for each sub-task. Create separate issues for
each sub-task e.g. implementing a new feature and major refactoring should be two separate issues.  
Use the following format to create issues:

```
==== Title ====
[Sub-task]: Rough title explaining the task at hand

==== Body ====
# Purpose
Explain the behaviour/structure of the current program and the proposed fix in plain english (a few sentences).

# Required Actions
- The fix/change in more concrete terms

# Deliverables
- Expected change in the behaviour/structure of the program in more concrete terms
```

`issue_example.png` is an example of what an issue should look like.

Assign the issue(s) you want to work on to yourself and create a new branch before starting any work. Reference the relevant issue # in each commit message.

Ideally assign merge/pull requests to someone else so everyone comes into contact with the entire codebase.

# Compilation

The client and server depend on `socketutil.c`. Compile with (at least) the following flags: `-Wall`, `-Werror`, `-Wpedantic` and
add the `.out` extension to executables so they get gitignored.

I'm going to create a makefile soon.

# Formatting

Use `snake_case` for all var/method naming.
