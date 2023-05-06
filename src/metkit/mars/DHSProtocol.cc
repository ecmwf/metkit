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
#include "eckit/net/IPAddress.h"
#include "eckit/net/TCPClient.h"
#include "eckit/net/TCPStream.h"
#include "eckit/io/AutoCloser.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars/ClientTask.h"
#include "metkit/mars/RequestEnvironment.h"


namespace metkit {
namespace mars {

static eckit::Reanimator<DHSProtocol> dhsProtocolReanimator;

// ---------------------------------------------------------------------------------------------------------------------

// Implement the default callback behaviour. Client opens a socket that can be connected to by
// the server or data mover

class SimpleCallback : public BaseCallbackConnection {

public:

    SimpleCallback() {
        LOG_DEBUG_LIB(LibMetkit) << "Simple callback. host=" << host() << " port=" << port() << std::endl;
    }
    SimpleCallback(const eckit::Configuration&) : SimpleCallback() {}
    SimpleCallback(eckit::Stream&) : SimpleCallback() {}

private:

    std::string host() const override {
        return eckit::net::IPAddress::hostAddress(callback_.localHost()).asString();
    }

    int port() const override {
        return callback_.localPort();
    }

    eckit::net::TCPSocket& connect() override {
        return callback_.accept();
    }

    eckit::net::EphemeralTCPServer callback_;

public:
    static const eckit::ClassSpec&  classSpec() {
        static eckit::ClassSpec spec = {&BaseCallbackConnection::classSpec(), "SimpleCallback"};
        return spec;
    }
protected:
    virtual const eckit::ReanimatorBase& reanimator() const override { return reanimator_; }
    virtual void encode(eckit::Stream&) const override {}
    static eckit::Reanimator<SimpleCallback> reanimator_;
};

eckit::Reanimator<SimpleCallback> SimpleCallback::reanimator_;

// ---------------------------------------------------------------------------------------------------------------------

// Implement a callback via the callback proxy. First we open a connection to the proxy. This then returns to us
// the host/port that it has made available for the server/mover to connect to. It then proxies the connection
// to our host/port

class ProxyCallback : public BaseCallbackConnection {

public:

    ProxyCallback(const std::string& proxyhost, int proxyport) :
        control_(eckit::net::TCPClient().connect(proxyhost, proxyport)),
        proxyHost_(proxyhost),
        proxyPort_(proxyport) {

        LOG_DEBUG_LIB(LibMetkit) << "Proxy callback. proxyhost=" << proxyhost
                                 << " proxyport=" << proxyport << std::endl;

        std::string localAddr = eckit::net::IPAddress::hostAddress(callback_.localHost()).asString();
        int localPort         = callback_.localPort();

        bool passive = false;
        control_ << localAddr;
        control_ << localPort;
        control_ << passive;

        control_ >> remoteAddr_;
        control_ >> remotePort_;
    }

    ProxyCallback(const eckit::Configuration& config) :
        ProxyCallback(config.getString("proxyHost"), config.getInt("proxyPort")) {}

    ProxyCallback(eckit::Stream& s) :
        ProxyCallback(readStreamHost(s), readStreamPort(s)) {}

private:

    std::string readStreamHost(eckit::Stream& s) {
        std::string h;
        s >> h;
        return h;
    }

    int readStreamPort(eckit::Stream& s) {
        int p;
        s >> p;
        return p;
    }

    std::string host() const override {
        return remoteAddr_;
    }

    int port() const override {
        return remotePort_;
    }

    eckit::net::TCPSocket& connect() override {
        // FIXME: Check that the callback connection is still alive...
        return callback_.accept();
    }

    eckit::net::TCPStream control_;
    eckit::net::EphemeralTCPServer callback_;

    std::string proxyHost_;
    int proxyPort_;

    std::string remoteAddr_;
    int remotePort_;

public:
    static const eckit::ClassSpec&  classSpec() {
        static eckit::ClassSpec spec = {&BaseCallbackConnection::classSpec(), "ProxyCallback"};
        return spec;
    }
protected:
    virtual const eckit::ReanimatorBase& reanimator() const override { return reanimator_; }
    virtual void encode(eckit::Stream& s) const override {
        s << proxyHost_;
        s << proxyPort_;
    }
    static eckit::Reanimator<ProxyCallback> reanimator_;
};

eckit::Reanimator<ProxyCallback> ProxyCallback::reanimator_;

// ---------------------------------------------------------------------------------------------------------------------

// Implement a callback via the callback proxy using passive mode. First we open a connection to the proxy.
// This then returns to us the host/port that it has made available for the server/mover to connect to.
// We then need to open connections to the proxy for the callbacks to connect to.

class PassiveProxyCallback : public BaseCallbackConnection {

public:

    PassiveProxyCallback(const std::string& proxyhost, int proxyport) :
        control_(eckit::net::TCPClient().connect(proxyhost, proxyport)),
        proxyHost_(proxyhost),
        proxyPort_(proxyport) {

        LOG_DEBUG_LIB(LibMetkit) << "Passive proxy callback. proxyhost=" << proxyhost
                                 << " proxyport=" << proxyport << std::endl;

        std::string localAddr = "<invalid>";
        int localPort         = -1;

        bool passive = true;
        control_ << localAddr;
        control_ << localPort;
        control_ << passive;

        control_ >> remoteAddr_;
        control_ >> remotePort_;

        LOG_DEBUG_LIB(LibMetkit) << "Remote address. host=" << remoteAddr_ << " port=" << remotePort_ << std::endl;

        control_ >> passiveAddr_;
        control_ >> passivePort_;
        control_ >> passiveCheck_;

        LOG_DEBUG_LIB(LibMetkit) << "Passive address. host=" << passiveAddr_ << " port=" << passivePort_
                                 << " check=" << passiveCheck_ << std::endl;
    }

    PassiveProxyCallback(const eckit::Configuration& config) :
        PassiveProxyCallback(config.getString("proxyHost"), config.getInt("proxyPort")) {}

    PassiveProxyCallback(eckit::Stream& s) :
        PassiveProxyCallback(readStreamHost(s), readStreamPort(s)) {}

private:

    std::string readStreamHost(eckit::Stream& s) {
        std::string h;
        s >> h;
        return h;
    }

    int readStreamPort(eckit::Stream& s) {
        int p;
        s >> p;
        return p;
    }

    std::string host() const override {
        return remoteAddr_;
    }

    int port() const override {
        return remotePort_;
    }

    eckit::net::TCPSocket& connect() override {
        // FIXME: Check that the callback connection is still alive...
        ASSERT(!socket_.isConnected());
        socket_ = eckit::net::TCPClient().connect(passiveAddr_, passivePort_);
        eckit::net::InstantTCPStream s(socket_);
        s << passiveCheck_;
        return socket_;
    }

    eckit::net::TCPStream control_;
    eckit::net::TCPSocket socket_;

    std::string proxyHost_;
    std::string remoteAddr_;
    std::string passiveAddr_;
    int proxyPort_;
    int remotePort_;
    int passivePort_;

    unsigned long long passiveCheck_;

public:
    static const eckit::ClassSpec&  classSpec() {
        static eckit::ClassSpec spec = {&BaseCallbackConnection::classSpec(), "PassiveProxyCallback"};
        return spec;
    }
protected:
    virtual const eckit::ReanimatorBase& reanimator() const override { return reanimator_; }
    virtual void encode(eckit::Stream& s) const override {
        s << proxyHost_;
        s << proxyPort_;
    }
    static eckit::Reanimator<PassiveProxyCallback> reanimator_;
};

eckit::Reanimator<PassiveProxyCallback> PassiveProxyCallback::reanimator_;


// ---------------------------------------------------------------------------------------------------------------------

const eckit::ClassSpec& BaseCallbackConnection::classSpec() {
    static eckit::ClassSpec spec = { &Streamable::classSpec(), "BaseCallbackConnection" };
    return spec;
}

BaseCallbackConnection* BaseCallbackConnection::build(const eckit::Configuration& config) {
    if (config.has("proxyHost")) {
        if (config.getBool("passiveProxy", false)) {
            return new PassiveProxyCallback{config};
        }
        return new ProxyCallback{config};
    }

    static bool passiveProxy = eckit::Resource<bool>("$MARS_CLIENT_PASSIVE_PROXY", false);
    static std::string proxyHost = eckit::Resource<std::string>("$MARS_CLIENT_CALLBACK_PROXY_HOST", "");
    static int proxyPort = eckit::Resource<int>("$MARS_CLIENT_CALLBACK_PROXY_PORT", -1);

    if (!proxyHost.empty()) {
        ASSERT(proxyPort != -1);
        if (passiveProxy) {
            return new PassiveProxyCallback(proxyHost, proxyPort);
        } else {
            return new ProxyCallback(proxyHost, proxyPort);
        }
    }

    return new SimpleCallback{config};
}

// ---------------------------------------------------------------------------------------------------------------------

DHSProtocol::DHSProtocol(const std::string& name,
                         const std::string& host,
                         int port,
                         bool forwardMessages ) :
    callback_(new SimpleCallback{}),
    name_(name),
    host_(host),
    port_(port),
    done_(false),
    error_(false),
    sending_(false),
    forward_(forwardMessages) {}

DHSProtocol::DHSProtocol(const eckit::Configuration& params):
    BaseProtocol(params),
    callback_(BaseCallbackConnection::build(params)),
    name_(params.getString("name")),
    host_(params.getString("host")),
    port_(params.getInt("port", 9000)),
    done_(false),
    error_(false),
    sending_(false),
    forward_(false)
{
}

DHSProtocol::DHSProtocol(eckit::Stream& s):
    BaseProtocol(s),
    callback_(eckit::Reanimator<BaseCallbackConnection>::reanimate(s)) {
    s >> name_;
    s >> host_;
    s >> port_;
    s >> done_;
    s >> error_;
    s >> sending_;
    s >> forward_;
}

DHSProtocol::~DHSProtocol()
{
    done_ = true;
    cleanup();
}

const eckit::ReanimatorBase& DHSProtocol::reanimator() const {
    return dhsProtocolReanimator;
}

const eckit::ClassSpec& DHSProtocol::classSpec() {
    static eckit::ClassSpec spec = { &BaseProtocol::classSpec(), "DHSProtocol" };
    return spec;
}

eckit::Length DHSProtocol::retrieve(const MarsRequest& request)
{
    std::string addr = callback_->host();
    int         port = callback_->port();

    eckit::Log::info() << "DHSProtocol: call back on " << addr << ":" << port << std::endl;

    task_.reset(new ClientTask(request, RequestEnvironment::instance().request(), addr, port));

    eckit::net::TCPStream s(eckit::net::TCPClient().connect(host_, port_));

    task_->send(s);

    ASSERT(task_->receive(s) == 'a'); // Acknoledgement

    eckit::Length result = 0;
    while (wait(result)) {
    }

    eckit::Log::info() << "DHSProtocol::retrieve " << result << std::endl;
    return result;
}

void DHSProtocol::archive(const MarsRequest& request, const eckit::Length& size)
{
    std::string addr = callback_->host();
    int         port = callback_->port();

    eckit::Log::info() << "DHSProtocol::archive " << size << std::endl;
    eckit::Log::info() << "DHSProtocol: call back on " << addr << ":" << port << std::endl;

    task_.reset(new ClientTask(request, RequestEnvironment::instance().request(), addr, port));

    eckit::net::TCPStream s(eckit::net::TCPClient().connect(host_, port_));

    task_->send(s);

    eckit::Log::info() << "DHSProtocol: task sent." << std::endl;

    ASSERT(task_->receive(s) == 'a'); // Acknoledgement

    eckit::Length result = size;
    while (wait(result)) {
    }
    eckit::Log::info() << "DHSProtocol: archive completed." << std::endl;
}

void DHSProtocol::cleanup()
{
    if (socket_.isConnected())
    {
        if ( sending_ )
        {
            unsigned long version = 1;
            unsigned long long crc = 0;

            try {
                eckit::net::InstantTCPStream s(socket_);
                s << version;
                s << crc;
            }
            catch (std::exception& e)
            {
                eckit::Log::error() << "** " << e.what() << " Caught in " << Here() << std::endl;
                eckit::Log::error() << "** Exception is ignored" << std::endl;
            }
        }
        socket_.close();
    }

    sending_ = false;

    if (!done_) {
        eckit::Length result = 0;
        while (wait(result)) {
            ;
        }
    }

    if (error_)
    {
        error_ = false;
        throw eckit::UserError(std::string("Error from [") + name_ + "]: "  + msg_);
    }
}

void DHSProtocol::print(std::ostream& s) const
{
    s << "DHSProtocol[" << name_ << "]";
}

void DHSProtocol::encode(eckit::Stream& s) const {
    BaseProtocol::encode(s);
    callback_->encode(s);
    s << name_;
    s << host_;
    s << port_;
    s << done_;
    s << error_;
    s << sending_;
    s << forward_;
}

long DHSProtocol::read(void* buffer, long len)
{
    return socket_.read(buffer, len);
}

long DHSProtocol::write(const void* buffer, long len)
{
    return socket_.write(buffer, len);
}

bool DHSProtocol::wait(eckit::Length& size)
{
    for (;;) {

        if (socket_.isConnected()) socket_.close();
        socket_ = callback_->connect();

        eckit::net::InstantTCPStream s(socket_);

        char code = task_->receive(s);

        eckit::Log::debug() << "DHSProtocol: code [" << code << "]" << std::endl;

        std::string msg;
        long long bytes;

        switch (code) {
        /* OK */
        case 'o':
            done_ = true;
            return false;
            break;

        /* read source */
        case 'r':
            bytes = size;
            eckit::Log::debug() << "DHSProtocol:r [" << bytes << "]" << std::endl;
            s << bytes;
            sending_ = true;
            return false;
            break;

        /* get */
        case 'h':
            NOTIMP;
            break;

        case 'w':
            s >> bytes;
            eckit::Log::debug() << "DHSProtocol:w " << bytes << std::endl;
            size = bytes;
            return false;
            break;

        case 'm':
            NOTIMP;
            break;

        case 'X':
            NOTIMP;
            break;

        case 'e':
            s >> msg_;
            eckit::Log::error() << msg_ << " [" << name_ << "]" << std::endl;
            error_ = true;
            done_ = true;
            return false;
            break;

        case 'y':    /* retry */
            NOTIMP;
            break;

        case 'I': /* info */
            s >> msg;
            eckit::Log::info() << msg << " [" << name_ << "]" << std::endl;
            if (forward_) {
                eckit::Log::userInfo() << msg << " [" << name_ << "]" << std::endl;
            }
            break;

        case 'W': /* warning */
            s >> msg;
            eckit::Log::warning() << msg << " [" << name_ << "]" << std::endl;
            if (forward_) {
                eckit::Log::userWarning() << msg << " [" << name_ << "]" << std::endl;
            }
            break;

        case 'D': /* debug */
            s >> msg;
            eckit::Log::debug() << msg << " [" << name_ << "]" << std::endl;
            if (forward_) {
                eckit::Log::userInfo() << msg << " [" << name_ << "]" << std::endl;
            }
            break;

        case 'E': /* error */
            s >> msg;
            eckit::Log::error() << msg << " [" << name_ << "]" << std::endl;
            if (forward_) {
                eckit::Log::userError() << msg << " [" << name_ << "]" << std::endl;
            }
            break;

        case 'N': /* notification */
            NOTIMP;
            break;

        case 'p': /* ping */
            s << 'p';
            break;

        case 's': /* statistics */
        {
            int n;
            s >> n;
            std::string key, value;
            for (int i = 0; i < n; i++)
            {
                s >> key >> value;
                eckit::Log::info() << "DHSProtocol:s " << key << "=" << value << std::endl;
            }
        }
        break;

        case 'S': /* notification start */
            NOTIMP;
            break;

        case 't': /* new timeout */
            NOTIMP;
            break;

        default:
            throw eckit::Exception(std::string("Unknown code [") + code + "]");
            break;
        }
    }
}

static ProtocolBuilder<DHSProtocol> builder("dhsbase");

}
}
