cmake_minimum_required (VERSION 2.8)

project (MavLinkCom)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread -g -O0")


set(OSDK
	../osdk-core/api/src/ 
	../osdk-core/api/inc 
	../osdk-core/protocol/inc
        ../osdk-core/hal/inc
        ../osdk-core/utility/inc
        ../osdk-core/platform/linux/inc
)

include_directories (
	${PROJECT_SOURCE_DIR}/include
	${PROJECT_SOURCE_DIR}/mavlink
	${PROJECT_SOURCE_DIR}/mavlink/common
	${PROJECT_SOURCE_DIR}/common_utils
	${PROJECT_SOURCE_DIR}/src
	${PROJECT_SOURCE_DIR}/src/impl
	${PROJECT_SOURCE_DIR}/src/serial_com
	#${PROJECT_SOURCE_DIR}/MavLinkTest




	${PROJECT_SOURCE_DIR}/../sample/linux/flight-control/
	${PROJECT_SOURCE_DIR}/../sample/linux/common/




	${PROJECT_SOURCE_DIR}/../osdk-core/api/inc/
	${PROJECT_SOURCE_DIR}/../osdk-core/api/src/

	${PROJECT_SOURCE_DIR}/../osdk-core/protocol/inc/
        ${PROJECT_SOURCE_DIR}/../osdk-core/hal/inc/
        ${PROJECT_SOURCE_DIR}/../osdk-core/utility/inc/
        ${PROJECT_SOURCE_DIR}/../osdk-core/platform/linux/inc/

	${PROJECT_SOURCE_DIR}/../osdk-core/protocol/src/
        ${PROJECT_SOURCE_DIR}/../osdk-core/hal/src/
        ${PROJECT_SOURCE_DIR}/../osdk-core/utility/src/
        ${PROJECT_SOURCE_DIR}/../osdk-core/platform/linux/src/
	
	
)

set(PSRC
	common_utils/FileSystem.cpp
	common_utils/ThreadUtils.cpp 
 	src/AdHocConnection.cpp
	src/MavLinkConnection.cpp
	src/MavLinkFtpClient.cpp
	src/MavLinkLog.cpp
	src/MavLinkMessageBase.cpp
	src/MavLinkMessages.cpp
	src/MavLinkNode.cpp
	src/MavLinkTcpServer.cpp
	src/MavLinkVehicle.cpp
	src/MavLinkVideoStream.cpp
	src/Semaphore.cpp
	src/impl/AdHocConnectionImpl.cpp
	src/impl/MavLinkConnectionImpl.cpp
	src/impl/MavLinkFtpClientImpl.cpp
	src/impl/MavLinkNodeImpl.cpp
	src/impl/MavLinkTcpServerImpl.cpp
	src/impl/MavLinkVehicleImpl.cpp
	src/impl/MavLinkVideoStreamImpl.cpp
	src/impl/linux/MavLinkFindSerialPorts.cpp
	src/impl/onecore/OneCoreFindSerialPorts.cpp
	src/impl/windows/WindowsFindSerialPorts.cpp
	src/serial_com/SerialPort.cpp
	src/serial_com/SocketInit.cpp
	src/serial_com/TcpClientPort.cpp
	src/serial_com/UdpClientPort.cpp
	src/serial_com/wifi.cpp
	src/UnitTests.cpp
	
	../sample/linux/flight-control/flight_control_sample.cpp

	../sample/linux/common/dji_linux_environment.cpp



)

add_library (MavLinkCom ${PSRC} ${OSDK})

