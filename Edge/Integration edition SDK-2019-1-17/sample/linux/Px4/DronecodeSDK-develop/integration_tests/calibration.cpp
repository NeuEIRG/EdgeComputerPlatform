#include <iostream>
#include <future>
#include "integration_test_helper.h"
#include "global_include.h"
#include "dronecode_sdk.h"
#include "plugins/calibration/calibration.h"
#include "plugins/params_raw/params_raw.h"
#include "plugins/telemetry/telemetry.h"

using namespace dronecode_sdk;
using namespace std::placeholders; // for `_1`

static void receive_calibration_callback(const Calibration::Result result,
                                         const Calibration::ProgressData &progress_data,
                                         const std::string &calibration_type,
                                         std::promise<Calibration::Result> &prom);

TEST(HardwareTest, CalibrationGyro)
{
    DronecodeSDK dc;

    ConnectionResult ret = dc.add_udp_connection();
    ASSERT_EQ(ret, ConnectionResult::SUCCESS);

    // Wait for system to connect via heartbeat.
    std::this_thread::sleep_for(std::chrono::seconds(2));
    System &system = dc.system();
    ASSERT_TRUE(system.has_autopilot());

    auto calibration = std::make_shared<Calibration>(system);

    std::promise<Calibration::Result> prom{};
    std::future<Calibration::Result> fut = prom.get_future();

    calibration->calibrate_gyro_async(
        std::bind(&receive_calibration_callback, _1, _2, "gyro", std::ref(prom)));

    auto future_ret = fut.get();
    EXPECT_EQ(future_ret, Calibration::Result::SUCCESS);
    if (future_ret != Calibration::Result::SUCCESS) {
        LogErr() << "Calibration error: " << Calibration::result_str(future_ret);
    }
}

TEST(HardwareTest, CalibrationAccelerometer)
{
    DronecodeSDK dc;

    ConnectionResult ret = dc.add_udp_connection();
    ASSERT_EQ(ret, ConnectionResult::SUCCESS);

    // Wait for system to connect via heartbeat.
    std::this_thread::sleep_for(std::chrono::seconds(2));
    System &system = dc.system();
    ASSERT_TRUE(system.has_autopilot());

    auto calibration = std::make_shared<Calibration>(system);

    std::promise<Calibration::Result> prom{};
    std::future<Calibration::Result> fut = prom.get_future();

    calibration->calibrate_accelerometer_async(
        std::bind(&receive_calibration_callback, _1, _2, "accelerometer", std::ref(prom)));

    auto future_ret = fut.get();
    EXPECT_EQ(future_ret, Calibration::Result::SUCCESS);
    if (future_ret != Calibration::Result::SUCCESS) {
        LogErr() << "Calibration error: " << Calibration::result_str(future_ret);
    }
}

TEST(HardwareTest, CalibrationMagnetometer)
{
    DronecodeSDK dc;

    ConnectionResult ret = dc.add_udp_connection();
    ASSERT_EQ(ret, ConnectionResult::SUCCESS);

    // Wait for system to connect via heartbeat.
    std::this_thread::sleep_for(std::chrono::seconds(2));
    System &system = dc.system();

    auto calibration = std::make_shared<Calibration>(system);
    ASSERT_TRUE(system.has_autopilot());

    std::promise<Calibration::Result> prom{};
    std::future<Calibration::Result> fut = prom.get_future();

    calibration->calibrate_magnetometer_async(
        std::bind(&receive_calibration_callback, _1, _2, "magnetometer", std::ref(prom)));

    auto future_ret = fut.get();
    EXPECT_EQ(future_ret, Calibration::Result::SUCCESS);
    if (future_ret != Calibration::Result::SUCCESS) {
        LogErr() << "Calibration error: " << Calibration::result_str(future_ret);
    }
}

TEST(HardwareTest, CalibrationGimbalAccelerometer)
{
    DronecodeSDK dc;

    ConnectionResult ret = dc.add_udp_connection();
    ASSERT_EQ(ret, ConnectionResult::SUCCESS);

    // Wait for system to connect via heartbeat.
    std::this_thread::sleep_for(std::chrono::seconds(2));
    System &system = dc.system();
    ASSERT_TRUE(system.has_gimbal());

    auto calibration = std::make_shared<Calibration>(system);

    std::promise<Calibration::Result> prom{};
    std::future<Calibration::Result> fut = prom.get_future();

    calibration->calibrate_gimbal_accelerometer_async(
        std::bind(&receive_calibration_callback, _1, _2, "gimbal accelerometer", std::ref(prom)));

    auto future_ret = fut.get();
    EXPECT_EQ(future_ret, Calibration::Result::SUCCESS);
    if (future_ret != Calibration::Result::SUCCESS) {
        LogErr() << "Calibration error: " << Calibration::result_str(future_ret);
    }
}

TEST(HardwareTest, CalibrationGyroWithTelemetry)
{
    DronecodeSDK dc;

    ConnectionResult ret = dc.add_udp_connection();
    ASSERT_EQ(ret, ConnectionResult::SUCCESS);

    // Wait for system to connect via heartbeat.
    std::this_thread::sleep_for(std::chrono::seconds(2));
    System &system = dc.system();
    ASSERT_TRUE(system.has_autopilot());

    // Reset Gyro calibration using param.
    auto params_raw = std::make_shared<ParamsRaw>(system);
    params_raw->set_param_int("CAL_GYRO0_ID", 0);

    // Make sure telemetry reports gyro calibration as false.
    auto telemetry = std::make_shared<Telemetry>(system);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    ASSERT_FALSE(telemetry->health().gyrometer_calibration_ok);

    // Do gyro calibration.
    auto calibration = std::make_shared<Calibration>(system);

    std::promise<Calibration::Result> prom{};
    std::future<Calibration::Result> fut = prom.get_future();

    calibration->calibrate_gyro_async(
        std::bind(&receive_calibration_callback, _1, _2, "gyro", std::ref(prom)));

    auto future_ret = fut.get();
    EXPECT_EQ(future_ret, Calibration::Result::SUCCESS);
    if (future_ret != Calibration::Result::SUCCESS) {
        LogErr() << "Calibration error: " << Calibration::result_str(future_ret);
    }

    // Now, telemetry should be updated showing that the gyro calibration is ok.
    std::this_thread::sleep_for(std::chrono::seconds(2));
    EXPECT_TRUE(telemetry->health().gyrometer_calibration_ok);
}

TEST(HardwareTest, CalibrationGyroCancelled)
{
    DronecodeSDK dc;

    ConnectionResult ret = dc.add_udp_connection();
    ASSERT_EQ(ret, ConnectionResult::SUCCESS);

    // Wait for system to connect via heartbeat.
    std::this_thread::sleep_for(std::chrono::seconds(2));
    System &system = dc.system();
    ASSERT_TRUE(system.has_autopilot());

    // Do gyro calibration.
    auto calibration = std::make_shared<Calibration>(system);

    std::promise<Calibration::Result> prom{};
    std::future<Calibration::Result> fut = prom.get_future();

    calibration->calibrate_gyro_async(
        std::bind(&receive_calibration_callback, _1, _2, "gyro", std::ref(prom)));

    auto status = fut.wait_for(std::chrono::seconds(2));
    EXPECT_EQ(status, std::future_status::timeout);

    calibration->cancel_calibration();

    status = fut.wait_for(std::chrono::seconds(1));
    EXPECT_EQ(status, std::future_status::ready);
    auto future_ret = fut.get();
    EXPECT_EQ(future_ret, Calibration::Result::CANCELLED);
}

void receive_calibration_callback(const Calibration::Result result,
                                  const Calibration::ProgressData &progress_data,
                                  const std::string &calibration_type,
                                  std::promise<Calibration::Result> &prom)
{
    if (result == Calibration::Result::IN_PROGRESS) {
        LogInfo() << calibration_type << " calibration in progress: " << progress_data.progress;
    } else if (result == Calibration::Result::INSTRUCTION) {
        LogInfo() << calibration_type << " calibration instruction: " << progress_data.status_text;
    } else {
        prom.set_value(result);
    }
}
