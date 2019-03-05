set(PSRC AdHocConnection.cpp
	MavLinkConnection.cpp
	MavLinkFtpClient.cpp
	MavLinkLog.cpp
	MavLinkMessageBase.cpp
	MavLinkMessages.cpp
	MavLinkNode.cpp
	MavLinkTcpServer.cpp
	MavLinkVehicle.cpp
	MavLinkVideoStream.cpp
	Semaphore.cpp
	impl/AdHocConnectionImpl.cpp
	impl/MavLinkConnectionImpl.cpp
	impl/MavLinkFtpClientImpl.cpp
	impl/MavLinkNodeImpl.cpp
	impl/MavLinkTcpServerImpl.cpp
	impl/MavLinkVehicleImpl.cpp
	impl/MavLinkVideoStreamImpl.cpp
	impl/linux/MavLinkFindSerialPorts.cpp
	impl/onecore/OneCoreFindSerialPorts.cpp
	impl/windows/WindowsFindSerialPorts.cpp
	serial_com/SerialPort.cpp
	serial_com/SocketInit.cpp
	serial_com/TcpClientPort.cpp
	serial_com/UdpClientPort.cpp
	serial_com/wifi.cpp
)

add_library (MavLinkCom ${PSRC})

target_link_libraries(MavLinkCom MavLinkComFileSystem)
