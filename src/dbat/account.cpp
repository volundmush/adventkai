#include "dbat/account.h"
#include "dbat/db.h"
#include <boost/algorithm/string.hpp>
#include "sodium.h"

std::map<vnum, account_data> accounts;

struct account_data *findAccount(const std::string &name) {
    for (auto &[aid, account] : accounts) {
        if (boost::iequals(account.name, name)) {
            return &account;
        }
    }
    return nullptr;
}

bool account_data::checkPassword(const std::string &password) {
    auto result = crypto_pwhash_str_verify(passHash.data(), password.data(), password.size());
    return result == 0;
}

bool account_data::setPassword(const std::string &password) {
    char hashed_password[crypto_pwhash_STRBYTES];
    if(password.empty()) return false;
    if(crypto_pwhash_str(hashed_password, password.data(), password.size(),
                         crypto_pwhash_OPSLIMIT_INTERACTIVE, crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
        return false;
    }
    passHash = hashed_password;
    dirty_accounts.insert(vn);
    return true;
}

nlohmann::json account_data::serialize() {
    nlohmann::json j;

    if(vn != NOTHING) j["vn"] = vn;
    if(!name.empty()) j["name"] = name;
    if(!passHash.empty()) j["passHash"] = passHash;
    if(!email.empty()) j["email"] = email;
    if(created) j["created"] = created;
    if(lastLogin) j["lastLogin"] = lastLogin;
    if(lastLogout) j["lastLogout"] = lastLogout;
    if(lastPasswordChanged) j["lastPasswordChanged"] = lastPasswordChanged;
    if(totalPlayTime != 0.0) j["totalPlayTime"] = totalPlayTime;
    if(!disabledReason.empty()) j["disabledReason"] = disabledReason;
    if(disabledUntil) j["disabledUntil"] = disabledUntil;
    if(rpp) j["rpp"] = rpp;
    if(slots != 3) j["slots"] = slots;
    if(adminLevel) j["adminLevel"] = adminLevel;
    for(auto c : characters) j["characters"].push_back(c);

    return j;
}

void account_data::deserialize(const nlohmann::json& j) {
    if(j.contains("vn")) vn = j["vn"];
    if(j.contains("name")) name = j["name"];
    if(j.contains("passHash")) passHash = j["passHash"];
    if(j.contains("email")) email = j["email"];
    if(j.contains("created")) created = j["created"];
    if(j.contains("lastLogin")) lastLogin = j["lastLogin"];
    if(j.contains("lastLogout")) lastLogout = j["lastLogout"];
    if(j.contains("lastPasswordChanged")) lastPasswordChanged = j["lastPasswordChanged"];
    if(j.contains("totalPlayTime")) totalPlayTime = j["totalPlayTime"];
    if(j.contains("disabledReason")) disabledReason = j["disabledReason"];
    if(j.contains("disabledUntil")) disabledUntil = j["disabledUntil"];
    if(j.contains("rpp")) rpp = j["rpp"];
    if(j.contains("slots")) slots = j["slots"];
    if(j.contains("adminLevel")) adminLevel = j["adminLevel"];
    if(j.contains("characters")) {
        for(auto c : j["characters"]) {
            characters.push_back(c);
        }
    }
}

account_data::account_data(const nlohmann::json &j) {
    deserialize(j);
}

int account_data::getNextID() {
    int id = 0;
    while(accounts.contains(id)) id++;
    return id;
}