add_executable(omnilyzer ${OMNISCIO_SOURCE_DIR}/src/sequitur/analyzer.cpp
			${OMNISCIO_SOURCE_DIR}/src/sequitur/oracle.cpp
			${OMNISCIO_SOURCE_DIR}/src/sequitur/rules.cpp
			${OMNISCIO_SOURCE_DIR}/src/sequitur/symbols.cpp)
target_link_libraries(omnilyzer)
