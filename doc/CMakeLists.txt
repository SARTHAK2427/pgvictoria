#
# Generate manual
#
if (DOCS)
  #
  # Generate API docs
  #
  if (DOXYGEN_FOUND)
    add_custom_target(api ALL)

    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_MAN NO)
    set(DOXYGEN_WARN_AS_ERROR FAIL_ON_WARNINGS)
    set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.doxygen)

    doxygen_add_docs(doxygen ${PROJECT_SOURCE_DIR}/src/include/)

    if (${DOXYGEN_VERSION} STREQUAL "1.10.0" OR
        ${DOXYGEN_VERSION} STREQUAL "1.11.0" OR
        ${DOXYGEN_VERSION} STREQUAL "1.12.0" OR
        ${DOXYGEN_VERSION} STREQUAL "1.13.0" OR
        ${DOXYGEN_VERSION} STREQUAL "1.13.1" OR
        ${DOXYGEN_VERSION} STREQUAL "1.13.2" OR
        ${DOXYGEN_VERSION} STREQUAL "1.14.0" OR
        ${DOXYGEN_VERSION} STREQUAL "1.15.0" OR
        ${DOXYGEN_VERSION} STREQUAL "1.16.0" OR
        ${DOXYGEN_VERSION} STREQUAL "1.16.1")

      add_custom_command(
        TARGET api
        COMMAND ${DOXYGEN_EXECUTABLE} -q ${DOXYFILE_OUT}
        COMMENT "Generating API documentation"
        POST_BUILD
      )
    else()
      add_custom_command(
        TARGET api
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
        COMMENT "Generating API documentation"
        POST_BUILD
      )
    endif()
  endif()
endif()
