# Contributing

This is a European Spallation Source (ESS) project. If you are not an
ESS employee or ESS partner, you should first get in touch with the main contributors regarding specific changes.

### Issues
The workflow starts with the creation of a ticket in either the ESS JIRA
system or you can use github issues. You will need permissions to access
these.

### Branching

 * Branch your feature off 'master'
 * Create pull requests against 'master'.

### Branch naming
The names should start with the ticket number and contain a brief description.
For example:

 * `DM-1014_byebye_dead_code` for a (ESS) JIRA issue
 * `issue343_1014_byebye_dead_code` for a github issue

### Pull requests
There is a template for pull requests. This should contain enough information for the reviewer to be able to review the code efficiently.

## Code

### Style
We use `clang-format` v3.9 LLVM default style.
We also follow the LLVM coding standards for naming conventions etc. and use
Doxygen for source code documentation.

Please refer to [LLVM documentation](https://llvm.org/docs/CodingStandards.html).

### Unit tests
Unit tests should be written/modified for any code added or changed (within
reason, of course).

### System tests
System tests should be written/modified for any changes that affect the "public"
API of the application, i.e. anything that affects another component of the data
streaming pipeline.
