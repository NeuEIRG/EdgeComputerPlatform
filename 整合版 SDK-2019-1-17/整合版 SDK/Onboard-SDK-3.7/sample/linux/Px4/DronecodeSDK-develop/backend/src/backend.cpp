#include "backend.h"

#include <memory>

#include "connection_initiator.h"
#include "dronecode_sdk.h"
#include "grpc_server.h"

namespace dronecode_sdk {
namespace backend {

class DronecodeSDKBackend::Impl {
public:
    Impl() {}
    ~Impl() {}

    void connect(const int mavlink_listen_port)
    {
        _connection_initiator.start(_dc, 14540);
        _connection_initiator.wait();
    }

    void startGRPCServer()
    {
        _server = std::unique_ptr<GRPCServer>(new GRPCServer(_dc));
        _server->run();
    }

    void wait() { _server->wait(); }

private:
    DronecodeSDK _dc;
    ConnectionInitiator<dronecode_sdk::DronecodeSDK> _connection_initiator;
    std::unique_ptr<GRPCServer> _server;
};

DronecodeSDKBackend::DronecodeSDKBackend() : _impl(new Impl()) {}
DronecodeSDKBackend::~DronecodeSDKBackend() = default;

void DronecodeSDKBackend::startGRPCServer()
{
    _impl->startGRPCServer();
}
void DronecodeSDKBackend::connect(const int mavlink_listen_port)
{
    return _impl->connect(mavlink_listen_port);
}
void DronecodeSDKBackend::wait()
{
    _impl->wait();
}

} // namespace backend
} // namespace dronecode_sdk
