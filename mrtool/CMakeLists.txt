
set(LIB_HEADER
	mr_time.h
	mr_buffer.h
	mr_code.h
)

set(LIB_SRC
	mr_buffer.c
)

list(APPEND LIB_SRC ${LIB_HEADER})
add_library(mrtool STATIC ${LIB_SRC})
