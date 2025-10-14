/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// File DHSProtocol.cc
// Baudouin Raoult - (c) ECMWF Feb 12

#include "metkit/mars/DHSProtocol.h"

#include "eckit/config/Configuration.h"
#include "eckit/config/Resource.h"
#include "eckit/io/AutoCloser.h"
#include "eckit/net/Endpoint.h"
#include "eckit/net/IPAddress.h"
#include "eckit/net/TCPClient.h"
#include "eckit/net/TCPStream.h"
#include "eckit/utils/StringTools.h"
#include "eckit/utils/Translator.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars/ClientTask.h"
#include "metkit/mars/RequestEnvironment.h"


using namespace eckit;
using eckit::net::Endpoint;

namespace metkit {
namespace mars {

static Reanimator<DHSProtocol> dhsProtocolReanimator;

// ---------------------------------------------------------------------------------------------------------------------

namespace {

constexpr const int DEFAULT_CALLBACK_PROXY_PORT = 9707;

// Normally we would let the Endpoint constructor do this unpacking itself, but we want to set a default port

Endpoint unpackHostPort(const std::string& hoststr) {

    auto bits = StringTools::split(":", hoststr);
    ASSERT(!bits.empty() && bits.size() < 3);

    int port = DEFAULT_CALLBACK_PROXY_PORT;
    if (bits.size() == 2) {
        port = Translator<std::string, int>()(bits[1]);
    }

    return {bits[0], port};
}


Endpoint selectProxyHost(const std::vector<std::string> proxies) {
    return unpackHostPort(proxies[std::rand() % proxies.size()]);
}


Endpoint selectProxyHost(const Configuration& config) {
    if (config.has("proxyHost")) {
        return unpackHostPort(config.getString("proxyHost"));
    }

    if (config.has("proxyHosts")) {
        return selectProxyHost(config.getStringVector("proxyHosts"));
    }

    throw UserError("Neither proxyHosts nor proxyHost specified in configuration");
}
}  // namespace


// Implement the default callback behaviour. Client opens a socket that can be connected to by
// the server or data mover

class SimpleCallback : public BaseCallbackConnection {

public:

    SimpleCallback() : callback_(Resource<int>("$MARS_DHS_LOCALPORT", 0)), callbackEndpoint_(computeEndpoint()) {
        LOG_DEBUG_LIB(LibMetkit) << "Simple callback. local port=" << callback_.localPort()
                                 << " host=" << callbackEndpoint_.host() << " port=" << callbackEndpoint_.port()
                                 << std::endl;
    }
    explicit SimpleCallback(const Configuration&) : SimpleCallback() {}
    explicit SimpleCallback(Stream&) : SimpleCallback() {}

private:

    const Endpoint& endpoint() const override { return callbackEndpoint_; }

    net::TCPSocket& connect() override { return callback_.accept(); }

    net::EphemeralTCPServer callback_;
    Endpoint callbackEndpoint_;

    Endpoint computeEndpoint() const {
        static std::string callbackHost = Resource<std::string>("$MARS_DHS_CALLBACK_HOST", "");
        static int callbackPort         = Resource<int>("$MARS_DHS_CALLBACK_PORT", 0);

        if (callbackHost.empty()) {
            return Endpoint{net::IPAddress::hostAddress(callback_.localHost()).asString(), callback_.localPort()};
        }
        if (callbackPort == 0) {
            return Endpoint{callbackHost, callback_.localPort()};
        }
        return Endpoint{callbackHost, callbackPort};
    }

public:

    static const ClassSpec& classSpec() {
        static ClassSpec spec = {&BaseCallbackConnection::classSpec(), "SimpleCallback"};
        return spec;
    }

protected:

    const ReanimatorBase& reanimator() const override { return reanimator_; }
    void encode(Stream&) const override {}
    static Reanimator<SimpleCallback> reanimator_;
};

Reanimator<SimpleCallback> SimpleCallback::reanimator_;

// ---------------------------------------------------------------------------------------------------------------------

// Implement a callback via the callback proxy. First we open a connection to the proxy. This then returns to us
// the host/port that it has made available for the server/mover to connect to. It then proxies the connection
// to our host/port

class ProxyCallback : public BaseCallbackConnection {

public:

    explicit ProxyCallback(const Endpoint& proxyhost) :
        control_(net::TCPClient().connect(proxyhost)), proxyHost_(proxyhost) {

        LOG_DEBUG_LIB(LibMetkit) << "Proxy callback. proxyhost=" << proxyhost.host()
                                 << " proxyport=" << proxyhost.port() << std::endl;

        std::string localAddr = net::IPAddress::hostAddress(callback_.localHost()).asString();
        int localPort         = callback_.localPort();

        bool passive = false;
        control_ << localAddr;
        control_ << localPort;
        control_ << passive;

        remoteAddr_ = Endpoint(control_);
    }

    explicit ProxyCallback(const Configuration& config) : ProxyCallback(selectProxyHost(config)) {}

    explicit ProxyCallback(Stream& s) : ProxyCallback(Endpoint(s)) {}

private:

    const Endpoint& endpoint() const override { return remoteAddr_; }

    net::TCPSocket& connect() override {
        // FIXME: Check that the callback connection is still alive...
        return callback_.accept();
    }

    net::TCPStream control_;
    net::EphemeralTCPServer callback_;

    Endpoint proxyHost_;
    Endpoint remoteAddr_;

public:

    static const ClassSpec& classSpec() {
        static ClassSpec spec = {&BaseCallbackConnection::classSpec(), "ProxyCallback"};
        return spec;
    }

protected:

    const ReanimatorBase& reanimator() const override { return reanimator_; }
    void encode(Stream& s) const override { s << proxyHost_; }
    static Reanimator<ProxyCallback> reanimator_;
};

Reanimator<ProxyCallback> ProxyCallback::reanimator_;

// ---------------------------------------------------------------------------------------------------------------------

// Implement a callback via the callback proxy using passive mode. First we open a connection to the proxy.
// This then returns to us the host/port that it has made available for the server/mover to connect to.
// We then need to open connections to the proxy for the callbacks to connect to.

class PassiveProxyCallback : public BaseCallbackConnection {

public:

    explicit PassiveProxyCallback(const Endpoint& proxyhost, bool useProxyHostAsCallback = true) :
        control_(net::TCPClient().connect(proxyhost)), proxyHost_(proxyhost) {

        LOG_DEBUG_LIB(LibMetkit) << "Passive proxy callback. proxyhost=" << proxyhost << std::endl;

        std::string localAddr = "<invalid>";
        int localPort         = -1;

        bool passive = true;
        control_ << localAddr;
        control_ << localPort;
        control_ << passive;

        remoteAddr_ = Endpoint(control_);

        LOG_DEBUG_LIB(LibMetkit) << "Remote address. host=" << remoteAddr_ << std::endl;

        passiveAddr_ = Endpoint(control_);

        if (useProxyHostAsCallback) {
            passiveAddr_ = Endpoint(proxyhost.host(), passiveAddr_.port());
        }

        LOG_DEBUG_LIB(LibMetkit) << "Passive address. host=" << passiveAddr_ << std::endl;

        control_ >> passiveCheck_;

        LOG_DEBUG_LIB(LibMetkit) << "Passive address. host=" << passiveAddr_ << " check=" << passiveCheck_ << std::endl;
    }

    explicit PassiveProxyCallback(const Configuration& config) :
        PassiveProxyCallback(selectProxyHost(config), config.getBool("useProxyHostAsCallback", true)) {}

    explicit PassiveProxyCallback(Stream& s) : PassiveProxyCallback(Endpoint(s)) {}

private:

    const Endpoint& endpoint() const override { return remoteAddr_; }

    net::TCPSocket& connect() override {
        // FIXME: Check that the callback connection is still alive...
        ASSERT(!socket_.isConnected());
        socket_ = net::TCPClient().connect(passiveAddr_);
        net::InstantTCPStream s(socket_);
        s << passiveCheck_;
        return socket_;
    }

    net::TCPStream control_;
    net::TCPSocket socket_;

    Endpoint proxyHost_;
    Endpoint remoteAddr_;
    Endpoint passiveAddr_;

    unsigned long long passiveCheck_;

public:

    static const ClassSpec& classSpec() {
        static ClassSpec spec = {&BaseCallbackConnection::classSpec(), "PassiveProxyCallback"};
        return spec;
    }

protected:

    const ReanimatorBase& reanimator() const override { return reanimator_; }
    void encode(Stream& s) const override { s << proxyHost_; }
    static Reanimator<PassiveProxyCallback> reanimator_;
};

Reanimator<PassiveProxyCallback> PassiveProxyCallback::reanimator_;


// ---------------------------------------------------------------------------------------------------------------------

const ClassSpec& BaseCallbackConnection::classSpec() {
    static ClassSpec spec = {&Streamable::classSpec(), "BaseCallbackConnection"};
    return spec;
}

BaseCallbackConnection* BaseCallbackConnection::build(const Configuration& config, const std::string& host) {
    if (config.has("proxyHost") || config.has("proxyHosts") ||
        (config.getBool("passiveProxy", true) && config.getBool("useHostAsProxy", false))) {
        if (config.getBool("passiveProxy", true)) {
            if (config.getBool("useHostAsProxy", false)) {
                return new PassiveProxyCallback{unpackHostPort(host)};
            }
            return new PassiveProxyCallback{config};
        }
        return new ProxyCallback{config};
    }

    static bool passiveProxy = Resource<bool>("$MARS_DHS_PASSIVE_PROXY", true);
    static std::vector<std::string> proxyHosts =
        Resource<std::vector<std::string>>("$MARS_DHS_CALLBACK_PROXY_HOST", {});

    if (!proxyHosts.empty()) {
        Endpoint proxyHost = selectProxyHost(proxyHosts);

        if (passiveProxy) {
            return new PassiveProxyCallback(proxyHost);
        }
        else {
            return new ProxyCallback(proxyHost);
        }
    }

    return new SimpleCallback{config};
}

// ---------------------------------------------------------------------------------------------------------------------

DHSProtocol::DHSProtocol(const std::string& name, const std::string& host, int port, bool forwardMessages) :
    callback_(new SimpleCallback{}),
    name_(name),
    host_(host),
    port_(port),
    done_(false),
    error_(false),
    sending_(false),
    forward_(forwardMessages) {}

DHSProtocol::DHSProtocol(const Configuration& params) :
    BaseProtocol(params),
    name_(params.getString("name")),
    port_(params.getInt("port", 9000)),
    done_(false),
    error_(false),
    sending_(false),
    forward_(false) {
    if (params.has("hosts")) {
        std::vector<std::string> hosts = params.getStringVector("hosts");
        host_                          = hosts.at(std::rand() % hosts.size());
    }
    else {
        ASSERT(params.has("host"));
        host_ = params.getString("host");
    }
    callback_.reset(BaseCallbackConnection::build(params, host_));
}

DHSProtocol::DHSProtocol(const Configuration& params, const std::map<std::string, std::string>& env) :
    DHSProtocol(params) {
    auto requestEnv = RequestEnvironment::instance();
    requestEnv.update(env);
    env_ = requestEnv.request();
}

DHSProtocol::DHSProtocol(Stream& s) : BaseProtocol(s), callback_(Reanimator<BaseCallbackConnection>::reanimate(s)) {
    s >> name_;
    s >> host_;
    s >> port_;
    s >> done_;
    s >> error_;
    s >> sending_;
    s >> forward_;
    env_ = MarsRequest(s);
}

DHSProtocol::~DHSProtocol() {
    done_ = true;
    cleanup();
}

const ReanimatorBase& DHSProtocol::reanimator() const {
    return dhsProtocolReanimator;
}

const ClassSpec& DHSProtocol::classSpec() {
    static ClassSpec spec = {&BaseProtocol::classSpec(), "DHSProtocol"};
    return spec;
}

Length DHSProtocol::retrieve(const MarsRequest& request) {
    Endpoint callbackEndpoint = callback_->endpoint();

    LOG_DEBUG_LIB(LibMetkit) << "DHSProtocol: call back on " << callbackEndpoint << std::endl;


    task_.reset(new ClientTask(request, env_, callbackEndpoint.host(), callbackEndpoint.port()));

    net::TCPStream s(net::TCPClient().connect(host_, port_));

    task_->send(s);

    ASSERT(task_->receive(s) == 'a');  // Acknoledgement

    Length result = 0;
    while (wait(result)) {}

    LOG_DEBUG_LIB(LibMetkit) << "DHSProtocol::retrieve " << result << std::endl;
    return result;
}

void DHSProtocol::archive(const MarsRequest& request, const Length& size) {
    Endpoint callbackEndpoint = callback_->endpoint();

    LOG_DEBUG_LIB(LibMetkit) << "DHSProtocol::archive " << size << std::endl;
    LOG_DEBUG_LIB(LibMetkit) << "DHSProtocol: call back on " << callbackEndpoint << std::endl;

    task_.reset(new ClientTask(request, env_, callbackEndpoint.host(), callbackEndpoint.port()));

    net::TCPStream s(net::TCPClient().connect(host_, port_));

    task_->send(s);

    ASSERT(task_->receive(s) == 'a');  // Acknoledgement

    Length result = size;
    while (wait(result)) {}
    LOG_DEBUG_LIB(LibMetkit) << "DHSProtocol: archive completed." << std::endl;
}

void DHSProtocol::cleanup() {
    if (socket_.isConnected()) {
        if (sending_) {
            unsigned long version  = 1;
            unsigned long long crc = 0;

            try {
                net::InstantTCPStream s(socket_);
                s << version;
                s << crc;
            }
            catch (std::exception& e) {
                Log::error() << "** " << e.what() << " Caught in " << Here() << std::endl;
                Log::error() << "** Exception is ignored" << std::endl;
            }
        }
        socket_.close();
    }

    sending_ = false;

    if (!done_) {
        Length result = 0;
        while (wait(result)) {
            ;
        }
    }

    if (error_) {
        error_ = false;
        throw UserError(std::string("Error from [") + name_ + "]: " + msg_);
    }
}

void DHSProtocol::print(std::ostream& s) const {
    s << "DHSProtocol[" << name_ << "]";
}

void DHSProtocol::encode(Stream& s) const {
    BaseProtocol::encode(s);
    callback_->encode(s);
    s << name_;
    s << host_;
    s << port_;
    s << done_;
    s << error_;
    s << sending_;
    s << forward_;
    s << env_;
}

long DHSProtocol::read(void* buffer, long len) {
    return socket_.read(buffer, len);
}

long DHSProtocol::write(const void* buffer, long len) {
    return socket_.write(buffer, len);
}

bool DHSProtocol::wait(Length& size) {
    for (;;) {

        if (socket_.isConnected())
            socket_.close();
        socket_ = callback_->connect();

        net::InstantTCPStream s(socket_);

        char code = task_->receive(s);

        LOG_DEBUG_LIB(LibMetkit) << "DHSProtocol: code [" << code << "]" << std::endl;

        std::string msg;
        long long bytes;

        switch (code) {
            /* OK */
            case 'o':
                done_ = true;
                return false;

            /* read source */
            case 'r':
                bytes = size;
                LOG_DEBUG_LIB(LibMetkit) << "DHSProtocol:r [" << bytes << "]" << std::endl;
                s << bytes;
                sending_ = true;
                return false;

            /* get */
            case 'h':
                NOTIMP;

            case 'w':
                s >> bytes;
                LOG_DEBUG_LIB(LibMetkit) << "DHSProtocol:w " << bytes << std::endl;
                size = bytes;
                return false;

            case 'm':
                NOTIMP;

            case 'X':
                NOTIMP;

            case 'e':
                s >> msg_;
                Log::error() << msg_ << " [" << name_ << "]" << std::endl;
                error_ = true;
                done_  = true;
                return false;

            case 'y': /* retry */
                NOTIMP;

            case 'I': /* info */
                s >> msg;
                Log::info() << msg << " [" << name_ << "]" << std::endl;
                if (forward_) {
                    Log::userInfo() << msg << " [" << name_ << "]" << std::endl;
                }
                break;

            case 'W': /* warning */
                s >> msg;
                Log::warning() << msg << " [" << name_ << "]" << std::endl;
                if (forward_) {
                    Log::userWarning() << msg << " [" << name_ << "]" << std::endl;
                }
                break;

            case 'D': /* debug */
                s >> msg;
                LOG_DEBUG_LIB(LibMetkit) << msg << " [" << name_ << "]" << std::endl;
                if (forward_) {
                    Log::userInfo() << msg << " [" << name_ << "]" << std::endl;
                }
                break;

            case 'E': /* error */
                s >> msg;
                Log::error() << msg << " [" << name_ << "]" << std::endl;
                if (forward_) {
                    Log::userError() << msg << " [" << name_ << "]" << std::endl;
                }
                break;

            case 'N': /* notification */
                NOTIMP;

            case 'p': /* ping */
                s << 'p';
                break;

            case 's': /* statistics */
            {
                int n;
                s >> n;
                std::string key, value;
                for (int i = 0; i < n; i++) {
                    s >> key >> value;
                    Log::info() << "DHSProtocol:s " << key << "=" << value << std::endl;
                }
            } break;

            case 'S': /* notification start */
                NOTIMP;

            case 't': /* new timeout */
                NOTIMP;

            default:
                throw Exception(std::string("Unknown code [") + code + "]");
        }
    }
}

static ProtocolBuilder<DHSProtocol> builder("dhsbase");

}  // namespace mars
}  // namespace metkit
