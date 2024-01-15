//
// Created by volund on 11/4/21.
//

#include "dbat/techniques.h"
#include "dbat/structs.h"
#include "dbat/utils.h"
#include "dbat/combat.h"
#include "dbat/comm.h"
#include "dbat/spells.h"

bool tech_handle_zanzoken(char_data *ch, char_data *vict, const std::string &name) {
    if (((!IS_NPC(vict) && IS_ICER(vict) && rand_number(1, 30) >= 28) || AFF_FLAGGED(vict, AFF_ZANZOKEN)) &&
        (vict->getCurST()) >= 1 && GET_POS(vict) != POS_SLEEPING) {
        if (!AFF_FLAGGED(ch, AFF_ZANZOKEN) || (AFF_FLAGGED(ch, AFF_ZANZOKEN) && GET_SPEEDI(ch) + rand_number(1, 5) <
                                                                                GET_SPEEDI(vict) + rand_number(1, 5))) {
            auto msg = fmt::format("@C$N@c disappears, avoiding your {} before reappearing!@n", name);
            act(msg.c_str(), true, ch, nullptr, vict, TO_CHAR);
            msg = fmt::format("@cYou disappear, avoiding @C$n's@c {} before reappearing!@n", name);
            act(msg.c_str(), true, ch, nullptr, vict, TO_VICT);
            msg = fmt::format("@C$N@c disappears, avoiding @C$n's@c {} before reappearing!@n", name);
            act(msg.c_str(), true, ch, nullptr, vict, TO_NOTVICT);
            for(auto c : {ch, vict}) c->affected_by.reset(AFF_ZANZOKEN);
            return false;
        } else {
            act("@C$N@c disappears, trying to avoid your attack but your zanzoken is faster!@n", false, ch, nullptr,
                vict, TO_CHAR);
            act("@cYou zanzoken to avoid the attack but @C$n's@c zanzoken is faster!@n", false, ch, nullptr, vict,
                TO_VICT);
            act("@C$N@c disappears, trying to avoid @C$n's@c attack but @C$n's@c zanzoken is faster!@n", false, ch,
                nullptr, vict, TO_NOTVICT);
            for(auto c : {ch, vict}) c->affected_by.reset(AFF_ZANZOKEN);
        }
    }
    return true;
}

void tech_handle_posmodifier(char_data *vict, int &pry, int &blk, int &dge, int &prob) {
    switch (GET_POS(vict)) {
        case POS_SLEEPING:
            pry = 0;
            blk = 0;
            dge = 0;
            prob += 50;
        case POS_RESTING:
            pry /= 4;
            blk /= 4;
            dge /= 4;
            prob += 25;
        case POS_SITTING:
            pry /= 2;
            blk /= 2;
            dge /= 2;
            prob += 10;
    }
}

bool tech_handle_charge(char_data *ch, char *arg, double minimum, double *attperc) {
    if (*arg) {
        double adjust = (double) (atoi(arg)) * 0.01;
        if (adjust < 0.01 || adjust > 1.00) {
            send_to_char(ch,
                         "If you are going to supply a percentage of your charge to use then use an acceptable number (1-100)\r\n");
            return false;
        } else if (adjust < *attperc && adjust >= minimum) {
            *attperc = adjust;
        } else if (adjust < minimum) {
            *attperc = minimum;
        }
    }
    return true;
}

bool tech_handle_targeting(char_data *ch, char *arg, char_data **vict, obj_data **obj) {
    *vict = nullptr;
    *obj = nullptr;
    if (!*arg || !(*vict = get_char_vis(ch, arg, nullptr, FIND_CHAR_ROOM))) {
        if (FIGHTING(ch) && IN_ROOM(FIGHTING(ch)) == IN_ROOM(ch)) {
            *vict = FIGHTING(ch);
            return true;
        } else if (!(*obj = get_obj_in_list_vis(ch, arg, nullptr, ch->getRoom()->contents))) {
            send_to_char(ch, "Nothing around here by that name.\r\n");
            return false;
        }
        return true;
    }
    return true;
}


void tech_handle_fireshield(char_data *ch, char_data *vict, const std::string &part) {
    if (GET_HIT(vict) > 0 && !AFF_FLAGGED(vict, AFF_SPIRIT) && AFF_FLAGGED(vict, AFF_FIRESHIELD) &&
        !GET_BONUS(ch, BONUS_FIREPROOF) && !IS_DEMON(ch)) {
        auto msg = fmt::format("@c$N's@W fireshield burns your {}!@n", part);
        act(msg.c_str(), true, ch, nullptr, vict, TO_CHAR);
        msg = fmt::format("@C$n's@W {} is burned by your fireshield!@n", part);
        act(msg.c_str(), true, ch, nullptr, vict, TO_VICT);
        msg = fmt::format("@c$n's@W {} is burned by @C$N's@W fireshield!@n", part);
        act(msg.c_str(), true, ch, nullptr, vict, TO_NOTVICT);
        int64_t dmg = GET_MAX_MANA(vict) * 0.02;
        LASTATK(vict) += 1000;
        hurt(0, 0, vict, ch, nullptr, dmg, 0);
        if (GET_BONUS(ch, BONUS_FIREPRONE)) {
            send_to_char(ch, "@RYou are extremely flammable and are burned by the attack!@n\r\n");
            send_to_char(vict, "@RThey are easily burned!@n\r\n");
            ch->affected_by.set(AFF_BURNED);
        } else if (GET_CON(ch) < axion_dice(0)) {
            send_to_char(ch, "@RYou are badly burned!@n\r\n");
            send_to_char(vict, "@RThey are burned!@n\r\n");
            ch->affected_by.set(AFF_BURNED);
        }
    } else if (GET_HIT(vict) > 0 && !AFF_FLAGGED(vict, AFF_SPIRIT) && AFF_FLAGGED(vict, AFF_FIRESHIELD) &&
               (GET_BONUS(ch, BONUS_FIREPROOF) || IS_DEMON(ch))) {
        send_to_char(vict, "@RThey appear to be fireproof!@n\r\n");
    }
}

bool tech_handle_android_absorb(char_data *ch, char_data *vict) {
    if (IS_ANDROID(vict) && HAS_ARMS(vict) && GET_SKILL(vict, SKILL_ABSORB) > rand_number(1, 140)) {
        act("@C$N@W absorbs your ki attack and all your charged ki with $S hand!@n", true, ch, nullptr, vict, TO_CHAR);
        act("@WYou absorb @C$n's@W ki attack and all $s charged ki with your hand!@n", true, ch, nullptr, vict,
            TO_VICT);
        act("@C$N@W absorbs @c$n's@W ki attack and all $s charged ki with $S hand!@n", true, ch, nullptr, vict,
            TO_NOTVICT);
        int amot = GET_CHARGE(ch);
        if (IS_NPC(ch)) {
            amot = GET_MAX_MANA(ch) / 20;
        }
        if (GET_CHARGE(vict) + amot > GET_MAX_MANA(vict)) {
            vict->incCurKI(vict->getMaxKI() - GET_CHARGE(vict));
            GET_CHARGE(vict) = GET_MAX_MANA(vict);
        } else {
            GET_CHARGE(vict) += amot;
        }
        return true;
    }
    return false;
}

void tech_handle_crashdown(char_data *ch, char_data *vict) {
    if (AFF_FLAGGED(vict, AFF_FLYING)) {
        act("@w$N@w is knocked out of the air!@n", true, ch, nullptr, vict, TO_CHAR);
        act("@wYou are knocked out of the air!@n", true, ch, nullptr, vict, TO_VICT);
        act("@w$N@w is knocked out of the air!@n", true, ch, nullptr, vict, TO_NOTVICT);
        vict->affected_by.reset(AFF_FLYING);
        GET_ALT(vict) = 0;
        GET_POS(vict) = POS_SITTING;
    } else {
        handle_knockdown(vict);
    }
}