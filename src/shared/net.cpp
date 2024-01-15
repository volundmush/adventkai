#include "shared/net.h"
#include <boost/algorithm/string.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>

std::shared_ptr<spdlog::logger> logger;

std::shared_ptr<spdlog::logger> setup_logging(const std::string &name, const std::string& path) {
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::info); // Set the console to output info level and above messages
    console_sink->set_pattern("[%^%l%$] %v"); // Example pattern: [INFO] some message

    auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(path, 1024 * 1024 * 5, 3);
    file_sink->set_level(spdlog::level::trace); // Set the file to output all levels of messages

    std::vector<spdlog::sink_ptr> sinks {console_sink, file_sink};

    auto out = std::make_shared<spdlog::logger>(name, begin(sinks), end(sinks));
    out->set_level(spdlog::level::trace); // Set the logger to trace level
    spdlog::register_logger(out);
    return out;
}

namespace net {
    void ProtocolCapabilities::deserialize(const nlohmann::json& j) {
        if(j.contains("protocol")) {
            auto s = j["protocol"].get<std::string>();
            if(boost::iequals(s, "Telnet")) protocol = Protocol::Telnet;
            else if(boost::iequals(s, "WebSocket")) protocol = Protocol::WebSocket;
        }
        if(j.contains("encryption")) encryption = j["encryption"];
        if(j.contains("client_name")) clientName = j["client_name"];
        if(j.contains("client_version")) clientVersion = j["client_version"];
		if(j.contains("host_address")) hostAddress = j["host_address"];
        if(j.contains("host_port")) hostPort = j["host_port"];
        if(j.contains("host_names")) for(auto &hn : j["host_names"]) {
            hostNames.emplace_back(hn.get<std::string>());
        }
        if(j.contains("encoding")) encoding = j["encoding"];
        if(j.contains("utf8")) utf8 = j["utf8"];

        if(j.contains("colorType")) colorType = j["colorType"].get<ColorType>();

        if(j.contains("width")) width = j["width"];
        if(j.contains("height")) height = j["height"];
        if(j.contains("gmcp")) gmcp = j["gmcp"];
        if(j.contains("msdp")) msdp = j["msdp"];
        if(j.contains("mssp")) mssp = j["mssp"];
        if(j.contains("mxp")) mxp = j["mxp"];
        if(j.contains("mccp2")) mccp2 = j["mccp2"];
        if(j.contains("mccp3")) mccp3 = j["mccp3"];
        if(j.contains("ttype")) ttype = j["ttype"];
        if(j.contains("naws")) naws = j["naws"];
        if(j.contains("sga")) sga = j["sga"];
        if(j.contains("linemode")) linemode = j["linemode"];
        if(j.contains("force_endline")) force_endline = j["force_endline"];
        if(j.contains("oob")) oob = j["oob"];
        if(j.contains("tls")) tls = j["tls"];
        if(j.contains("screen_reader")) screen_reader = j["screen_reader"];
        if(j.contains("mouse_tracking")) mouse_tracking = j["mouse_tracking"];
        if(j.contains("vt100")) vt100 = j["vt100"];
        if(j.contains("osc_color_palette")) osc_color_palette = j["osc_color_palette"];
        if(j.contains("proxy")) proxy = j["proxy"];
        if(j.contains("mnes")) mnes = j["mnes"];
    }

    nlohmann::json ProtocolCapabilities::serialize() {
        nlohmann::json j;

        if(encryption) j["encryption"] = encryption;
        j["client_name"] = clientName;
        j["client_version"] = clientVersion;
        j["host_address"] = hostAddress;
        if(hostPort) j["host_port"] = hostPort;
        for(auto &hn : hostNames) {
            j["host_names"].push_back(hn);
        }
        if(!encoding.empty()) j["encoding"] = encoding;
        if(utf8) j["utf8"] = utf8;
        if(colorType != ColorType::NoColor) j["colorType"] = colorType;
        if(width != 80) j["width"] = width;
        if(height != 52) j["height"] = height;
        if(gmcp) j["gmcp"] = gmcp;
        if(msdp) j["msdp"] = msdp;
        if(mssp) j["mssp"] = mssp;
        if(mxp) j["mxp"] = mxp;
        if(mccp2) j["mccp2"] = mccp2;
        if(mccp3) j["mccp3"] = mccp3;
        if(ttype) j["ttype"] = ttype;
        if(naws) j["naws"] = naws;
        if(sga) j["sga"] = sga;
        if(linemode) j["linemode"] = linemode;
        if(force_endline) j["force_endline"] = force_endline;
        if(oob) j["oob"] = oob;
        if(tls) j["tls"] = tls;
        if(screen_reader) j["screen_reader"] = screen_reader;
        if(mouse_tracking) j["mouse_tracking"] = mouse_tracking;
        if(vt100) j["vt100"] = vt100;
        if(osc_color_palette) j["osc_color_palette"] = osc_color_palette;
        if(proxy) j["proxy"] = proxy;
        if(mnes) j["mnes"] = mnes;

        return j;
    }

    std::string ProtocolCapabilities::protocolName() {
        switch(protocol) {
            case Protocol::Telnet: return "telnet";
            case Protocol::WebSocket: return "websocket";
            default:
                return "unknown";
        }
    }

    nlohmann::json GameMessage::serialize() const {
        nlohmann::json j;
        j["type"] = type;
        if(!data.empty()) j["data"] = data;

        return j;
    }

    GameMessage::GameMessage(const nlohmann::json& j) : GameMessage() {
        if(j.contains("type")) type = j["type"].get<GameMessageType>();
        if(j.contains("data")) data = j["data"];
    }

}