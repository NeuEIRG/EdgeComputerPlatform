#include <future>
#include <gmock/gmock.h>
#include <grpc++/grpc++.h>
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <memory>
#include <random>
#include <vector>

#include "telemetry/mocks/telemetry_mock.h"
#include "telemetry/telemetry_service_impl.h"

namespace {

using testing::_;
using testing::NiceMock;

using MockTelemetry = NiceMock<dronecode_sdk::testing::MockTelemetry>;
using TelemetryServiceImpl = dronecode_sdk::backend::TelemetryServiceImpl<MockTelemetry>;
using TelemetryService = dronecode_sdk::rpc::telemetry::TelemetryService;

using PositionResponse = dronecode_sdk::rpc::telemetry::PositionResponse;
using Position = dronecode_sdk::Telemetry::Position;

using HealthResponse = dronecode_sdk::rpc::telemetry::HealthResponse;
using Health = dronecode_sdk::Telemetry::Health;

using GpsInfo = dronecode_sdk::Telemetry::GPSInfo;
using FixType = dronecode_sdk::rpc::telemetry::FixType;

using Battery = dronecode_sdk::Telemetry::Battery;

using FlightMode = dronecode_sdk::Telemetry::FlightMode;
using RPCFlightMode = dronecode_sdk::rpc::telemetry::FlightMode;

using Quaternion = dronecode_sdk::Telemetry::Quaternion;

using EulerAngle = dronecode_sdk::Telemetry::EulerAngle;

using GroundSpeedNed = dronecode_sdk::Telemetry::GroundSpeedNED;

using RcStatus = dronecode_sdk::Telemetry::RCStatus;

class TelemetryServiceImplTest : public ::testing::Test {
protected:
    virtual void SetUp()
    {
        _telemetry = std::unique_ptr<MockTelemetry>(new MockTelemetry());
        _telemetry_service =
            std::unique_ptr<TelemetryServiceImpl>(new TelemetryServiceImpl(*_telemetry));

        grpc::ServerBuilder builder;
        builder.RegisterService(_telemetry_service.get());
        _server = builder.BuildAndStart();

        grpc::ChannelArguments channel_args;
        auto channel = _server->InProcessChannel(channel_args);
        _stub = TelemetryService::NewStub(channel);

        initRandomGenerator();
    }

    virtual void TearDown() { _server->Shutdown(); }

    std::future<void> subscribePositionAsync(std::vector<Position> &positions);
    Position createPosition(const double lat,
                            const double lng,
                            const float abs_alt,
                            const float rel_alt) const;
    void checkSendsPositions(const std::vector<Position> &positions);

    std::future<void> subscribeHealthAsync(std::vector<Health> &healths);
    void checkSendsHealths(const std::vector<Health> &healths);
    Health createRandomHealth();
    std::vector<Health> generateRandomHealthsVector(const int size);
    bool generateRandomBool();

    void checkSendsHomePositions(const std::vector<Position> &home_positions) const;
    std::future<void> subscribeHomeAsync(std::vector<Position> &home_positions) const;

    void checkSendsInAirEvents(const std::vector<bool> &in_air_events) const;
    std::future<void> subscribeInAirAsync(std::vector<bool> &in_air_events) const;

    void checkSendsArmedEvents(const std::vector<bool> &armed_events) const;
    std::future<void> subscribeArmedAsync(std::vector<bool> &armed_events) const;

    GpsInfo createGpsInfo(const int num_satellites, const int fix_type) const;
    void checkSendsGpsInfoEvents(const std::vector<GpsInfo> &gps_info_events) const;
    std::future<void> subscribeGpsInfoAsync(std::vector<GpsInfo> &gps_info_events) const;
    int translateRPCGpsFixType(const FixType rpc_fix_type) const;

    void checkSendsBatteryEvents(const std::vector<Battery> &battery_events) const;
    Battery createBattery(const float voltage_v, const float remaining_percent) const;
    std::future<void> subscribeBatteryAsync(std::vector<Battery> &battery_events) const;

    void checkSendsFlightModeEvents(const std::vector<FlightMode> &flight_mode_events) const;
    FlightMode translateRPCFlightMode(const RPCFlightMode rpc_flight_mode) const;
    std::future<void> subscribeFlightModeAsync(std::vector<FlightMode> &flight_mode_events) const;

    void checkSendsAttitudeQuaternions(const std::vector<Quaternion> &quaternions) const;
    Quaternion createQuaternion(const float w, const float x, const float y, const float z) const;
    std::future<void> subscribeAttitudeQuaternionAsync(std::vector<Quaternion> &quaternions) const;

    void checkSendsAttitudeEulerAngles(const std::vector<EulerAngle> &euler_angles) const;
    EulerAngle
    createEulerAngle(const float roll_deg, const float pitch_deg, const float yaw_deg) const;
    std::future<void> subscribeAttitudeEulerAsync(std::vector<EulerAngle> &euler_angles) const;

    void checkSendsCameraAttitudeQuaternions(const std::vector<Quaternion> &quaternions) const;
    std::future<void>
    subscribeCameraAttitudeQuaternionAsync(std::vector<Quaternion> &quaternions) const;

    void checkSendsCameraAttitudeEulerAngles(const std::vector<EulerAngle> &euler_angles) const;
    std::future<void>
    subscribeCameraAttitudeEulerAsync(std::vector<EulerAngle> &euler_angles) const;

    void checkSendsGroundSpeedEvents(const std::vector<GroundSpeedNed> &ground_speed_events) const;
    GroundSpeedNed
    createGroundSpeedNed(const float vel_north, const float vel_east, const float vel_down) const;
    std::future<void>
    subscribeGroundSpeedNedAsync(std::vector<GroundSpeedNed> &ground_speed_events) const;

    void checkSendsRcStatusEvents(const std::vector<RcStatus> &rc_status_events) const;
    RcStatus createRcStatus(const bool was_available_once,
                            const bool is_available,
                            const float signal_strength_percent) const;
    std::future<void> subscribeRcStatusAsync(std::vector<RcStatus> &rc_status_events) const;

    std::unique_ptr<grpc::Server> _server{};
    std::unique_ptr<TelemetryService::Stub> _stub{};
    std::unique_ptr<MockTelemetry> _telemetry{};
    std::unique_ptr<TelemetryServiceImpl> _telemetry_service{};

private:
    void initRandomGenerator();

    std::random_device _random_device{};
    std::mt19937 _generator{};
    std::uniform_int_distribution<> _uniform_int_distribution{};
};

void TelemetryServiceImplTest::initRandomGenerator()
{
    _generator = std::mt19937(_random_device());
    _uniform_int_distribution = std::uniform_int_distribution<>(0, 1);
}

ACTION_P2(SaveCallback, callback, callback_promise)
{
    *callback = arg0;
    callback_promise->set_value();
}

TEST_F(TelemetryServiceImplTest, registersToTelemetryPositionAsync)
{
    EXPECT_CALL(*_telemetry, position_async(_)).Times(1);

    std::vector<Position> positions;
    auto position_stream_future = subscribePositionAsync(positions);

    _telemetry_service->stop();
    position_stream_future.wait();
}

std::future<void> TelemetryServiceImplTest::subscribePositionAsync(std::vector<Position> &positions)
{
    return std::async(std::launch::async, [&]() {
        grpc::ClientContext context;
        dronecode_sdk::rpc::telemetry::SubscribePositionRequest request;
        auto response_reader = _stub->SubscribePosition(&context, request);

        dronecode_sdk::rpc::telemetry::PositionResponse response;
        while (response_reader->Read(&response)) {
            auto position_rpc = response.position();

            Position position;
            position.latitude_deg = position_rpc.latitude_deg();
            position.longitude_deg = position_rpc.longitude_deg();
            position.absolute_altitude_m = position_rpc.absolute_altitude_m();
            position.relative_altitude_m = position_rpc.relative_altitude_m();

            positions.push_back(position);
        }

        response_reader->Finish();
    });
}

TEST_F(TelemetryServiceImplTest, doesNotSendPositionIfCallbackNotCalled)
{
    std::vector<Position> positions;
    auto position_stream_future = subscribePositionAsync(positions);

    _telemetry_service->stop();
    position_stream_future.wait();

    EXPECT_EQ(0, positions.size());
}

TEST_F(TelemetryServiceImplTest, sendsOnePosition)
{
    std::vector<Position> positions;
    positions.push_back(createPosition(41.848695, 75.132751, 3002.1f, 50.3f));

    checkSendsPositions(positions);
}

void TelemetryServiceImplTest::checkSendsPositions(const std::vector<Position> &positions)
{
    std::promise<void> subscription_promise;
    auto subscription_future = subscription_promise.get_future();
    dronecode_sdk::Telemetry::position_callback_t position_callback;
    EXPECT_CALL(*_telemetry, position_async(_))
        .WillOnce(SaveCallback(&position_callback, &subscription_promise));

    std::vector<Position> received_positions;
    auto position_stream_future = subscribePositionAsync(received_positions);
    subscription_future.wait();
    for (const auto position : positions) {
        position_callback(position);
    }
    _telemetry_service->stop();
    position_stream_future.wait();

    ASSERT_EQ(positions.size(), received_positions.size());
    for (size_t i = 0; i < positions.size(); i++) {
        EXPECT_EQ(positions.at(i), received_positions.at(i));
    }
}

Position TelemetryServiceImplTest::createPosition(const double lat,
                                                  const double lng,
                                                  const float abs_alt,
                                                  const float rel_alt) const
{
    dronecode_sdk::Telemetry::Position expected_position;

    expected_position.latitude_deg = lat;
    expected_position.longitude_deg = lng;
    expected_position.absolute_altitude_m = abs_alt;
    expected_position.relative_altitude_m = rel_alt;

    return expected_position;
}

TEST_F(TelemetryServiceImplTest, sendsMultiplePositions)
{
    std::vector<Position> positions;
    positions.push_back(createPosition(41.848695, 75.132751, 3002.1f, 50.3f));
    positions.push_back(createPosition(46.522626, 6.635356, 542.2f, 79.8f));
    positions.push_back(createPosition(-50.995944711358824, -72.99892046835936, 1217.12f, 2.52f));

    checkSendsPositions(positions);
}

TEST_F(TelemetryServiceImplTest, registersToTelemetryHealthAsync)
{
    EXPECT_CALL(*_telemetry, health_async(_)).Times(1);

    std::vector<Health> healths;
    auto health_stream_future = subscribeHealthAsync(healths);

    _telemetry_service->stop();
    health_stream_future.wait();
}

std::future<void> TelemetryServiceImplTest::subscribeHealthAsync(std::vector<Health> &healths)
{
    return std::async(std::launch::async, [&]() {
        grpc::ClientContext context;
        dronecode_sdk::rpc::telemetry::SubscribeHealthRequest request;
        auto response_reader = _stub->SubscribeHealth(&context, request);

        dronecode_sdk::rpc::telemetry::HealthResponse response;
        while (response_reader->Read(&response)) {
            auto health_rpc = response.health();

            Health health;
            health.gyrometer_calibration_ok = health_rpc.is_gyrometer_calibration_ok();
            health.accelerometer_calibration_ok = health_rpc.is_accelerometer_calibration_ok();
            health.magnetometer_calibration_ok = health_rpc.is_magnetometer_calibration_ok();
            health.level_calibration_ok = health_rpc.is_level_calibration_ok();
            health.local_position_ok = health_rpc.is_local_position_ok();
            health.global_position_ok = health_rpc.is_global_position_ok();
            health.home_position_ok = health_rpc.is_home_position_ok();

            healths.push_back(health);
        }

        response_reader->Finish();
    });
}

TEST_F(TelemetryServiceImplTest, doesNotSendHealthIfCallbackNotCalled)
{
    std::vector<Health> healths;
    auto health_stream_future = subscribeHealthAsync(healths);

    _telemetry_service->stop();
    health_stream_future.wait();

    EXPECT_EQ(0, healths.size());
}

TEST_F(TelemetryServiceImplTest, sendsOneHealth)
{
    const auto health = generateRandomHealthsVector(1);
    checkSendsHealths(health);
}

std::vector<Health> TelemetryServiceImplTest::generateRandomHealthsVector(const int size)
{
    std::vector<Health> healths;
    for (int i = 0; i < size; i++) {
        healths.push_back(createRandomHealth());
    }

    return healths;
}

void TelemetryServiceImplTest::checkSendsHealths(const std::vector<Health> &healths)
{
    std::promise<void> subscription_promise;
    auto subscription_future = subscription_promise.get_future();
    dronecode_sdk::Telemetry::health_callback_t health_callback;
    EXPECT_CALL(*_telemetry, health_async(_))
        .WillOnce(SaveCallback(&health_callback, &subscription_promise));

    std::vector<Health> received_healths;
    auto health_stream_future = subscribeHealthAsync(received_healths);
    subscription_future.wait();
    for (const auto health : healths) {
        health_callback(health);
    }
    _telemetry_service->stop();
    health_stream_future.wait();

    ASSERT_EQ(healths.size(), received_healths.size());
    for (size_t i = 0; i < healths.size(); i++) {
        EXPECT_EQ(healths.at(i), received_healths.at(i));
    }
}

Health TelemetryServiceImplTest::createRandomHealth()
{
    dronecode_sdk::Telemetry::Health health;

    health.gyrometer_calibration_ok = generateRandomBool();
    health.accelerometer_calibration_ok = generateRandomBool();
    health.magnetometer_calibration_ok = generateRandomBool();
    health.level_calibration_ok = generateRandomBool();
    health.local_position_ok = generateRandomBool();
    health.global_position_ok = generateRandomBool();
    health.home_position_ok = generateRandomBool();

    return health;
}

bool TelemetryServiceImplTest::generateRandomBool()
{
    return _uniform_int_distribution(_generator) == 0;
}

TEST_F(TelemetryServiceImplTest, sendsMultipleHealths)
{
    const auto health = generateRandomHealthsVector(10);
    checkSendsHealths(health);
}

TEST_F(TelemetryServiceImplTest, registersToTelemetryHomeAsync)
{
    EXPECT_CALL(*_telemetry, home_position_async(_)).Times(1);

    std::vector<Position> home_positions;
    auto home_stream_future = subscribeHomeAsync(home_positions);

    _telemetry_service->stop();
    home_stream_future.wait();
}

std::future<void>
TelemetryServiceImplTest::subscribeHomeAsync(std::vector<Position> &home_positions) const
{
    return std::async(std::launch::async, [&]() {
        grpc::ClientContext context;
        dronecode_sdk::rpc::telemetry::SubscribeHomeRequest request;
        auto response_reader = _stub->SubscribeHome(&context, request);

        dronecode_sdk::rpc::telemetry::HomeResponse response;
        while (response_reader->Read(&response)) {
            auto home_rpc = response.home();

            Position home;
            home.latitude_deg = home_rpc.latitude_deg();
            home.longitude_deg = home_rpc.longitude_deg();
            home.absolute_altitude_m = home_rpc.absolute_altitude_m();
            home.relative_altitude_m = home_rpc.relative_altitude_m();

            home_positions.push_back(home);
        }

        response_reader->Finish();
    });
}

TEST_F(TelemetryServiceImplTest, doesNotSendHomeIfCallbackNotCalled)
{
    std::vector<Position> home_positions;
    auto home_stream_future = subscribeHomeAsync(home_positions);

    _telemetry_service->stop();
    home_stream_future.wait();

    EXPECT_EQ(0, home_positions.size());
}

TEST_F(TelemetryServiceImplTest, sendsOneHome)
{
    std::vector<Position> home_positions;
    home_positions.push_back(createPosition(41.848695, 75.132751, 3002.1f, 50.3f));

    checkSendsHomePositions(home_positions);
}

void TelemetryServiceImplTest::checkSendsHomePositions(
    const std::vector<Position> &home_positions) const
{
    std::promise<void> subscription_promise;
    auto subscription_future = subscription_promise.get_future();
    dronecode_sdk::Telemetry::position_callback_t home_callback;
    EXPECT_CALL(*_telemetry, home_position_async(_))
        .WillOnce(SaveCallback(&home_callback, &subscription_promise));

    std::vector<Position> received_home_positions;
    auto home_stream_future = subscribeHomeAsync(received_home_positions);
    subscription_future.wait();
    for (const auto home_position : home_positions) {
        home_callback(home_position);
    }
    _telemetry_service->stop();
    home_stream_future.wait();

    ASSERT_EQ(home_positions.size(), received_home_positions.size());
    for (size_t i = 0; i < home_positions.size(); i++) {
        EXPECT_EQ(home_positions.at(i), received_home_positions.at(i));
    }
}

TEST_F(TelemetryServiceImplTest, sendsMultipleHomePositions)
{
    std::vector<Position> home_positions;
    home_positions.push_back(createPosition(41.848695, 75.132751, 3002.1f, 50.3f));
    home_positions.push_back(createPosition(46.522626, 6.635356, 542.2f, 79.8f));
    home_positions.push_back(
        createPosition(-50.995944711358824, -72.99892046835936, 1217.12f, 2.52f));

    checkSendsHomePositions(home_positions);
}

TEST_F(TelemetryServiceImplTest, registersToTelemetryInAirAsync)
{
    EXPECT_CALL(*_telemetry, in_air_async(_)).Times(1);

    std::vector<bool> in_air_events;
    auto in_air_stream_future = subscribeInAirAsync(in_air_events);

    _telemetry_service->stop();
    in_air_stream_future.wait();
}

std::future<void>
TelemetryServiceImplTest::subscribeInAirAsync(std::vector<bool> &in_air_events) const
{
    return std::async(std::launch::async, [&]() {
        grpc::ClientContext context;
        dronecode_sdk::rpc::telemetry::SubscribeInAirRequest request;
        auto response_reader = _stub->SubscribeInAir(&context, request);

        dronecode_sdk::rpc::telemetry::InAirResponse response;
        while (response_reader->Read(&response)) {
            auto is_in_air = response.is_in_air();
            in_air_events.push_back(is_in_air);
        }

        response_reader->Finish();
    });
}

TEST_F(TelemetryServiceImplTest, doesNotSendInAirIfCallbackNotCalled)
{
    std::vector<bool> in_air_events;
    auto in_air_stream_future = subscribeInAirAsync(in_air_events);

    _telemetry_service->stop();
    in_air_stream_future.wait();

    EXPECT_EQ(0, in_air_events.size());
}

TEST_F(TelemetryServiceImplTest, sendsOneInAirEvent)
{
    std::vector<bool> in_air_events;
    in_air_events.push_back(generateRandomBool());

    checkSendsInAirEvents(in_air_events);
}

void TelemetryServiceImplTest::checkSendsInAirEvents(const std::vector<bool> &in_air_events) const
{
    std::promise<void> subscription_promise;
    auto subscription_future = subscription_promise.get_future();
    dronecode_sdk::Telemetry::in_air_callback_t in_air_callback;
    EXPECT_CALL(*_telemetry, in_air_async(_))
        .WillOnce(SaveCallback(&in_air_callback, &subscription_promise));

    std::vector<bool> received_in_air_events;
    auto in_air_stream_future = subscribeInAirAsync(received_in_air_events);
    subscription_future.wait();
    for (const auto is_in_air : in_air_events) {
        in_air_callback(is_in_air);
    }
    _telemetry_service->stop();
    in_air_stream_future.wait();

    ASSERT_EQ(in_air_events.size(), received_in_air_events.size());
    for (size_t i = 0; i < in_air_events.size(); i++) {
        EXPECT_EQ(in_air_events.at(i), received_in_air_events.at(i));
    }
}

TEST_F(TelemetryServiceImplTest, sendsMultipleInAirEvents)
{
    std::vector<bool> in_air_events;

    for (int i = 0; i < 10; i++) {
        in_air_events.push_back(generateRandomBool());
    }

    checkSendsInAirEvents(in_air_events);
}

TEST_F(TelemetryServiceImplTest, registersToTelemetryArmedAsync)
{
    EXPECT_CALL(*_telemetry, armed_async(_)).Times(1);

    std::vector<bool> armed_events;
    auto armed_stream_future = subscribeArmedAsync(armed_events);

    _telemetry_service->stop();
    armed_stream_future.wait();
}

std::future<void>
TelemetryServiceImplTest::subscribeArmedAsync(std::vector<bool> &armed_events) const
{
    return std::async(std::launch::async, [&]() {
        grpc::ClientContext context;
        dronecode_sdk::rpc::telemetry::SubscribeArmedRequest request;
        auto response_reader = _stub->SubscribeArmed(&context, request);

        dronecode_sdk::rpc::telemetry::ArmedResponse response;
        while (response_reader->Read(&response)) {
            auto is_armed = response.is_armed();
            armed_events.push_back(is_armed);
        }

        response_reader->Finish();
    });
}

TEST_F(TelemetryServiceImplTest, doesNotSendArmedIfCallbackNotCalled)
{
    std::vector<bool> armed_events;
    auto armed_stream_future = subscribeArmedAsync(armed_events);

    _telemetry_service->stop();
    armed_stream_future.wait();

    EXPECT_EQ(0, armed_events.size());
}

TEST_F(TelemetryServiceImplTest, sendsOneArmedEvent)
{
    std::vector<bool> armed_events;
    armed_events.push_back(generateRandomBool());

    checkSendsArmedEvents(armed_events);
}

void TelemetryServiceImplTest::checkSendsArmedEvents(const std::vector<bool> &armed_events) const
{
    std::promise<void> subscription_promise;
    auto subscription_future = subscription_promise.get_future();
    dronecode_sdk::Telemetry::armed_callback_t armed_callback;
    EXPECT_CALL(*_telemetry, armed_async(_))
        .WillOnce(SaveCallback(&armed_callback, &subscription_promise));

    std::vector<bool> received_armed_events;
    auto armed_stream_future = subscribeArmedAsync(received_armed_events);
    subscription_future.wait();
    for (const auto is_armed : armed_events) {
        armed_callback(is_armed);
    }
    _telemetry_service->stop();
    armed_stream_future.wait();

    ASSERT_EQ(armed_events.size(), received_armed_events.size());
    for (size_t i = 0; i < armed_events.size(); i++) {
        EXPECT_EQ(armed_events.at(i), received_armed_events.at(i));
    }
}

TEST_F(TelemetryServiceImplTest, sendsMultipleArmedEvents)
{
    std::vector<bool> armed_events;

    for (int i = 0; i < 10; i++) {
        armed_events.push_back(generateRandomBool());
    }

    checkSendsArmedEvents(armed_events);
}

TEST_F(TelemetryServiceImplTest, registersToTelemetryGpsInfoAsync)
{
    EXPECT_CALL(*_telemetry, gps_info_async(_)).Times(1);

    std::vector<GpsInfo> gps_info_events;
    auto gps_info_stream_future = subscribeGpsInfoAsync(gps_info_events);

    _telemetry_service->stop();
    gps_info_stream_future.wait();
}

std::future<void>
TelemetryServiceImplTest::subscribeGpsInfoAsync(std::vector<GpsInfo> &gps_info_events) const
{
    return std::async(std::launch::async, [&]() {
        grpc::ClientContext context;
        dronecode_sdk::rpc::telemetry::SubscribeGpsInfoRequest request;
        auto response_reader = _stub->SubscribeGpsInfo(&context, request);

        dronecode_sdk::rpc::telemetry::GpsInfoResponse response;
        while (response_reader->Read(&response)) {
            auto gps_info_rpc = response.gps_info();

            GpsInfo gps_info;
            gps_info.num_satellites = gps_info_rpc.num_satellites();
            gps_info.fix_type = translateRPCGpsFixType(gps_info_rpc.fix_type());

            gps_info_events.push_back(gps_info);
        }

        response_reader->Finish();
    });
}

int TelemetryServiceImplTest::translateRPCGpsFixType(const FixType rpc_fix_type) const
{
    switch (rpc_fix_type) {
        default:
        case FixType::NO_GPS:
            return 0;
        case FixType::NO_FIX:
            return 1;
        case FixType::FIX_2D:
            return 2;
        case FixType::FIX_3D:
            return 3;
        case FixType::FIX_DGPS:
            return 4;
        case FixType::RTK_FLOAT:
            return 5;
        case FixType::RTK_FIXED:
            return 6;
    }
}

TEST_F(TelemetryServiceImplTest, doesNotSendGpsInfoIfCallbackNotCalled)
{
    std::vector<GpsInfo> gps_info_events;
    auto gps_info_stream_future = subscribeGpsInfoAsync(gps_info_events);

    _telemetry_service->stop();
    gps_info_stream_future.wait();

    EXPECT_EQ(0, gps_info_events.size());
}

TEST_F(TelemetryServiceImplTest, sendsOneGpsInfoEvent)
{
    std::vector<GpsInfo> gps_info_events;
    gps_info_events.push_back(createGpsInfo(10, 3));

    checkSendsGpsInfoEvents(gps_info_events);
}

void TelemetryServiceImplTest::checkSendsGpsInfoEvents(
    const std::vector<GpsInfo> &gps_info_events) const
{
    std::promise<void> subscription_promise;
    auto subscription_future = subscription_promise.get_future();
    dronecode_sdk::Telemetry::gps_info_callback_t gps_info_callback;
    EXPECT_CALL(*_telemetry, gps_info_async(_))
        .WillOnce(SaveCallback(&gps_info_callback, &subscription_promise));

    std::vector<GpsInfo> received_gps_info_events;
    auto gps_info_stream_future = subscribeGpsInfoAsync(received_gps_info_events);
    subscription_future.wait();
    for (const auto gps_info : gps_info_events) {
        gps_info_callback(gps_info);
    }
    _telemetry_service->stop();
    gps_info_stream_future.wait();

    ASSERT_EQ(gps_info_events.size(), received_gps_info_events.size());
    for (size_t i = 0; i < gps_info_events.size(); i++) {
        EXPECT_EQ(gps_info_events.at(i), received_gps_info_events.at(i));
    }
}

GpsInfo TelemetryServiceImplTest::createGpsInfo(const int num_satellites, const int fix_type) const
{
    GpsInfo expected_gps_info;

    expected_gps_info.num_satellites = num_satellites;
    expected_gps_info.fix_type = fix_type;

    return expected_gps_info;
}

TEST_F(TelemetryServiceImplTest, sendsMultipleGpsInfoEvents)
{
    std::vector<GpsInfo> gps_info_events;
    gps_info_events.push_back(createGpsInfo(5, 0));
    gps_info_events.push_back(createGpsInfo(0, 1));
    gps_info_events.push_back(createGpsInfo(10, 2));
    gps_info_events.push_back(createGpsInfo(8, 3));
    gps_info_events.push_back(createGpsInfo(22, 4));
    gps_info_events.push_back(createGpsInfo(13, 5));
    gps_info_events.push_back(createGpsInfo(7, 6));

    checkSendsGpsInfoEvents(gps_info_events);
}

TEST_F(TelemetryServiceImplTest, registersToTelemetryBatteryAsync)
{
    EXPECT_CALL(*_telemetry, battery_async(_)).Times(1);

    std::vector<Battery> battery_events;
    auto battery_stream_future = subscribeBatteryAsync(battery_events);

    _telemetry_service->stop();
    battery_stream_future.wait();
}

std::future<void>
TelemetryServiceImplTest::subscribeBatteryAsync(std::vector<Battery> &battery_events) const
{
    return std::async(std::launch::async, [&]() {
        grpc::ClientContext context;
        dronecode_sdk::rpc::telemetry::SubscribeBatteryRequest request;
        auto response_reader = _stub->SubscribeBattery(&context, request);

        dronecode_sdk::rpc::telemetry::BatteryResponse response;
        while (response_reader->Read(&response)) {
            auto battery_rpc = response.battery();

            Battery battery;
            battery.voltage_v = battery_rpc.voltage_v();
            battery.remaining_percent = battery_rpc.remaining_percent();

            battery_events.push_back(battery);
        }

        response_reader->Finish();
    });
}

TEST_F(TelemetryServiceImplTest, doesNotSendBatteryIfCallbackNotCalled)
{
    std::vector<Battery> battery_events;
    auto battery_stream_future = subscribeBatteryAsync(battery_events);

    _telemetry_service->stop();
    battery_stream_future.wait();

    EXPECT_EQ(0, battery_events.size());
}

TEST_F(TelemetryServiceImplTest, sendsOneBatteryEvent)
{
    std::vector<Battery> battery_events;
    battery_events.push_back(createBattery(4.2f, 0.63f));

    checkSendsBatteryEvents(battery_events);
}

Battery TelemetryServiceImplTest::createBattery(const float voltage_v,
                                                const float remaining_percent) const
{
    Battery battery;
    battery.voltage_v = voltage_v;
    battery.remaining_percent = remaining_percent;

    return battery;
}

void TelemetryServiceImplTest::checkSendsBatteryEvents(
    const std::vector<Battery> &battery_events) const
{
    std::promise<void> subscription_promise;
    auto subscription_future = subscription_promise.get_future();
    dronecode_sdk::Telemetry::battery_callback_t battery_callback;
    EXPECT_CALL(*_telemetry, battery_async(_))
        .WillOnce(SaveCallback(&battery_callback, &subscription_promise));

    std::vector<Battery> received_battery_events;
    auto battery_stream_future = subscribeBatteryAsync(received_battery_events);
    subscription_future.wait();
    for (const auto battery : battery_events) {
        battery_callback(battery);
    }
    _telemetry_service->stop();
    battery_stream_future.wait();

    ASSERT_EQ(battery_events.size(), received_battery_events.size());
    for (size_t i = 0; i < battery_events.size(); i++) {
        EXPECT_EQ(battery_events.at(i), received_battery_events.at(i));
    }
}

TEST_F(TelemetryServiceImplTest, sendsMultipleBatteryEvents)
{
    std::vector<Battery> battery_events;

    battery_events.push_back(createBattery(4.1f, 0.34f));
    battery_events.push_back(createBattery(5.1f, 0.12f));
    battery_events.push_back(createBattery(2.4f, 0.99f));
    battery_events.push_back(createBattery(5.7f, 1.0f));

    checkSendsBatteryEvents(battery_events);
}

TEST_F(TelemetryServiceImplTest, registersToTelemetryFlightModeAsync)
{
    EXPECT_CALL(*_telemetry, flight_mode_async(_)).Times(1);

    std::vector<FlightMode> flight_mode_events;
    auto flight_mode_stream_future = subscribeFlightModeAsync(flight_mode_events);

    _telemetry_service->stop();
    flight_mode_stream_future.wait();
}

std::future<void> TelemetryServiceImplTest::subscribeFlightModeAsync(
    std::vector<FlightMode> &flight_mode_events) const
{
    return std::async(std::launch::async, [&]() {
        grpc::ClientContext context;
        dronecode_sdk::rpc::telemetry::SubscribeFlightModeRequest request;
        auto response_reader = _stub->SubscribeFlightMode(&context, request);

        dronecode_sdk::rpc::telemetry::FlightModeResponse response;
        while (response_reader->Read(&response)) {
            FlightMode flight_mode = translateRPCFlightMode(response.flight_mode());
            flight_mode_events.push_back(flight_mode);
        }

        response_reader->Finish();
    });
}

FlightMode
TelemetryServiceImplTest::translateRPCFlightMode(const RPCFlightMode rpc_flight_mode) const
{
    switch (rpc_flight_mode) {
        default:
        case RPCFlightMode::UNKNOWN:
            return FlightMode::UNKNOWN;
        case RPCFlightMode::READY:
            return FlightMode::READY;
        case RPCFlightMode::TAKEOFF:
            return FlightMode::TAKEOFF;
        case RPCFlightMode::HOLD:
            return FlightMode::HOLD;
        case RPCFlightMode::MISSION:
            return FlightMode::MISSION;
        case RPCFlightMode::RETURN_TO_LAUNCH:
            return FlightMode::RETURN_TO_LAUNCH;
        case RPCFlightMode::LAND:
            return FlightMode::LAND;
        case RPCFlightMode::OFFBOARD:
            return FlightMode::OFFBOARD;
        case RPCFlightMode::FOLLOW_ME:
            return FlightMode::FOLLOW_ME;
    }
}

TEST_F(TelemetryServiceImplTest, doesNotSendFlightModeInfoIfCallbackNotCalled)
{
    std::vector<FlightMode> flight_mode_events;
    auto flight_mode_stream_future = subscribeFlightModeAsync(flight_mode_events);

    _telemetry_service->stop();
    flight_mode_stream_future.wait();

    EXPECT_EQ(0, flight_mode_events.size());
}

TEST_F(TelemetryServiceImplTest, sendsOneFlightModeEvent)
{
    std::vector<FlightMode> flight_mode_events;
    flight_mode_events.push_back(FlightMode::UNKNOWN);

    checkSendsFlightModeEvents(flight_mode_events);
}

void TelemetryServiceImplTest::checkSendsFlightModeEvents(
    const std::vector<FlightMode> &flight_mode_events) const
{
    std::promise<void> subscription_promise;
    auto subscription_future = subscription_promise.get_future();
    dronecode_sdk::Telemetry::flight_mode_callback_t flight_mode_callback;
    EXPECT_CALL(*_telemetry, flight_mode_async(_))
        .WillOnce(SaveCallback(&flight_mode_callback, &subscription_promise));

    std::vector<FlightMode> received_flight_mode_events;
    auto flight_mode_stream_future = subscribeFlightModeAsync(received_flight_mode_events);
    subscription_future.wait();
    for (const auto flight_mode : flight_mode_events) {
        flight_mode_callback(flight_mode);
    }
    _telemetry_service->stop();
    flight_mode_stream_future.wait();

    ASSERT_EQ(flight_mode_events.size(), received_flight_mode_events.size());
    for (size_t i = 0; i < flight_mode_events.size(); i++) {
        EXPECT_EQ(flight_mode_events.at(i), received_flight_mode_events.at(i));
    }
}

TEST_F(TelemetryServiceImplTest, sendsMultipleFlightModeEvents)
{
    std::vector<FlightMode> flight_mode_events;
    flight_mode_events.push_back(FlightMode::UNKNOWN);
    flight_mode_events.push_back(FlightMode::READY);
    flight_mode_events.push_back(FlightMode::TAKEOFF);
    flight_mode_events.push_back(FlightMode::HOLD);
    flight_mode_events.push_back(FlightMode::MISSION);
    flight_mode_events.push_back(FlightMode::RETURN_TO_LAUNCH);
    flight_mode_events.push_back(FlightMode::LAND);
    flight_mode_events.push_back(FlightMode::OFFBOARD);
    flight_mode_events.push_back(FlightMode::FOLLOW_ME);

    checkSendsFlightModeEvents(flight_mode_events);
}

TEST_F(TelemetryServiceImplTest, registersToTelemetryAttitudeQuaternionAsync)
{
    EXPECT_CALL(*_telemetry, attitude_quaternion_async(_)).Times(1);

    std::vector<Quaternion> quaternions;
    auto quaternion_stream_future = subscribeAttitudeQuaternionAsync(quaternions);

    _telemetry_service->stop();
    quaternion_stream_future.wait();
}

std::future<void> TelemetryServiceImplTest::subscribeAttitudeQuaternionAsync(
    std::vector<Quaternion> &quaternions) const
{
    return std::async(std::launch::async, [&]() {
        grpc::ClientContext context;
        dronecode_sdk::rpc::telemetry::SubscribeAttitudeQuaternionRequest request;
        auto response_reader = _stub->SubscribeAttitudeQuaternion(&context, request);

        dronecode_sdk::rpc::telemetry::AttitudeQuaternionResponse response;
        while (response_reader->Read(&response)) {
            auto quaternion_rpc = response.attitude_quaternion();

            Quaternion quaternion;
            quaternion.w = quaternion_rpc.w();
            quaternion.x = quaternion_rpc.x();
            quaternion.y = quaternion_rpc.y();
            quaternion.z = quaternion_rpc.z();

            quaternions.push_back(quaternion);
        }

        response_reader->Finish();
    });
}

TEST_F(TelemetryServiceImplTest, doesNotSendAttitudeQuaternionIfCallbackNotCalled)
{
    std::vector<Quaternion> quaternions;
    auto quaternion_stream_future = subscribeAttitudeQuaternionAsync(quaternions);

    _telemetry_service->stop();
    quaternion_stream_future.wait();

    EXPECT_EQ(0, quaternions.size());
}

TEST_F(TelemetryServiceImplTest, sendsOneAttitudeQuaternion)
{
    std::vector<Quaternion> quaternions;
    quaternions.push_back(createQuaternion(0.1f, 0.2f, 0.3f, 0.4f));

    checkSendsAttitudeQuaternions(quaternions);
}

Quaternion TelemetryServiceImplTest::createQuaternion(const float w,
                                                      const float x,
                                                      const float y,
                                                      const float z) const
{
    dronecode_sdk::Telemetry::Quaternion quaternion;

    quaternion.w = w;
    quaternion.x = x;
    quaternion.y = y;
    quaternion.z = z;

    return quaternion;
}

void TelemetryServiceImplTest::checkSendsAttitudeQuaternions(
    const std::vector<Quaternion> &quaternions) const
{
    std::promise<void> subscription_promise;
    auto subscription_future = subscription_promise.get_future();
    dronecode_sdk::Telemetry::attitude_quaternion_callback_t attitude_quaternion_callback;
    EXPECT_CALL(*_telemetry, attitude_quaternion_async(_))
        .WillOnce(SaveCallback(&attitude_quaternion_callback, &subscription_promise));

    std::vector<Quaternion> received_quaternions;
    auto quaternion_stream_future = subscribeAttitudeQuaternionAsync(received_quaternions);
    subscription_future.wait();
    for (const auto quaternion : quaternions) {
        attitude_quaternion_callback(quaternion);
    }
    _telemetry_service->stop();
    quaternion_stream_future.wait();

    ASSERT_EQ(quaternions.size(), received_quaternions.size());
    for (size_t i = 0; i < quaternions.size(); i++) {
        EXPECT_EQ(quaternions.at(i), received_quaternions.at(i));
    }
}

TEST_F(TelemetryServiceImplTest, sendsMultipleAttitudeQuaternions)
{
    std::vector<Quaternion> quaternions;
    quaternions.push_back(createQuaternion(0.1f, 0.2f, 0.3f, 0.4f));
    quaternions.push_back(createQuaternion(2.1f, 0.4f, -2.2f, 0.3f));
    quaternions.push_back(createQuaternion(5.2f, 5.9f, 1.1f, 0.8f));

    checkSendsAttitudeQuaternions(quaternions);
}

TEST_F(TelemetryServiceImplTest, registersToTelemetryAttitudeEulerAsync)
{
    EXPECT_CALL(*_telemetry, attitude_euler_angle_async(_)).Times(1);

    std::vector<EulerAngle> euler_angles;
    auto euler_angle_stream_future = subscribeAttitudeEulerAsync(euler_angles);

    _telemetry_service->stop();
    euler_angle_stream_future.wait();
}

std::future<void>
TelemetryServiceImplTest::subscribeAttitudeEulerAsync(std::vector<EulerAngle> &euler_angles) const
{
    return std::async(std::launch::async, [&]() {
        grpc::ClientContext context;
        dronecode_sdk::rpc::telemetry::SubscribeAttitudeEulerRequest request;
        auto response_reader = _stub->SubscribeAttitudeEuler(&context, request);

        dronecode_sdk::rpc::telemetry::AttitudeEulerResponse response;
        while (response_reader->Read(&response)) {
            auto euler_angle_rpc = response.attitude_euler();

            EulerAngle euler_angle;
            euler_angle.roll_deg = euler_angle_rpc.roll_deg();
            euler_angle.pitch_deg = euler_angle_rpc.pitch_deg();
            euler_angle.yaw_deg = euler_angle_rpc.yaw_deg();

            euler_angles.push_back(euler_angle);
        }

        response_reader->Finish();
    });
}

TEST_F(TelemetryServiceImplTest, doesNotSendAttitudeEulerIfCallbackNotCalled)
{
    std::vector<EulerAngle> euler_angles;
    auto euler_angle_stream_future = subscribeAttitudeEulerAsync(euler_angles);

    _telemetry_service->stop();
    euler_angle_stream_future.wait();

    EXPECT_EQ(0, euler_angles.size());
}

TEST_F(TelemetryServiceImplTest, sendsOneAttitudeEuler)
{
    std::vector<EulerAngle> euler_angles;
    euler_angles.push_back(createEulerAngle(23.4f, 90.2f, 7.8f));

    checkSendsAttitudeEulerAngles(euler_angles);
}

EulerAngle TelemetryServiceImplTest::createEulerAngle(const float roll_deg,
                                                      const float pitch_deg,
                                                      const float yaw_deg) const
{
    EulerAngle euler_angle;
    euler_angle.roll_deg = roll_deg;
    euler_angle.pitch_deg = pitch_deg;
    euler_angle.yaw_deg = yaw_deg;

    return euler_angle;
}

void TelemetryServiceImplTest::checkSendsAttitudeEulerAngles(
    const std::vector<EulerAngle> &euler_angles) const
{
    std::promise<void> subscription_promise;
    auto subscription_future = subscription_promise.get_future();
    dronecode_sdk::Telemetry::attitude_euler_angle_callback_t attitude_euler_angle_callback;
    EXPECT_CALL(*_telemetry, attitude_euler_angle_async(_))
        .WillOnce(SaveCallback(&attitude_euler_angle_callback, &subscription_promise));

    std::vector<EulerAngle> received_euler_angles;
    auto euler_angle_stream_future = subscribeAttitudeEulerAsync(received_euler_angles);
    subscription_future.wait();
    for (const auto euler_angle : euler_angles) {
        attitude_euler_angle_callback(euler_angle);
    }
    _telemetry_service->stop();
    euler_angle_stream_future.wait();

    ASSERT_EQ(euler_angles.size(), received_euler_angles.size());
    for (size_t i = 0; i < euler_angles.size(); i++) {
        EXPECT_EQ(euler_angles.at(i), received_euler_angles.at(i));
    }
}

TEST_F(TelemetryServiceImplTest, sendsMultipleAttitudeEuler)
{
    std::vector<EulerAngle> euler_angles;
    euler_angles.push_back(createEulerAngle(12.2f, 11.8f, -54.2f));
    euler_angles.push_back(createEulerAngle(18.3f, 37.4f, -61.7f));
    euler_angles.push_back(createEulerAngle(6.3f, 34.11f, -36.5f));

    checkSendsAttitudeEulerAngles(euler_angles);
}

TEST_F(TelemetryServiceImplTest, registersToTelemetryCameraAttitudeQuaternionAsync)
{
    EXPECT_CALL(*_telemetry, camera_attitude_quaternion_async(_)).Times(1);

    std::vector<Quaternion> quaternions;
    auto quaternion_stream_future = subscribeCameraAttitudeQuaternionAsync(quaternions);

    _telemetry_service->stop();
    quaternion_stream_future.wait();
}

std::future<void> TelemetryServiceImplTest::subscribeCameraAttitudeQuaternionAsync(
    std::vector<Quaternion> &quaternions) const
{
    return std::async(std::launch::async, [&]() {
        grpc::ClientContext context;
        dronecode_sdk::rpc::telemetry::SubscribeCameraAttitudeQuaternionRequest request;
        auto response_reader = _stub->SubscribeCameraAttitudeQuaternion(&context, request);

        dronecode_sdk::rpc::telemetry::CameraAttitudeQuaternionResponse response;
        while (response_reader->Read(&response)) {
            auto quaternion_rpc = response.attitude_quaternion();

            Quaternion quaternion;
            quaternion.w = quaternion_rpc.w();
            quaternion.x = quaternion_rpc.x();
            quaternion.y = quaternion_rpc.y();
            quaternion.z = quaternion_rpc.z();

            quaternions.push_back(quaternion);
        }

        response_reader->Finish();
    });
}

TEST_F(TelemetryServiceImplTest, doesNotSendCameraAttitudeQuaternionIfCallbackNotCalled)
{
    std::vector<Quaternion> quaternions;
    auto quaternion_stream_future = subscribeCameraAttitudeQuaternionAsync(quaternions);

    _telemetry_service->stop();
    quaternion_stream_future.wait();

    EXPECT_EQ(0, quaternions.size());
}

TEST_F(TelemetryServiceImplTest, sendsOneCameraAttitudeQuaternion)
{
    std::vector<Quaternion> quaternions;
    quaternions.push_back(createQuaternion(0.1f, 0.2f, 0.3f, 0.4f));

    checkSendsCameraAttitudeQuaternions(quaternions);
}

void TelemetryServiceImplTest::checkSendsCameraAttitudeQuaternions(
    const std::vector<Quaternion> &quaternions) const
{
    std::promise<void> subscription_promise;
    auto subscription_future = subscription_promise.get_future();
    dronecode_sdk::Telemetry::attitude_quaternion_callback_t attitude_quaternion_callback;
    EXPECT_CALL(*_telemetry, camera_attitude_quaternion_async(_))
        .WillOnce(SaveCallback(&attitude_quaternion_callback, &subscription_promise));

    std::vector<Quaternion> received_quaternions;
    auto quaternion_stream_future = subscribeCameraAttitudeQuaternionAsync(received_quaternions);
    subscription_future.wait();
    for (const auto quaternion : quaternions) {
        attitude_quaternion_callback(quaternion);
    }
    _telemetry_service->stop();
    quaternion_stream_future.wait();

    ASSERT_EQ(quaternions.size(), received_quaternions.size());
    for (size_t i = 0; i < quaternions.size(); i++) {
        EXPECT_EQ(quaternions.at(i), received_quaternions.at(i));
    }
}

TEST_F(TelemetryServiceImplTest, sendsMultipleCameraAttitudeQuaternions)
{
    std::vector<Quaternion> quaternions;
    quaternions.push_back(createQuaternion(0.1f, 0.2f, 0.3f, 0.4f));
    quaternions.push_back(createQuaternion(2.1f, 0.4f, -2.2f, 0.3f));
    quaternions.push_back(createQuaternion(5.2f, 5.9f, 1.1f, 0.8f));

    checkSendsCameraAttitudeQuaternions(quaternions);
}

TEST_F(TelemetryServiceImplTest, registersToTelemetryCameraAttitudeEulerAsync)
{
    EXPECT_CALL(*_telemetry, camera_attitude_euler_angle_async(_)).Times(1);

    std::vector<EulerAngle> euler_angles;
    auto euler_angle_stream_future = subscribeCameraAttitudeEulerAsync(euler_angles);

    _telemetry_service->stop();
    euler_angle_stream_future.wait();
}

std::future<void> TelemetryServiceImplTest::subscribeCameraAttitudeEulerAsync(
    std::vector<EulerAngle> &euler_angles) const
{
    return std::async(std::launch::async, [&]() {
        grpc::ClientContext context;
        dronecode_sdk::rpc::telemetry::SubscribeCameraAttitudeEulerRequest request;
        auto response_reader = _stub->SubscribeCameraAttitudeEuler(&context, request);

        dronecode_sdk::rpc::telemetry::CameraAttitudeEulerResponse response;
        while (response_reader->Read(&response)) {
            auto euler_angle_rpc = response.attitude_euler();

            EulerAngle euler_angle;
            euler_angle.roll_deg = euler_angle_rpc.roll_deg();
            euler_angle.pitch_deg = euler_angle_rpc.pitch_deg();
            euler_angle.yaw_deg = euler_angle_rpc.yaw_deg();

            euler_angles.push_back(euler_angle);
        }

        response_reader->Finish();
    });
}

TEST_F(TelemetryServiceImplTest, doesNotSendCameraAttitudeEulerIfCallbackNotCalled)
{
    std::vector<EulerAngle> euler_angles;
    auto euler_angle_stream_future = subscribeCameraAttitudeEulerAsync(euler_angles);

    _telemetry_service->stop();
    euler_angle_stream_future.wait();

    EXPECT_EQ(0, euler_angles.size());
}

TEST_F(TelemetryServiceImplTest, sendsOneCameraAttitudeEuler)
{
    std::vector<EulerAngle> euler_angles;
    euler_angles.push_back(createEulerAngle(23.4f, 90.2f, 7.8f));

    checkSendsCameraAttitudeEulerAngles(euler_angles);
}

void TelemetryServiceImplTest::checkSendsCameraAttitudeEulerAngles(
    const std::vector<EulerAngle> &euler_angles) const
{
    std::promise<void> subscription_promise;
    auto subscription_future = subscription_promise.get_future();
    dronecode_sdk::Telemetry::attitude_euler_angle_callback_t attitude_euler_angle_callback;
    EXPECT_CALL(*_telemetry, camera_attitude_euler_angle_async(_))
        .WillOnce(SaveCallback(&attitude_euler_angle_callback, &subscription_promise));

    std::vector<EulerAngle> received_euler_angles;
    auto euler_angle_stream_future = subscribeCameraAttitudeEulerAsync(received_euler_angles);
    subscription_future.wait();
    for (const auto euler_angle : euler_angles) {
        attitude_euler_angle_callback(euler_angle);
    }
    _telemetry_service->stop();
    euler_angle_stream_future.wait();

    ASSERT_EQ(euler_angles.size(), received_euler_angles.size());
    for (size_t i = 0; i < euler_angles.size(); i++) {
        EXPECT_EQ(euler_angles.at(i), received_euler_angles.at(i));
    }
}

TEST_F(TelemetryServiceImplTest, sendsMultipleCameraAttitudeEuler)
{
    std::vector<EulerAngle> euler_angles;
    euler_angles.push_back(createEulerAngle(12.2f, 11.8f, -54.2f));
    euler_angles.push_back(createEulerAngle(18.3f, 37.4f, -61.7f));
    euler_angles.push_back(createEulerAngle(6.3f, 34.11f, -36.5f));

    checkSendsCameraAttitudeEulerAngles(euler_angles);
}

TEST_F(TelemetryServiceImplTest, registersToTelemetryGroundSpeedNedAsync)
{
    EXPECT_CALL(*_telemetry, ground_speed_ned_async(_)).Times(1);

    std::vector<GroundSpeedNed> ground_speed_events;
    auto ground_speed_stream_future = subscribeGroundSpeedNedAsync(ground_speed_events);

    _telemetry_service->stop();
    ground_speed_stream_future.wait();
}

std::future<void> TelemetryServiceImplTest::subscribeGroundSpeedNedAsync(
    std::vector<GroundSpeedNed> &ground_speed_events) const
{
    return std::async(std::launch::async, [&]() {
        grpc::ClientContext context;
        dronecode_sdk::rpc::telemetry::SubscribeGroundSpeedNedRequest request;
        auto response_reader = _stub->SubscribeGroundSpeedNed(&context, request);

        dronecode_sdk::rpc::telemetry::GroundSpeedNedResponse response;
        while (response_reader->Read(&response)) {
            auto ground_speed_rpc = response.ground_speed_ned();

            GroundSpeedNed ground_speed;
            ground_speed.velocity_north_m_s = ground_speed_rpc.velocity_north_m_s();
            ground_speed.velocity_east_m_s = ground_speed_rpc.velocity_east_m_s();
            ground_speed.velocity_down_m_s = ground_speed_rpc.velocity_down_m_s();

            ground_speed_events.push_back(ground_speed);
        }

        response_reader->Finish();
    });
}

TEST_F(TelemetryServiceImplTest, doesNotSendGroundSpeedNedIfCallbackNotCalled)
{
    std::vector<GroundSpeedNed> ground_speed_events;
    auto ground_speed_stream_future = subscribeGroundSpeedNedAsync(ground_speed_events);

    _telemetry_service->stop();
    ground_speed_stream_future.wait();

    EXPECT_EQ(0, ground_speed_events.size());
}

TEST_F(TelemetryServiceImplTest, sendsOneGroundSpeedEvent)
{
    std::vector<GroundSpeedNed> ground_speed_events;
    ground_speed_events.push_back(createGroundSpeedNed(12.3f, 1.2f, -0.2f));

    checkSendsGroundSpeedEvents(ground_speed_events);
}

GroundSpeedNed TelemetryServiceImplTest::createGroundSpeedNed(const float vel_north,
                                                              const float vel_east,
                                                              const float vel_down) const
{
    GroundSpeedNed ground_speed;

    ground_speed.velocity_north_m_s = vel_north;
    ground_speed.velocity_east_m_s = vel_east;
    ground_speed.velocity_down_m_s = vel_down;

    return ground_speed;
}

void TelemetryServiceImplTest::checkSendsGroundSpeedEvents(
    const std::vector<GroundSpeedNed> &ground_speed_events) const
{
    std::promise<void> subscription_promise;
    auto subscription_future = subscription_promise.get_future();
    dronecode_sdk::Telemetry::ground_speed_ned_callback_t ground_speed_ned_callback;
    EXPECT_CALL(*_telemetry, ground_speed_ned_async(_))
        .WillOnce(SaveCallback(&ground_speed_ned_callback, &subscription_promise));

    std::vector<GroundSpeedNed> received_ground_speed_events;
    auto ground_speed_stream_future = subscribeGroundSpeedNedAsync(received_ground_speed_events);
    subscription_future.wait();
    for (const auto ground_speed : ground_speed_events) {
        ground_speed_ned_callback(ground_speed);
    }
    _telemetry_service->stop();
    ground_speed_stream_future.wait();

    ASSERT_EQ(ground_speed_events.size(), received_ground_speed_events.size());
    for (size_t i = 0; i < ground_speed_events.size(); i++) {
        EXPECT_EQ(ground_speed_events.at(i), received_ground_speed_events.at(i));
    }
}

TEST_F(TelemetryServiceImplTest, sendsMultipleGroundSpeedEvents)
{
    std::vector<GroundSpeedNed> ground_speed_events;
    ground_speed_events.push_back(createGroundSpeedNed(2.3f, 22.1f, 1.1f));
    ground_speed_events.push_back(createGroundSpeedNed(5.23f, 1.2f, 4.0f));
    ground_speed_events.push_back(createGroundSpeedNed(-4.12f, -3.1f, 8.23f));

    checkSendsGroundSpeedEvents(ground_speed_events);
}

TEST_F(TelemetryServiceImplTest, registersToTelemetryRcStatusAsync)
{
    EXPECT_CALL(*_telemetry, rc_status_async(_)).Times(1);

    std::vector<RcStatus> rc_status_events;
    auto rc_status_stream_future = subscribeRcStatusAsync(rc_status_events);

    _telemetry_service->stop();
    rc_status_stream_future.wait();
}

std::future<void>
TelemetryServiceImplTest::subscribeRcStatusAsync(std::vector<RcStatus> &rc_status_events) const
{
    return std::async(std::launch::async, [&]() {
        grpc::ClientContext context;
        dronecode_sdk::rpc::telemetry::SubscribeRcStatusRequest request;
        auto response_reader = _stub->SubscribeRcStatus(&context, request);

        dronecode_sdk::rpc::telemetry::RcStatusResponse response;
        while (response_reader->Read(&response)) {
            auto rc_status_rpc = response.rc_status();

            RcStatus rc_status;
            rc_status.available_once = rc_status_rpc.was_available_once();
            rc_status.available = rc_status_rpc.is_available();
            rc_status.signal_strength_percent = rc_status_rpc.signal_strength_percent();

            rc_status_events.push_back(rc_status);
        }

        response_reader->Finish();
    });
}

TEST_F(TelemetryServiceImplTest, doesNotSendRcStatusIfCallbackNotCalled)
{
    std::vector<RcStatus> rc_status_events;
    auto rc_status_stream_future = subscribeRcStatusAsync(rc_status_events);

    _telemetry_service->stop();
    rc_status_stream_future.wait();

    EXPECT_EQ(0, rc_status_events.size());
}

TEST_F(TelemetryServiceImplTest, sendsOneRcStatusEvent)
{
    std::vector<RcStatus> rc_status_events;
    rc_status_events.push_back(createRcStatus(true, false, 33.0f));

    checkSendsRcStatusEvents(rc_status_events);
}

RcStatus TelemetryServiceImplTest::createRcStatus(const bool was_available_once,
                                                  const bool is_available,
                                                  const float signal_strength_percent) const
{
    RcStatus rc_status;

    rc_status.available_once = was_available_once;
    rc_status.available = is_available;
    rc_status.signal_strength_percent = signal_strength_percent;

    return rc_status;
}

void TelemetryServiceImplTest::checkSendsRcStatusEvents(
    const std::vector<RcStatus> &rc_status_events) const
{
    std::promise<void> subscription_promise;
    auto subscription_future = subscription_promise.get_future();
    dronecode_sdk::Telemetry::rc_status_callback_t rc_status_callback;
    EXPECT_CALL(*_telemetry, rc_status_async(_))
        .WillOnce(SaveCallback(&rc_status_callback, &subscription_promise));

    std::vector<RcStatus> received_rc_status_events;
    auto rc_status_stream_future = subscribeRcStatusAsync(received_rc_status_events);
    subscription_future.wait();
    for (const auto rc_status : rc_status_events) {
        rc_status_callback(rc_status);
    }
    _telemetry_service->stop();
    rc_status_stream_future.wait();

    ASSERT_EQ(rc_status_events.size(), received_rc_status_events.size());
    for (size_t i = 0; i < rc_status_events.size(); i++) {
        EXPECT_EQ(rc_status_events.at(i), received_rc_status_events.at(i));
    }
}

TEST_F(TelemetryServiceImplTest, sendsMultipleRcStatusEvents)
{
    std::vector<RcStatus> rc_status_events;
    rc_status_events.push_back(createRcStatus(false, false, 0.0f));
    rc_status_events.push_back(createRcStatus(false, true, 54.2f));
    rc_status_events.push_back(createRcStatus(true, true, 89.12));

    checkSendsRcStatusEvents(rc_status_events);
}

} // namespace
