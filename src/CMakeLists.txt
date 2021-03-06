# Setting headers just for showing them up in IDEs
file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${hydrogenWeb_SOURCE_DIR}/include/*.hpp")

# Will be static or dynamic based on user setting
add_library(core Timestamp.cpp Timer.cpp TimerQueue.cpp EventLoopThread.cpp
        EventLoopThreadPool.cpp HyBuffer.cpp ${HEADER_LIST})
add_library(io sio.cpp robustio.cpp ${HEADER_LIST})
add_library(network socketDetail.cpp EventLoop.cpp Channel.cpp Poller.cpp
        InetAddr.cpp Socket.cpp Acceptor.cpp TcpServer.cpp TcpConnection.cpp
        HttpRequest.cpp HttpResponse.cpp HttpContext.cpp
        HttpServer.cpp ${HEADER_LIST})
add_library(unixUtility unixUtility.cpp ${HEADER_LIST})
add_library(sem sem.cpp sharedBuf.cpp ${HEADER_LIST})

# Include
target_include_directories(core PUBLIC ../include)
target_include_directories(io PUBLIC ../include)
target_include_directories(network PUBLIC ../include)
target_include_directories(unixUtility PUBLIC ../include)
target_include_directories(sem PUBLIC ../include)

# link dependence
target_link_libraries(core PRIVATE unixUtility spdlog::spdlog)
target_link_libraries(network PRIVATE core unixUtility spdlog::spdlog)
target_link_libraries(unixUtility PRIVATE Threads::Threads spdlog::spdlog)

# Just c++17
target_compile_features(core PUBLIC cxx_std_17)
target_compile_features(io PUBLIC cxx_std_17)
target_compile_features(network PUBLIC cxx_std_17)
target_compile_features(unixUtility PUBLIC cxx_std_17)
target_compile_features(sem PUBLIC cxx_std_17)

# Sets compile flags
target_compile_options(core PUBLIC -Wall -Wextra -pedantic -pedantic-errors
        "$<$<CONFIG:RELEASE>:-O2>")
target_compile_options(io PUBLIC -Wall -Wextra -pedantic -pedantic-errors
        "$<$<CONFIG:RELEASE>:-O2>")
target_compile_options(network PUBLIC -Wall -Wextra -pedantic -pedantic-errors
        "$<$<CONFIG:RELEASE>:-O2>")
target_compile_options(unixUtility PUBLIC -Wall -Wextra -pedantic -pedantic-errors
        "$<$<CONFIG:RELEASE>:-O2>")
target_compile_options(sem PUBLIC -Wall -Wextra -pedantic -pedantic-errors
        "$<$<CONFIG:RELEASE>:-O2>")
