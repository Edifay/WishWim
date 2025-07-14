[
  "("
  ")"
  "{"
  "}"
  ] @property

[
  ":"
  "&:"
  "::"
  "|"
  ";"
  "\""
  "'"
  ","
  ] @property

[
  "$"
  "$$"
  ] @property

(automatic_variable
  ["@" "%" "<" "?" "^" "+" "/" "*" "D" "F"] @property)

(automatic_variable
  "/" @error . ["D" "F"])

[
  "="
  ":="
  "::="
  "?="
  "+="
  "!="
  "@"
  "-"
  "+"
  ] @property

[
  (text)
  (string)
  (raw_text)
  ] @string

(variable_assignment (word) @string)

[
  "ifeq"
  "ifneq"
  "ifdef"
  "ifndef"
  "else"
  "endif"
  "if"
  "or"  ; boolean functions are conditional in make grammar
  "and"
  ] @keyword

"foreach" @keyword

[
  "define"
  "endef"
  "vpath"
  "undefine"
  "export"
  "unexport"
  "override"
  "private"
  ; "load"
  ] @keyword

[
  "include"
  "sinclude"
  "-include"
  ] @keyword

[
  "subst"
  "patsubst"
  "strip"
  "findstring"
  "filter"
  "filter-out"
  "sort"
  "word"
  "words"
  "wordlist"
  "firstword"
  "lastword"
  "dir"
  "notdir"
  "suffix"
  "basename"
  "addsuffix"
  "addprefix"
  "join"
  "wildcard"
  "realpath"
  "abspath"
  "call"
  "eval"
  "file"
  "value"
  "shell"
  ] @keyword

[
  "error"
  "warning"
  "info"
  ] @keyword

;; Variable
(variable_assignment
  name: (word) @constant)

(variable_reference
  (word) @constant)

(comment)+ @comment

((word) @property
  (#match? @property "[%\*\?]")
  (#eq? @property "test"))

(function_call
  function: "error"
  (arguments (text) @property))

(function_call
  function: "warning"
  (arguments (text) @property))

(function_call
  function: "info"
  (arguments (text) @property))

;; Install Command Categories
;; Others special variables
;; Variables Used by Implicit Rules
[
  "VPATH"
  ".RECIPEPREFIX"
  ] @constant

(variable_assignment
  name: (word) @constant
  (#match? @constant "^(AR|AS|CC|CXX|CPP|FC|M2C|PC|CO|GET|LEX|YACC|LINT|MAKEINFO|TEX|TEXI2DVI|WEAVE|CWEAVE|TANGLE|CTANGLE|RM|ARFLAGS|ASFLAGS|CFLAGS|CXXFLAGS|COFLAGS|CPPFLAGS|FFLAGS|GFLAGS|LDFLAGS|LDLIBS|LFLAGS|YFLAGS|PFLAGS|RFLAGS|LINTFLAGS|PRE_INSTALL|POST_INSTALL|NORMAL_INSTALL|PRE_UNINSTALL|POST_UNINSTALL|NORMAL_UNINSTALL|MAKEFILE_LIST|MAKE_RESTARTS|MAKE_TERMOUT|MAKE_TERMERR|\.DEFAULT_GOAL|\.RECIPEPREFIX|\.EXTRA_PREREQS)$"))

(variable_reference
  (word) @constant
  (#match? @constant "^(AR|AS|CC|CXX|CPP|FC|M2C|PC|CO|GET|LEX|YACC|LINT|MAKEINFO|TEX|TEXI2DVI|WEAVE|CWEAVE|TANGLE|CTANGLE|RM|ARFLAGS|ASFLAGS|CFLAGS|CXXFLAGS|COFLAGS|CPPFLAGS|FFLAGS|GFLAGS|LDFLAGS|LDLIBS|LFLAGS|YFLAGS|PFLAGS|RFLAGS|LINTFLAGS|PRE_INSTALL|POST_INSTALL|NORMAL_INSTALL|PRE_UNINSTALL|POST_UNINSTALL|NORMAL_UNINSTALL|MAKEFILE_LIST|MAKE_RESTARTS|MAKE_TERMOUT|MAKE_TERMERR|\.DEFAULT_GOAL|\.RECIPEPREFIX|\.EXTRA_PREREQS\.VARIABLES|\.FEATURES|\.INCLUDE_DIRS|\.LOADED)$"))

(targets) @function

;; Standart targets
(targets
  (word) @function
  (#match? @function "^(all|install|install-html|install-dvi|install-pdf|install-ps|uninstall|install-strip|clean|distclean|mostlyclean|maintainer-clean|TAGS|info|dvi|html|pdf|ps|dist|check|installcheck|installdirs)$"))

(targets
  (word) @function
  (#match? @function "^(all|install|install-html|install-dvi|install-pdf|install-ps|uninstall|install-strip|clean|distclean|mostlyclean|maintainer-clean|TAGS|info|dvi|html|pdf|ps|dist|check|installcheck|installdirs)$"))

;; Builtin targets
(targets
  (word) @constant
  (#match? @constant "^\.(PHONY|SUFFIXES|DEFAULT|PRECIOUS|INTERMEDIATE|SECONDARY|SECONDEXPANSION|DELETE_ON_ERROR|IGNORE|LOW_RESOLUTION_TIME|SILENT|EXPORT_ALL_VARIABLES|NOTPARALLEL|ONESHELL|POSIX)$"))

