
function(apply_keys variable)
  set(keys "${${variable}}")

  set(keys1 "-pedantic -pedantic-errors -Wall -Weffc++ -Wextra -Werror")
  set(keys2 "-Wcast-align -Wold-style-cast -Wconversion -Wsign-conversion -Wcast-qual") 
  set(keys3 "-Woverloaded-virtual -Wctor-dtor-privacy -Wnon-virtual-dtor") 
  set(keys4 "-Winit-self -Wunreachable-code -Wunused-parameter -Wshadow")
  set(keys5 "-Wpointer-arith -Wreturn-type -Wswitch -Wformat -Wundef")
  set(keys6 "-Wwrite-strings -Wchar-subscripts -Wredundant-decls")
  set(keys7 "-Wparentheses -Wmissing-include-dirs -Wempty-body -Wextra")
  set(keys "${keys} ${keys1} ${keys2} ${keys3} ${keys4} ${keys5} ${keys6} ${keys7}")

  set(${variable} "${keys}" PARENT_SCOPE)
endfunction()

function(set_compiler_keys)

   set(CMAKE_CXX_FLAGS_RELEASE "${pre_keys} ${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG              ${post_keys}")
   set(CMAKE_CXX_FLAGS_DEBUG   "${pre_keys} ${CMAKE_CXX_FLAGS_DEBUG}   -O0 -g3 -DDEBUG -D_DEBUG  ${post_keys}")
   set(CMAKE_CXX_FLAGS         "${pre_keys} ${CMAKE_CXX_FLAGS}                                   ${post_keys}")

   set(CMAKE_C_FLAGS_RELEASE   "${pre_keys} ${CMAKE_C_FLAGS_RELEASE}   -O3 -DNDEBUG              ${post_keys}")
   set(CMAKE_C_FLAGS_DEBUG     "${pre_keys} ${CMAKE_C_FLAGS_DEBUG}     -O0 -g3 -DDEBUG -D_DEBUG  ${post_keys}")
   set(CMAKE_C_FLAGS           "${pre_keys} ${CMAKE_C_FLAGS}                                     ${post_keys}")

   apply_keys(CMAKE_C_FLAGS)
   apply_keys(CMAKE_CXX_FLAGS)
   foreach(cfg ${CMAKE_CONFIGURATION_TYPES})
     string(TOUPPER "${cfg}" CONFIG)
     apply_keys(CMAKE_CXX_FLAGS_${CONFIG})
     apply_keys(CMAKE_C_FLAGS_${CONFIG})
   endforeach()

   set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}" PARENT_SCOPE)
   set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}"   PARENT_SCOPE)
   set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS}"         PARENT_SCOPE)

   set(CMAKE_C_FLAGS_RELEASE   "${CMAKE_C_FLAGS_RELEASE}"   PARENT_SCOPE)
   set(CMAKE_C_FLAGS_DEBUG     "${CMAKE_C_FLAGS_DEBUG}"     PARENT_SCOPE)
   set(CMAKE_C_FLAGS           "${CMAKE_C_FLAGS}"           PARENT_SCOPE)

endfunction()

macro(set_standart_cxx)
  if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.25")  
    set(CMAKE_CXX_STANDARD 26)
  elseif(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.20")
    set(CMAKE_CXX_STANDARD 23)
  elseif(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.12")
    set(CMAKE_CXX_STANDARD 20)
  elseif(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.08")
    set(CMAKE_CXX_STANDARD 17)
  else()
    set(CMAKE_CXX_STANDARD 14)
  endif()

  set(CMAKE_CXX_EXTENSIONS        OFF)
  set(CMAKE_CXX_STANDARD_REQUIRED OFF)
endmacro()

set_compiler_keys()
set_standart_cxx()
