#pragma once
#include "dbat/structs.h"
#include "spells.h"

namespace atk {
    // If there is no entry for a skill, then the attack doesn't require a skill.
    extern std::map<int, std::vector<std::pair<int, int>>> minimumSkillRequired;

    extern std::map<int, std::vector<std::pair<int, int>>> trainSkillOnSuccess;
    extern std::map<int, std::vector<std::pair<int, int>>> trainSkillOnFailure;

    extern std::vector<int> comboSkills;

    enum class Result {
        Landed,
        Missed,
        Canceled
    };

    enum class DefenseResult {
        Blocked,
        Dodged,
        Parried,
        Failed
    };

    struct Attack {
        Attack(struct char_data *ch, char *arg);
        virtual int getSkillID() = 0;
        virtual int getAtkID() = 0; // used for damtype and roll_hitloc
        virtual std::string getName() = 0;
        virtual std::string getBodyPart() { return "body"; }
        virtual void execute();
        virtual void initStats();
        virtual int autoTrainSkillID() { return 0;};

        virtual Result doAttack();
        virtual Result attackCharacter();
        virtual Result attackObject();

        virtual void onLanded();
        virtual void onMissed();
        virtual void onCanceled();
        virtual void onLandedOrMissed();

        virtual std::optional<int> limbsToCheck();
        virtual std::optional<int> hurtInjuredLimbs();
        virtual void hurtLimb(int limb);
        virtual bool checkLimbs();
        virtual bool checkSkills();
        virtual bool needEmptyHands() {return false;};
        virtual bool checkEmptyHands();
        virtual bool checkCosts();
        virtual bool isPhysical();
        virtual bool usesWeapon();
        virtual bool isKiAttack();

        void actUser(const std::string& msg, bool hideInvisible = true);
        void actOthers(const std::string& msg, bool hideInvisible = true);
        void actRoom(const std::string& msg, bool hideInvisible = true);
        void actVictim(const std::string& msg, bool hideInvisible = true);

        virtual int64_t calculateHealthCost();
        virtual int64_t calculateStaminaCost();
        virtual int64_t calculateKiCost();
        virtual std::optional<int> hasCooldown();
        virtual int canKillType();
        virtual int limbhurtChance() {return 0;};

        virtual void doTrain(const std::vector<std::pair<int, int>>& skills);

        virtual bool canBlock();
        virtual bool canParry();
        virtual bool canDodge();
        virtual bool canZanzoken();
        virtual bool canCombo();

        virtual Result handleOtherHit();
        virtual Result handleCleanHit();
        virtual void handleHitspot();
        virtual void announceHitspot();

        virtual bool calculateDeflect();
        virtual Result handleMiss();
        virtual Result handleParry() = 0;
        virtual Result handleBlock() = 0;
        virtual Result handleDodge() = 0;

        virtual int64_t calculateObjectDamage();

        virtual void announceParry() = 0;
        virtual void announceBlock() = 0;
        virtual void announceDodge() = 0;
        virtual void announceMiss() = 0;
        virtual void announceObject() = 0;

        struct char_data *user{};
        struct char_data *victim{};
        struct obj_data *obj{};
        std::string input;
        std::vector<std::string> args;

        int currentSpeedIndexCheck{0};
        int currentBlockCheck{0};
        int currentDodgeCheck{0};
        int currentParryCheck{0};
        int currentHitProbability{0};
        int currentChanceToHit{0};
        int initSkill{0};
        int hitspot{1};
        int cooldownOverride{-1};
        int64_t currentHealthCost{0};
        int64_t currentStaminaCost{0};
        int64_t currentKiCost{0};
        double attPerc{0.0};
        int64_t calcDamage{0};
        DefenseResult defenseResult{DefenseResult::Failed};

    };

    struct MeleeAttack : Attack {
        using Attack::Attack;
        int64_t calculateObjectDamage() override;
        int64_t calculateStaminaCost() override;
        bool canBlock() override {return true;};
        bool canParry() override {return true;};
        bool canDodge() override {return true;};
        bool isPhysical() override {return true;};
        bool canCombo() override {return true;};
        Result handleParry() override;
        Result handleDodge() override;
        Result handleBlock() override;
        void handleHitspot() override;
    };

    struct RangedAttack : Attack {
        using Attack::Attack;
        int canKillType() override;
    };

    struct RangedKiAttack : RangedAttack {
        using RangedAttack::RangedAttack;
        bool isKiAttack() override {return true;};
        bool isPhysical() override {return false;};
    };

    // atkID -1 is used for parry.
    struct HandAttack : MeleeAttack {
        using MeleeAttack::MeleeAttack;
        bool needEmptyHands() override {return true;};
        std::optional<int> limbsToCheck() override {return 0;};
        std::optional<int> hurtInjuredLimbs() override {return 0;};
    };

    struct LegAttack : MeleeAttack {
        using MeleeAttack::MeleeAttack;
        std::optional<int> limbsToCheck() override {return 1;};
        std::optional<int> hurtInjuredLimbs() override {return 1;};
    };

    struct Punch : HandAttack {
        using HandAttack::HandAttack;
        int getSkillID() override { return SKILL_PUNCH; };
        int getAtkID() override {return 0;};
        std::string getName() override { return "punch"; };
        std::string getBodyPart() override {return "hand";};
        std::optional<int> hasCooldown() override {return 4;};
        void announceParry() override;
        void announceBlock() override;
        void announceDodge() override;
        void announceMiss() override;
        void announceObject() override;
        void announceHitspot() override;
    };

    struct Kick : LegAttack {
        using LegAttack::LegAttack;
        int getSkillID() override { return SKILL_KICK; }
        int getAtkID() override {return 1;};
        std::string getName() override { return "kick"; };
        std::string getBodyPart() override {return "foot";};
        std::optional<int> hasCooldown() override {return 5;};

        void announceParry() override;
        void announceBlock() override;
        void announceDodge() override;
        void announceMiss() override;
        void announceObject() override;
        void announceHitspot() override;
    };

    struct Elbow : HandAttack {
        using HandAttack::HandAttack;
        int getSkillID() override { return SKILL_ELBOW; }
        int getAtkID() override {return 2;};
        std::string getName() override { return "elbow"; }
        std::string getBodyPart() override {return "elbow";};
        std::optional<int> hasCooldown() override {return 6;};

        void announceParry() override;
        void announceBlock() override;
        void announceDodge() override;
        void announceMiss() override;
        void announceObject() override;
        void announceHitspot() override;
    };

    struct Knee : LegAttack {
        using LegAttack::LegAttack;
        int getSkillID() override { return SKILL_KNEE; }
        int getAtkID() override {return 3;};
        std::string getName() override { return "knee"; }
        std::string getBodyPart() override {return "knee";};
        std::optional<int> hasCooldown() override {return 6;};

        void announceParry() override;
        void announceBlock() override;
        void announceDodge() override;
        void announceMiss() override;
        void announceObject() override;
        void announceHitspot() override;
    };

    struct Roundhouse : LegAttack {
        using LegAttack::LegAttack;
        int getSkillID() override { return SKILL_ROUNDHOUSE; }
        int getAtkID() override {return 4;};
        std::string getName() override { return "roundhouse"; }
        std::string getBodyPart() override {return "foot";};
        std::optional<int> hasCooldown() override;

        int limbhurtChance() override;
        void handleHitspot() override;
        void announceParry() override;
        void announceBlock() override;
        void announceDodge() override;
        void announceMiss() override;
        void announceObject() override;
        void announceHitspot() override;
    };

    struct Uppercut : HandAttack {
        using HandAttack::HandAttack;
        int getSkillID() override { return SKILL_UPPERCUT; }
        int getAtkID() override {return 5;};
        std::string getName() override { return "uppercut"; }
        std::string getBodyPart() override {return "hand";};
    };

    struct Slam : MeleeAttack {
        int getSkillID() override { return SKILL_SLAM; }
        int getAtkID() override {return 6;};
        std::string getName() override { return "slam"; }
    };

    struct Heeldrop : MeleeAttack {
        int getSkillID() override { return SKILL_HEELDROP; }
        int getAtkID() override {return 8;};
        std::string getName() override { return "heeldrop"; }
    };

    struct Tsuihidan : RangedKiAttack {
        int getSkillID() override { return SKILL_TSUIHIDAN; }
        int getAtkID() override {return 11;};
        std::string getName() override { return "tsuihidan"; }
    };

    struct Renzo : RangedKiAttack {
        int getSkillID() override { return SKILL_RENZO; }
        int getAtkID() override {return 12;};
        std::string getName() override { return "renzo"; }
    };

    struct Kamehameha : RangedKiAttack {
        int getSkillID() override { return SKILL_KAMEHAMEHA; }
        int getAtkID() override {return 13;};
        std::string getName() override { return "kamehameha"; }
    };

    struct Masenko : RangedKiAttack {
        int getSkillID() override { return SKILL_MASENKO; }
        int getAtkID() override {return 14;};
        std::string getName() override { return "masenko"; }
    };

    struct Dodonpa : RangedKiAttack {
        int getSkillID() override { return SKILL_DODONPA; }
        int getAtkID() override {return 15;};
        std::string getName() override { return "dodonpa"; }
    };

    struct GalikGun : RangedKiAttack {
        int getSkillID() override { return SKILL_GALIKGUN; }
        int getAtkID() override {return 16;};
        std::string getName() override { return "galik gun"; }
    };

    struct DeathBeam : RangedKiAttack {
        int getSkillID() override { return SKILL_DEATHBEAM; }
        int getAtkID() override {return 17;};
        std::string getName() override { return "death beam"; }
    };

    struct EraserCannon : RangedKiAttack {
        int getSkillID() override { return SKILL_ERASER; }
        int getAtkID() override {return 18;};
        std::string getName() override { return "eraser cannon"; }
    };

    struct TwinSlash : RangedKiAttack {
        int getSkillID() override { return SKILL_TSLASH; }
        int getAtkID() override {return 19;};
        std::string getName() override { return "twin slash"; }
    };

    struct PsychicBlast : RangedKiAttack {
        int getSkillID() override { return SKILL_PSYBLAST; }
        int getAtkID() override {return 20;};
        std::string getName() override { return "psychic blast"; }
    };

    struct Honoo : RangedKiAttack {
        int getSkillID() override { return SKILL_HONOO; }
        int getAtkID() override {return 21;};
        std::string getName() override { return "honoo"; }
    };

    struct DualBeam : RangedKiAttack {
        int getSkillID() override { return SKILL_DUALBEAM; }
        int getAtkID() override {return 22;};
        std::string getName() override { return "dual beam"; }
    };

    struct Rogafufuken : MeleeAttack {
        int getSkillID() override { return SKILL_ROGAFUFUKEN; }
        int getAtkID() override {return 23;};
        std::string getName() override { return "rogafufuken"; }
    };

    struct Bakuhatsuha : MeleeAttack {
        int getSkillID() override { return SKILL_BAKUHATSUHA; }
        int getAtkID() override {return 24;};
        std::string getName() override { return "bakuhatsuha"; }
    };

    struct Kienzan : RangedKiAttack {
        int getSkillID() override { return SKILL_KIENZAN; }
        int getAtkID() override {return 25;};
        std::string getName() override { return "kienzan"; }
    };

    struct Tribeam : RangedKiAttack {
        int getSkillID() override { return SKILL_TRIBEAM; }
        int getAtkID() override {return 26;};
        std::string getName() override { return "tribeam"; }
    };

    struct SpecialBeamCannon : RangedKiAttack {
        int getSkillID() override { return SKILL_SBC; }
        int getAtkID() override {return 27;};
        std::string getName() override { return "special beam cannon"; }
    };

    struct Headbutt : MeleeAttack {
        int getSkillID() override { return SKILL_HEADBUTT; }
        int getAtkID() override {return 52;};
        std::string getName() override { return "headbutt"; }
    };

    struct Tailwhip : MeleeAttack {
        int getSkillID() override { return SKILL_TAILWHIP; }
        int getAtkID() override {return 56;};
        std::string getName() override { return "tailwhip"; }
    };

}
