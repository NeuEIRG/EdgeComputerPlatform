// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "UnitTests.h"
#include <thread>
#include <chrono>
#include "Utils.hpp"
#include "FileSystem.hpp"
#include "MavLinkVehicle.hpp"
#include "MavLinkMessages.hpp"
#include "MavLinkConnection.hpp"
#include "MavLinkVideoStream.hpp"
#include "MavLinkTcpServer.hpp"
#include "MavLinkFtpClient.hpp"
#include "Semaphore.hpp"
#include <cstdio>
#include <string>





#pragma once
#include <string>
#include <functional>
#include "MavLinkFtpClient.hpp"

#include <flight_control_sample.hpp>
#include<iostream>
using namespace std;



















STRICT_MODE_OFF
#include "json.hpp"
STRICT_MODE_ON

#include <iostream>

using namespace mavlink_utils;
using namespace mavlinkcom;


extern std::string replaceAll(std::string s, char toFind, char toReplace);

void UnitTests::RunAll(std::string comPort, int boardRate)
{
	com_port_ = comPort;
	baud_rate_ = boardRate;
	if (comPort == "") {
		throw std::runtime_error("unit tests need a serial connection to Pixhawk, please specify -serial argument");
	}

	// RunTest("UdpPingTest", [=] { UdpPingTest(); });
	RunTest("TcpPingTest", [=] { TcpPingTest(); });
	// RunTest("SendImageTest", [=] { SendImageTest(); });
	// RunTest("SerialPx4Test", [=] { SerialPx4Test(); });
	// RunTest("FtpTest", [=] { FtpTest(); });
    // RunTest("JSonLogTest", [=] { JSonLogTest(); });
}

void UnitTests::RunTest(const std::string& name, TestHandler handler)
{
	printf("%s: start\n", name.c_str());
	try {
		handler();
		printf("------------- %s test passed --------------------\n", name.c_str());
	}
	catch (const std::exception& e) {
		printf("------------- %s test failed: %s --------------- \n", name.c_str(), e.what());
	}
}

void UnitTests::UdpPingTest() {
    // string localip;
    // std::cin>>localip>>std::endl;
    auto localConnection = MavLinkConnection::connectLocalUdp("jMavSim", "192.168.43.188" , 49339);
    std::cout<<"本地Con建立，等待远程连接..."<<std::endl;
    char r[100] = {0};
        Semaphore  received;
        std::cout<<"again"<<std::endl;
	    auto id = localConnection->subscribe([&](std::shared_ptr<MavLinkConnection> connection, const MavLinkMessage& msg)
        {
            if(msg.msgid == (int)MavLinkMessageIds::MAVLINK_MSG_ID_STATUSTEXT)
            {
                MavLinkStatustext statustext;
                statustext.decode(msg);
                if(strcmp(statustext.text,"end") == 0)
                {
                    std::cout<<"接收结束"<<std::endl;
                    memcpy(r,statustext.text,sizeof(statustext.text));
                    received.post();
                }
                else
                {
                printf("Message  is %s\n", statustext.text);
                }
            }
        });
    received.wait();
    std::cout<<"消息接收成功，关闭连接中..."<<std::endl;
    localConnection->unsubscribe(id);
    localConnection->close();
    std::cout<<"连接关闭成功"<<std::endl;
}

void UnitTests::TcpPingTest(DJI::OSDK::Vehicle* vehicle) {

//***************************************************************
//int argc=2;
//char *argv[]={"flight_control_sample","UserConfig.txt"};
/*
argv[0]="flight_control_sample";
argv[1]="UserConfig.txt";
*/


/*
// Initialize variables
  int functionTimeout = 1;
std::cout<<"Hello World!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<std::endl;
  // Setup OSDK.
  LinuxSetup linuxEnvironment(argc, argv);
  Vehicle* vehicle = linuxEnvironment.getVehicle();
  if (vehicle == NULL)
  {
    std::cout << "Vehicle not initialized, exiting.\n";
//    return -1;
  }

  // Obtain Control Authority
  vehicle->obtainCtrlAuthority(functionTimeout);

*/
//************************************************************










    char b[100]={0};
	const int testPort = 50043;
    std::cout<<"create local server"<<std::endl;
	std::shared_ptr<MavLinkTcpServer> server = std::make_shared<MavLinkTcpServer>("192.168.43.135", testPort);
    Semaphore  received;
    std::cout<<"create local client"<<std::endl;
	auto client = MavLinkConnection::connectTcp("local", "192.168.43.135", "192.168.43.88", 49044);
    std::cout<<"3"<<std::endl;
    	client->subscribe([&](std::shared_ptr<MavLinkConnection> connection, const MavLinkMessage& msg)
    {
        std::cout<<"4"<<std::endl;
        if(msg.msgid == (int)MavLinkMessageIds::MAVLINK_MSG_ID_STATUSTEXT)
            {
                MavLinkStatustext statustext;
                statustext.decode(msg);
                char c[50]={0};
                char d='0';
                strcpy(c,statustext.text);
                printf("Message  is %s\n", c);
                d=c[0];
                switch(d){
                    case 't':
                      std::cout<<"qqq"<<std::endl;
		      monitoredTakeoff(vehicle);
                      break;
                    case 'l':
                      std::cout<<"www"<<std::endl;
                      monitoredLanding(vehicle);
                      break;
                      case 'g':
                      std::cout<<"www"<<std::endl;
                      moveByPositionOffset(vehicle,1,6,7,0);
                      break;
                    default:
                      break;











                }
            }
	});
	MavLinkStatustext statustext;
    while(strcmp(b,"end")){
            std::cin>>b;
            strcpy(statustext.text,b);
            statustext.severity = 1;
            client->sendMessage(statustext);
    }
    std::cout<<"start close remote"<<std::endl;
	client->close();
    std::cout<<"finish close"<<std::endl;
}

void UnitTests::SerialPx4Test()
{
	auto connection = MavLinkConnection::connectSerial("px4", com_port_, baud_rate_);

	int count = 0;
	Semaphore  received;

	auto id = connection->subscribe([&](std::shared_ptr<MavLinkConnection> con, const MavLinkMessage& msg) {
		//printf("    Received message %d\n", static_cast<int>(msg.msgid));
		count++;
		if (msg.msgid == 0) {
			received.post();
		}
	});

	if (!received.timed_wait(5000)) {
		throw std::runtime_error("MavLink heartbeat is not being received over serial, please make sure PX4 is plugged in and the unit test is using the right COM port.");
	}

	printf("Received %d mavlink messages over serial port\n", count);
	connection->unsubscribe(id);
	connection->close();
}

class ImageServer {
public:
	ImageServer(std::shared_ptr<MavLinkConnection> con)
	{
		stream = std::make_shared<MavLinkVideoServer>(1, 1);
		stream->connect(con);
		con->subscribe([&](std::shared_ptr<MavLinkConnection> connection, const MavLinkMessage& msg) {

			MavLinkVideoServer::MavLinkVideoRequest req;
			if (stream->hasVideoRequest(req))
			{
				printf("    server received request for video at %f frames every second\n", req.every_n_sec);
                int* image = new int[10000];
				int size1 = sizeof(int) * 10000;
				for (int i = 0; i < 10000; i++)
				{
					image[i] = i;
				}
				stream->sendFrame(reinterpret_cast<uint8_t*>(image), size1, 100, 100, 0, 0);
				delete[] image;
			}
		});
	}

	std::shared_ptr<MavLinkVideoServer> stream;
};

void UnitTests::SendImageTest() {

	const int testPort = 60006;
	std::string testAddr = "192.168.43.188";
    int a = 0;
    std::cin >> a;
    std::cout<<"create local server"<<std::endl;
	std::shared_ptr<MavLinkTcpServer> server = std::make_shared<MavLinkTcpServer>(testAddr, testPort);

	std::future<int> future = std::async(std::launch::async, [=] {

		// this is the server code, it will accept 1 connection from a client on port 14588
		// for this unit test we are expecting a request to send an image.
		auto con = server->acceptTcp("test");
		this->server_ = new ImageServer(con);
		return 0;
	});
    std::cin >> a;
    std::cout<<"create local client"<<std::endl;
	// add a drone connection so the mavLinkCom can use it to send requests to the above server.
	auto drone = MavLinkConnection::connectTcp("drone", testAddr, "192.168.43.88", 52006);

	MavLinkVideoClient client{ 150, 1 };
	client.connect(drone);
	client.requestVideo(1, 1, false);

	MavLinkVideoClient::MavLinkVideoFrame image;
	int retries = 100;
	while (!client.readNextFrame(image) && retries-- > 0) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	if (retries <= 0) {
		// hmmm
		throw std::runtime_error("no image received after timeout");
	}
	else {
		std::vector<unsigned char> raw = image.data;
		int* img = reinterpret_cast<int*>(raw.data());

		for (int i = 0, n = static_cast<int>(raw.size() / 4); i < n; i++)
		{
			if (img[i] != i) {
				throw std::runtime_error("corrupt image data received");
			}
		}
	}
	printf("    Received image %d bytes, width %d and height %d ok\n", static_cast<int>(image.data.size()), image.width, image.height);
    //client.close();
	return;

}

/*void UnitTests::VerifyFile(MavLinkFtpClient& ftp, const std::string& dir, const std::string& name, bool exists, bool isdir)
{
    MavLinkFtpProgress progress;
    std::vector<MavLinkFileInfo> files;
    ftp.list(progress, dir, files);
    if (progress.error != 0) {
        throw std::runtime_error(Utils::stringf("unexpected error %d: '%s' from ftp list '%s' command",
            progress.error, progress.message.c_str(), dir.c_str()));
    }

    bool found = false;
    for (auto ptr = files.begin(), end = files.end(); ptr != end; ptr++) {
        MavLinkFileInfo i = *ptr;
        if (isdir == i.is_directory && i.name == name) {
            found = true;
        }
    }
    if (!found && exists) {
        throw std::runtime_error(Utils::stringf("The %s '%s' not found in '%s', but it should be there",
            isdir ? "dir" : "file", name.c_str(), dir.c_str()));
    }
    else if (found && !exists) {
        throw std::runtime_error(Utils::stringf("The %s '%s' was found in '%s', but it should not have been",
            isdir ? "dir" : "file", name.c_str(), dir.c_str()));
    }

}

void UnitTests::FtpTest() {

    std::shared_ptr<MavLinkConnection> connection = MavLinkConnection::connectSerial("px4", com_port_, baud_rate_);

    MavLinkFtpClient ftp{ 166,1 };
    ftp.connect(connection);

    try {

        MavLinkFtpProgress progress;
        std::vector<MavLinkFileInfo> files;

        // ================ ls

        ftp.list(progress, "/fs/microsd", files);
        if (progress.error != 0) {
            throw std::runtime_error(Utils::stringf("unexpected error %d: '%s' from ftp list '/fs/microsd' command - does your pixhawk have an sd card?",
                progress.error, progress.message.c_str()));
        }
        else
        {
            printf("Found %d files in '/fs/microsd' folder\n", static_cast<int>(files.size()));
        }

        // ================ put file

        auto tempPath = FileSystem::getTempFolder();
        tempPath = FileSystem::combine(tempPath, "ftptest.txt");
        std::ofstream stream(tempPath);

        const char* TestPattern = "This is line %d\n";

        for (int i = 0; i < 100; i++) {
            std::string line = Utils::stringf(TestPattern, i);
            stream << line;
        }

        stream.close();

        std::string remotePath = "/fs/microsd/ftptest.txt";
        std::string localPath = tempPath;
#if defined(_WIN32)
        // I wish there was a cleaner way to do this, but I can't use tempPath.native() because on windows that is a wstring and on our linux build it is a string.
        replaceAll(localPath, '/', '\\');
#endif

        ftp.put(progress, remotePath, localPath);

        if (progress.error != 0) {
            throw std::runtime_error(Utils::stringf("unexpected error %d: '%s' from ftp put command",
                progress.error, progress.message.c_str()));
        }
        else
        {
            printf("put succeeded\n");
        }

        FileSystem::remove(tempPath);

        VerifyFile(ftp, "/fs/microsd", "ftptest.txt", true, false);

        // ================ get file
        ftp.get(progress, remotePath, localPath);

        if (progress.error != 0) {
            throw std::runtime_error(Utils::stringf("unexpected error %d: '%s' from ftp get command",
                progress.error, progress.message.c_str()));
        }

        // verify the file contents.
        std::ifstream istream(tempPath);

        int count = 0;
        std::string line;
        std::getline(istream, line);
        while (line.size() > 0) {
            line += '\n';
            std::string expected = Utils::stringf(TestPattern, count);
            if (line != expected)
            {
                throw std::runtime_error(Utils::stringf("ftp local file contains unexpected contents '%s' on line %d\n", line.c_str(), count));
            }
            count++;
            std::getline(istream, line);
        }

        printf("get succeeded\n");

        istream.close();

        // ================ remove file
        FileSystem::remove(tempPath);

        ftp.remove(progress, remotePath);

        if (progress.error != 0) {
            throw std::runtime_error(Utils::stringf("unexpected error %d: '%s' from ftp remove command",
                progress.error, progress.message.c_str()));
        }
        else
        {
            printf("remove succeeded\n");
        }

        VerifyFile(ftp, "/fs/microsd", "ftptest.txt", false, false);

        // ================ make directory
        // D:\px4\src\lovettchris\Firmware\rootfs\fs\microsd
        ftp.mkdir(progress, "/fs/microsd/testrmdir");
        if (progress.error != 0) {
            throw std::runtime_error(Utils::stringf("unexpected error %d: '%s' from ftp mkdir '/fs/microsd/testrmdir' command",
                progress.error, progress.message.c_str()));
        }

        VerifyFile(ftp, "/fs/microsd", "testrmdir", true, true);

        // ================ remove directory
        ftp.rmdir(progress, "/fs/microsd/testrmdir");
        if (progress.error != 0) {
            throw std::runtime_error(Utils::stringf("unexpected error %d: '%s' from ftp rmdir '/fs/microsd/testrmdir' command",
                progress.error, progress.message.c_str()));
        }

        VerifyFile(ftp, "/fs/microsd", "testrmdir", false, true);

    }
    catch (...) {
        ftp.close();
        connection->close();
        connection = nullptr;
        throw;
    }

    ftp.close();
    connection->close();
    connection = nullptr;

}
*/
void UnitTests::JSonLogTest()
{
    auto connection = MavLinkConnection::connectSerial("px4", com_port_, baud_rate_);

    MavLinkFileLog log;

    auto tempPath = FileSystem::getTempFolder();
    tempPath = FileSystem::combine(tempPath, "test.mavlink");
    log.openForWriting(tempPath, true);

    int count = 0;
    Semaphore  received;

    auto id = connection->subscribe([&](std::shared_ptr<MavLinkConnection> con, const MavLinkMessage& msg) {
        count++;
        log.write(msg);
        if (count > 50) {
            received.post();
        }
    });

    if (!received.timed_wait(30000)) {
        throw std::runtime_error("PX4 is not sending 50 messages in 30 seconds.");
    }

    connection->unsubscribe(id);
    connection->close();
    log.close();

    // Now verification
    nlohmann::json doc;

    std::ifstream s;
    FileSystem::openTextFile(tempPath, s);
    if (!s.fail()) {
        s >> doc;
    }
    else {
        throw std::runtime_error(Utils::stringf("Cannot open json file at '%s'.", tempPath.c_str()));
    }

    if (doc.count("rows") == 1) {
        nlohmann::json rows = doc["rows"].get<nlohmann::json>();
        int found = 0;
        int imu = 0;
        if (rows.is_array()) {
            size_t size = rows.size();
            for (size_t i = 0; i < size; i++)
            {
                auto ptr = rows[i];
                if (ptr.is_object()) {
                    if (ptr.count("name") == 1) {
                        auto name = ptr["name"].get<std::string>();
                        if (name == "HIGHRES_IMU") {
                            imu++;
                        }
                        found++;
                    }
                }
            }
        }

        printf("found %d valid rows in the json file, and %d HIGHRES_IMU records\n", found, imu);
    }

}
