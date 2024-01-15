#pragma once
#include "structs.h"

namespace eff {

    // EffectType forms the first element of a two-part primary key.
    enum class EffectType : uint8_t {
        Ability = 0, // An ability/skill activated upon yourself, or innate.
        Transformation = 1, // A special transformation. Super Saiyan, etc.
        Blessing = 2, // Something that is cast upon you which is beneficial. Can be dispelled. Example: Might, Haste.
        Curse = 3, // Something that is cast upon you which is detrimental. Can be dispelled. Example: Blindness, Poison.
        Penalty = 4, // A penalty that cannot be dispelled. Example: Death Penalty, Flaws, etc.
        Bonus = 5, // A bonus that cannot be dispelled. Example: Chargen Traits, Feats, etc.
        Sensei = 6, // Traits stemming from your Sensei.
        Racial = 7, // Traits stemming from your race/species.
        Faction = 8, // Traits stemming from your faction.
        Companion = 9, // Traits that companions provide to their master.
        Environment = 10, // Traits inflicted by the environment.
        Gear = 11, // Effects provided by gear.
        Consumable = 12, // Effects provided by consumables.
    };

    struct effect_component {
        effect_component() = default;
        explicit effect_component(const nlohmann::json& j);
        double modifier{};         /* This is added to apropriate ability     */
        int location{};         /* Tells which ability to change(APPLY_XXX)*/
        int specific{};         /* Some locations have parameters          */
        nlohmann::json serialize();
    };

    struct effect {
        effect() = default;
        explicit effect(const nlohmann::json& j);
        std::optional<int> duration; // How long the effect lasts. (in game-minutes? hours?) Empty means indefinite.
        std::vector<effect_component> components;
        std::optional<std::string> source;
        nlohmann::json serialize();
    };

    // basic display info for an effect
    struct effect_info {
        std::string name;
        std::string description;
    };

    extern std::unordered_map<EffectType, std::unordered_map<effect_t, effect_info>> effectInfo;

    // information specific to transformations. Only one of each TransType can be active at once.
    enum class TransType : uint8_t {
        // Physical transformations. Example: Super Saiyan 1-4, God, Blue, Great Ape,
        // Zarbon's ugly mode, Frieza's forms, giant Piccolo.
        Physical = 0,

        // A heightened state of being. Example: Ultra Instinct, Ultra Ego, Ultimate/Beast Gohan, Orange Piccolo.
        // These have minimal physical changes and can often be stacked with other transformations.
        Enlightenment = 1,

        // This is a special transformation that's more of a technique, status effect, or modifier than a transformation.
        // Example: Kaioken, Villainous Mode/Dark Metamorphosis, Berserker/'Legendary' Super Saiyan, etc.
        Modifier = 2,
    };

    struct effect_trans {
        TransType type; // the slot this transformation occupies.
    };


}