#include <iostream>
#include <cmath>
#include "integration_test_helper.h"
#include "dronecode_sdk.h"
#include "plugins/action/action.h"
#include "plugins/telemetry/telemetry.h"

using namespace dronecode_sdk;

static void takeoff_and_hover_at_altitude(float altitude_m);

TEST_F(SitlTest, ActionHoverSyncDefault)
{
    takeoff_and_hover_at_altitude(2.5);
}

TEST_F(SitlTest, ActionHoverSyncHigher)
{
    takeoff_and_hover_at_altitude(5.0f);
}

TEST_F(SitlTest, ActionHoverSyncLower)
{
    takeoff_and_hover_at_altitude(1.0f);
}

void takeoff_and_hover_at_altitude(float altitude_m)
{
    DronecodeSDK dc;

    ConnectionResult ret = dc.add_udp_connection();
    ASSERT_EQ(ret, ConnectionResult::SUCCESS);

    // Wait for system to connect via heartbeat.
    std::this_thread::sleep_for(std::chrono::seconds(2));
    ASSERT_TRUE(dc.is_connected());

    System &system = dc.system();
    auto telemetry = std::make_shared<Telemetry>(system);

    int iteration = 0;
    while (!telemetry->health_all_ok()) {
        LogInfo() << "waiting for system to be ready";
        std::this_thread::sleep_for(std::chrono::seconds(1));

        ASSERT_LT(++iteration, 10);
    }

    auto action = std::make_shared<Action>(system);
    Action::Result action_ret = action->arm();
    EXPECT_EQ(action_ret, Action::Result::SUCCESS);
    std::this_thread::sleep_for(std::chrono::seconds(1));

    EXPECT_EQ(Action::Result::SUCCESS, action->set_takeoff_altitude(altitude_m));
    auto takeoff_altitude_result = action->get_takeoff_altitude();
    EXPECT_EQ(takeoff_altitude_result.first, Action::Result::SUCCESS);
    EXPECT_FLOAT_EQ(takeoff_altitude_result.second, altitude_m);

    action_ret = action->takeoff();
    EXPECT_EQ(action_ret, Action::Result::SUCCESS);
    // We wait 1.5s / m plus a margin of 3s.
    const int wait_time_s = static_cast<int>(altitude_m * 1.5f + 3.0f);
    std::this_thread::sleep_for(std::chrono::seconds(wait_time_s));

    EXPECT_GT(telemetry->position().relative_altitude_m, altitude_m - 0.25f);
    EXPECT_LT(telemetry->position().relative_altitude_m, altitude_m + 0.25f);

    action_ret = action->land();
    EXPECT_EQ(action_ret, Action::Result::SUCCESS);

    iteration = 0;
    while (telemetry->in_air()) {
        LogInfo() << "waiting for system to be landed";
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // TODO: currently we need to wait a long time until landed is detected.
        ASSERT_LT(++iteration, 2 * wait_time_s);
    }

    action_ret = action->disarm();
    EXPECT_EQ(action_ret, Action::Result::SUCCESS);
}
