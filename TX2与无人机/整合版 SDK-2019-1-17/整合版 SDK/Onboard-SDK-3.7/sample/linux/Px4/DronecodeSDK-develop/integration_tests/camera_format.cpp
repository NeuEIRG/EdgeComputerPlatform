#include "integration_test_helper.h"
#include "dronecode_sdk.h"
#include <iostream>
#include <functional>
#include <atomic>
#include "plugins/camera/camera.h"

using namespace dronecode_sdk;

TEST(CameraTest, Format)
{
    DronecodeSDK dc;

    ConnectionResult ret = dc.add_udp_connection();
    ASSERT_EQ(ret, ConnectionResult::SUCCESS);

    // Wait for system to connect via heartbeat.
    std::this_thread::sleep_for(std::chrono::seconds(2));

    System &system = dc.system();
    ASSERT_TRUE(system.has_camera());

    auto camera = std::make_shared<Camera>(system);

    EXPECT_EQ(Camera::Result::SUCCESS, camera->format_storage());
}
