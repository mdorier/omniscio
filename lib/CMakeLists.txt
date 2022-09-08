set(OMNISCIO_SRC
	${OMNISCIO_SOURCE_DIR}/src/omniscio.cpp
	${OMNISCIO_SOURCE_DIR}/src/trace.cpp
	${OMNISCIO_SOURCE_DIR}/src/mpi.cpp
	${OMNISCIO_SOURCE_DIR}/src/files.cpp
	${OMNISCIO_SOURCE_DIR}/src/sequitur/oracle.cpp
	${OMNISCIO_SOURCE_DIR}/src/sequitur/symbols.cpp
	${OMNISCIO_SOURCE_DIR}/src/sequitur/rules.cpp
	)

add_library(omniscio ${OMNISCIO_SRC})

set(OMNISCIO_POSIX_SRC
	${OMNISCIO_SOURCE_DIR}/src/posix.cpp
	)

add_library(omniscio-posix SHARED ${OMNISCIO_POSIX_SRC})
