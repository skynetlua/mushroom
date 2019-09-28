
set(LIB_HEADER
	atomic.h
	socket_info.h
	socket_poll.h
	spinlock.h
	socket_epoll.h   
	socket_kqueue.h  
	socket_select.h  
	socket_server.h
	mr_slist.h
	mr_socket.h
	mr_config.h
)

set(LIB_SRC
	socket_server.c
	mr_socket.c
	mr_mem.c
)

#mr_time.h
#mr_buffer.h
#mr_code.h
#mr_buffer.c

if(WIN32)

list(APPEND LIB_HEADER win/spinlock.h)
list(APPEND LIB_HEADER win/winsocket.h)
list(APPEND LIB_HEADER win/winsocketdef.h)
list(APPEND LIB_HEADER win/atomic.h)
list(APPEND LIB_HEADER win/winport.h)

list(APPEND LIB_SRC win/winport.c)
list(APPEND LIB_SRC win/winsocket.c)

link_libraries(ws2_32)

else()

link_libraries(pthread)

endif()

list(APPEND LIB_SRC ${LIB_HEADER})

add_library(mrsocket STATIC ${LIB_SRC})

