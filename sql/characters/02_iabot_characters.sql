-- ============================================================================
-- mod-iaplayerbot — Phase 1b: Characters database setup
-- Run this ONCE against your `acore_characters` database
-- AFTER running 01_iabot_account.sql on the auth database
-- ============================================================================
--
-- This creates 18 bot characters (9 classes × 2 factions) in the bot account.
-- Each character is level 80 with a unique name.
--
-- The characters are created with minimal data. AzerothCore's Player::LoadFromDB
-- will handle the rest (spells, talents, starting gear, etc.) when they login.
--
-- GUID range: 90001-90018 (high range to avoid conflicts)
-- Account: 90000 (must match IABot.Account in conf)
--
-- WotLK 3.3.5a class/race combinations:
--   Warrior(1):  Human(1), Orc(2)
--   Paladin(2):  Human(1), BloodElf(10)
--   Hunter(3):   NightElf(4), Troll(8)
--   Rogue(4):    Human(1), Undead(5)
--   Priest(5):   Human(1), Undead(5)
--   Shaman(7):   Draenei(11), Orc(2)
--   Mage(8):     Human(1), Undead(5)
--   Warlock(9):  Human(1), Undead(5)
--   DK(6):       Human(1), Orc(2)
-- ============================================================================

SET @ACCOUNT := 90000;
SET @GUID_START := 90001;

-- Verify the account exists
SELECT COUNT(*) INTO @acc_exists FROM `acore_auth`.`account` WHERE `id` = @ACCOUNT;
-- Note: if this fails, run 01_iabot_account.sql on auth DB first

-- ============================================================================
-- Delete any existing bot characters in our GUID range (safe re-run)
-- ============================================================================
DELETE FROM `characters` WHERE `guid` BETWEEN @GUID_START AND @GUID_START + 17;
DELETE FROM `character_aura` WHERE `guid` BETWEEN @GUID_START AND @GUID_START + 17;
DELETE FROM `character_spell` WHERE `guid` BETWEEN @GUID_START AND @GUID_START + 17;
DELETE FROM `character_queststatus` WHERE `guid` BETWEEN @GUID_START AND @GUID_START + 17;
DELETE FROM `character_inventory` WHERE `guid` BETWEEN @GUID_START AND @GUID_START + 17;
DELETE FROM `character_action` WHERE `guid` BETWEEN @GUID_START AND @GUID_START + 17;
DELETE FROM `character_homebind` WHERE `guid` BETWEEN @GUID_START AND @GUID_START + 17;
DELETE FROM `character_skills` WHERE `guid` BETWEEN @GUID_START AND @GUID_START + 17;

-- ============================================================================
-- Create characters
-- Using INSERT with all required columns for a valid WotLK character row
-- position_x/y/z = Stormwind or Orgrimmar (will be overridden on login by teleport)
-- ============================================================================

-- The `characters` table requires these core columns at minimum:
-- guid, account, name, race, class, gender, level, money, playerBytes, playerBytes2,
-- playerFlags, map, position_x, position_y, position_z, orientation, at_login

-- Alliance bots (race-appropriate choices)
INSERT INTO `characters` (`guid`, `account`, `name`, `race`, `class`, `gender`, `level`,
    `money`, `playerBytes`, `playerBytes2`, `playerFlags`,
    `map`, `zone`, `position_x`, `position_y`, `position_z`, `orientation`,
    `at_login`, `online`, `cinematic`, `totaltime`, `leveltime`,
    `extra_flags`, `stable_slots`, `bank_slots`,
    `equipmentCache`, `knownTitles`) VALUES
-- Alliance
(@GUID_START + 0,  @ACCOUNT, 'IABotWarriorA',  1, 1, 0, 80, 1000000, 0, 0, 0, 0, 1519, -8833.38, 628.628, 94.0066, 0, 0, 0, 1, 0, 0, 0, 0, 0, '', '0 0 0 0 0 0'),
(@GUID_START + 1,  @ACCOUNT, 'IABotPaladinA',  1, 2, 0, 80, 1000000, 0, 0, 0, 0, 1519, -8833.38, 628.628, 94.0066, 0, 0, 0, 1, 0, 0, 0, 0, 0, '', '0 0 0 0 0 0'),
(@GUID_START + 2,  @ACCOUNT, 'IABotHunterA',   4, 3, 0, 80, 1000000, 0, 0, 0, 1, 141,  10311.3, 832.463, 1326.41, 0, 0, 0, 1, 0, 0, 0, 0, 0, '', '0 0 0 0 0 0'),
(@GUID_START + 3,  @ACCOUNT, 'IABotRogueA',    1, 4, 0, 80, 1000000, 0, 0, 0, 0, 1519, -8833.38, 628.628, 94.0066, 0, 0, 0, 1, 0, 0, 0, 0, 0, '', '0 0 0 0 0 0'),
(@GUID_START + 4,  @ACCOUNT, 'IABotPriestA',   1, 5, 0, 80, 1000000, 0, 0, 0, 0, 1519, -8833.38, 628.628, 94.0066, 0, 0, 0, 1, 0, 0, 0, 0, 0, '', '0 0 0 0 0 0'),
(@GUID_START + 5,  @ACCOUNT, 'IABotDKA',       1, 6, 0, 80, 1000000, 0, 0, 0, 0, 1519, -8833.38, 628.628, 94.0066, 0, 0, 0, 1, 0, 0, 0, 0, 0, '', '0 0 0 0 0 0'),
(@GUID_START + 6,  @ACCOUNT, 'IABotShamanA',  11, 7, 0, 80, 1000000, 0, 0, 0, 530, 3524, -3961.64, -13931.2, 100.615, 0, 0, 0, 1, 0, 0, 0, 0, 0, '', '0 0 0 0 0 0'),
(@GUID_START + 7,  @ACCOUNT, 'IABotMageA',     1, 8, 0, 80, 1000000, 0, 0, 0, 0, 1519, -8833.38, 628.628, 94.0066, 0, 0, 0, 1, 0, 0, 0, 0, 0, '', '0 0 0 0 0 0'),
(@GUID_START + 8,  @ACCOUNT, 'IABotWarlockA',  1, 9, 0, 80, 1000000, 0, 0, 0, 0, 1519, -8833.38, 628.628, 94.0066, 0, 0, 0, 1, 0, 0, 0, 0, 0, '', '0 0 0 0 0 0'),
-- Horde
(@GUID_START + 9,  @ACCOUNT, 'IABotWarriorH',  2, 1, 0, 80, 1000000, 0, 0, 0, 1, 1637, 1629.36, -4373.39, 31.2564, 0, 0, 0, 1, 0, 0, 0, 0, 0, '', '0 0 0 0 0 0'),
(@GUID_START + 10, @ACCOUNT, 'IABotPaladinH', 10, 2, 0, 80, 1000000, 0, 0, 0, 530, 3487, 9487.69, -7340.21, 14.2116, 0, 0, 0, 1, 0, 0, 0, 0, 0, '', '0 0 0 0 0 0'),
(@GUID_START + 11, @ACCOUNT, 'IABotHunterH',   8, 3, 0, 80, 1000000, 0, 0, 0, 1, 1637, 1629.36, -4373.39, 31.2564, 0, 0, 0, 1, 0, 0, 0, 0, 0, '', '0 0 0 0 0 0'),
(@GUID_START + 12, @ACCOUNT, 'IABotRogueH',    5, 4, 0, 80, 1000000, 0, 0, 0, 0, 1497, 1633.75, 240.167, -43.1034, 0, 0, 0, 1, 0, 0, 0, 0, 0, '', '0 0 0 0 0 0'),
(@GUID_START + 13, @ACCOUNT, 'IABotPriestH',   5, 5, 0, 80, 1000000, 0, 0, 0, 0, 1497, 1633.75, 240.167, -43.1034, 0, 0, 0, 1, 0, 0, 0, 0, 0, '', '0 0 0 0 0 0'),
(@GUID_START + 14, @ACCOUNT, 'IABotDKH',       2, 6, 0, 80, 1000000, 0, 0, 0, 1, 1637, 1629.36, -4373.39, 31.2564, 0, 0, 0, 1, 0, 0, 0, 0, 0, '', '0 0 0 0 0 0'),
(@GUID_START + 15, @ACCOUNT, 'IABotShamanH',   2, 7, 0, 80, 1000000, 0, 0, 0, 1, 1637, 1629.36, -4373.39, 31.2564, 0, 0, 0, 1, 0, 0, 0, 0, 0, '', '0 0 0 0 0 0'),
(@GUID_START + 16, @ACCOUNT, 'IABotMageH',     5, 8, 0, 80, 1000000, 0, 0, 0, 0, 1497, 1633.75, 240.167, -43.1034, 0, 0, 0, 1, 0, 0, 0, 0, 0, '', '0 0 0 0 0 0'),
(@GUID_START + 17, @ACCOUNT, 'IABotWarlockH',  5, 9, 0, 80, 1000000, 0, 0, 0, 0, 1497, 1633.75, 240.167, -43.1034, 0, 0, 0, 1, 0, 0, 0, 0, 0, '', '0 0 0 0 0 0');

-- ============================================================================
-- Create homebind entries (required for Player::LoadFromDB)
-- Alliance → Stormwind Inn, Horde → Orgrimmar Inn
-- ============================================================================
INSERT INTO `character_homebind` (`guid`, `mapId`, `zoneId`, `posX`, `posY`, `posZ`, `posO`) VALUES
(@GUID_START + 0,  0, 1519, -8867.68, 673.373, 97.9034, 0),
(@GUID_START + 1,  0, 1519, -8867.68, 673.373, 97.9034, 0),
(@GUID_START + 2,  0, 1519, -8867.68, 673.373, 97.9034, 0),
(@GUID_START + 3,  0, 1519, -8867.68, 673.373, 97.9034, 0),
(@GUID_START + 4,  0, 1519, -8867.68, 673.373, 97.9034, 0),
(@GUID_START + 5,  0, 1519, -8867.68, 673.373, 97.9034, 0),
(@GUID_START + 6,  0, 1519, -8867.68, 673.373, 97.9034, 0),
(@GUID_START + 7,  0, 1519, -8867.68, 673.373, 97.9034, 0),
(@GUID_START + 8,  0, 1519, -8867.68, 673.373, 97.9034, 0),
(@GUID_START + 9,  1, 1637, 1633.33, -4439.11, 15.7588, 0),
(@GUID_START + 10, 1, 1637, 1633.33, -4439.11, 15.7588, 0),
(@GUID_START + 11, 1, 1637, 1633.33, -4439.11, 15.7588, 0),
(@GUID_START + 12, 1, 1637, 1633.33, -4439.11, 15.7588, 0),
(@GUID_START + 13, 1, 1637, 1633.33, -4439.11, 15.7588, 0),
(@GUID_START + 14, 1, 1637, 1633.33, -4439.11, 15.7588, 0),
(@GUID_START + 15, 1, 1637, 1633.33, -4439.11, 15.7588, 0),
(@GUID_START + 16, 1, 1637, 1633.33, -4439.11, 15.7588, 0),
(@GUID_START + 17, 1, 1637, 1633.33, -4439.11, 15.7588, 0);

-- ============================================================================
-- Verify creation
-- ============================================================================
SELECT guid, name, race, class, level
FROM `characters`
WHERE `account` = @ACCOUNT
ORDER BY `guid`;
