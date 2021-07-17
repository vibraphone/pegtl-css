# PEGTL CSS Parser

This is a draft CSS parser written in C++ using the PEGTL 2.x library
(because a dependent project does not want to move to C++17 that
PEGTL 3.x requires).

Once you configure and build the project, run
```sh
./parse-css file.css
```
and you will see a stream of all the grammar matching done.
It is not pretty, but it allows hand-validation and debugging.

Turn off `DBG_PARSE` in `parse-css.cxx` and it will not
print these matches as it goes.
Instead, it will print a summary at the end and information
about how many rulesets and properties it encountered (which
is not perfectly computed at this point).

Turn off `DBG_GRAMMAR` in `parse-css.cxx` and it will not
invoke PEGTL's grammar analysis (cycle detection) before
parsing.
