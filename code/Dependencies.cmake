include(cmake/CPM.cmake)
# Done as a function so that updates to variables like
# CMAKE_CXX_FLAGS don't propagate out to other
# targets
function(ConstraintBasedSimulator_setup_dependencies)

  # For each dependency, see if it's
  # already been provided to us by a parent project

  if(NOT TARGET raylib)
    cpmaddpackage("gh:raysan5/raylib#5.0")
  endif()

  # TODO add criterion

endfunction()