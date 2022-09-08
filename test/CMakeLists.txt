add_executable(reader ${OMNISCIO_SOURCE_DIR}/test/reader.cpp
		      ${OMNISCIO_SOURCE_DIR}/src/sequitur/oracle.cpp
		      ${OMNISCIO_SOURCE_DIR}/src/sequitur/rules.cpp
		      ${OMNISCIO_SOURCE_DIR}/src/sequitur/symbols.cpp)

add_executable(test_tree ${OMNISCIO_SOURCE_DIR}/test/test_tree.cpp)
