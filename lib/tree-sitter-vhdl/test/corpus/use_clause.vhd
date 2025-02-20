================================================================================
Various edge-cases in the use clause
================================================================================

use ieee.math_real.all;
use ieee.math_real.sin;
use work.whatnot.all;
use work.whatnot.func;
use work.sin.func.all;
use my_work.whatnot.func;
use my_work.func;
use my_work.all;

--------------------------------------------------------------------------------

(design_file
  (design_unit
    (use_clause
      (selected_name_list
        (selected_name
          library: (library_namespace)
          package: (identifier)
          (ALL))))
    (use_clause
      (selected_name_list
        (selected_name
          library: (library_namespace)
          package: (identifier)
          (library_function))))
    (use_clause
      (selected_name_list
        (selected_name
          library: (library_namespace)
          package: (identifier)
          (ALL))))
    (use_clause
      (selected_name_list
        (selected_name
          library: (library_namespace)
          package: (identifier)
          (identifier))))
    (use_clause
      (selected_name_list
        (selected_name
          library: (library_namespace)
          package: (identifier)
          (identifier)
          (ALL))))
    (use_clause
      (selected_name_list
        (selected_name
          library: (identifier)
          package: (identifier)
          (identifier))))
    (use_clause
      (selected_name_list
        (selected_name
          library: (identifier)
          package: (identifier))))
    (use_clause
      (selected_name_list
        (selected_name
          library: (identifier)
          (ALL))))))

