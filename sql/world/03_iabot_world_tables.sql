-- ============================================================================
-- mod-iaplayerbot — Phase 1b: World database tables
-- Run this against your `acore_world` database
-- ============================================================================

-- ============================================================================
-- Module configuration table (runtime overrides for .conf values)
-- Allows hot-reloading config without server restart via .iabot reload
-- ============================================================================
DROP TABLE IF EXISTS `iabot_config`;
CREATE TABLE `iabot_config` (
    `key` VARCHAR(64) NOT NULL,
    `value` VARCHAR(255) NOT NULL DEFAULT '',
    `description` VARCHAR(255) NOT NULL DEFAULT '',
    PRIMARY KEY (`key`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='mod-iaplayerbot runtime config';

INSERT INTO `iabot_config` (`key`, `value`, `description`) VALUES
('MaxBots',              '100',  'Maximum total bots on the server'),
('MaxBotsPerPlayer',     '3',    'Maximum bots per real player'),
('SchedulerCpuBudgetUs', '5000', 'CPU budget per tick in microseconds'),
('FollowDistance',       '15',   'Default follow distance in yards');

-- ============================================================================
-- Boss strategy table (used by DungeonAI in Phase 9)
-- Pre-populated with a few iconic WotLK bosses
-- ============================================================================
DROP TABLE IF EXISTS `iabot_boss_strategies`;
CREATE TABLE `iabot_boss_strategies` (
    `entry` INT UNSIGNED NOT NULL COMMENT 'creature_template.entry of the boss',
    `name` VARCHAR(64) NOT NULL DEFAULT '',
    `requires_tank_swap` TINYINT(1) NOT NULL DEFAULT 0,
    `has_aoe_to_avoid` TINYINT(1) NOT NULL DEFAULT 0,
    `has_interruptable_spells` TINYINT(1) NOT NULL DEFAULT 0,
    `has_add_phase` TINYINT(1) NOT NULL DEFAULT 0,
    `interrupt_spells` VARCHAR(255) NOT NULL DEFAULT '' COMMENT 'comma-separated spell IDs to interrupt',
    `avoid_spells` VARCHAR(255) NOT NULL DEFAULT '' COMMENT 'comma-separated spell IDs with AoE to avoid',
    `melee_range` FLOAT NOT NULL DEFAULT 5.0,
    `ranged_range` FLOAT NOT NULL DEFAULT 30.0,
    `notes` TEXT,
    PRIMARY KEY (`entry`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='mod-iaplayerbot boss strategies for DungeonAI';

-- Sample entries (WotLK heroic dungeons)
INSERT INTO `iabot_boss_strategies` (`entry`, `name`, `requires_tank_swap`, `has_aoe_to_avoid`,
    `has_interruptable_spells`, `has_add_phase`, `interrupt_spells`, `avoid_spells`,
    `melee_range`, `ranged_range`, `notes`) VALUES
-- Utgarde Keep
(23953, 'Prince Keleseth',     0, 0, 1, 1, '48400',     '',      5, 30, 'Interrupt Shadow Bolt, kill skeletons'),
(23954, 'Skarvald & Dalronn',  0, 0, 1, 0, '43005',     '',      5, 30, 'Kill Skarvald first, interrupt Shadow Bolt'),
(23955, 'Ingvar the Plunderer',0, 1, 0, 0, '',           '42729', 5, 30, 'Avoid Dark Smash, two phases'),
-- The Nexus
(26731, 'Grand Magus Telestra',0, 1, 1, 1, '47765',     '',      5, 30, 'Split phase at 50%, interrupt Fireball'),
(26763, 'Anomalus',            0, 1, 0, 1, '',           '47743', 5, 30, 'Kill Chaotic Rifts, avoid Spark'),
(26723, 'Keristrasza',         0, 1, 0, 0, '',           '48094', 5, 30, 'Keep moving to avoid Intense Cold');

-- ============================================================================
-- Gear template table (used by BotScalingSystem in Phase 10)
-- Maps class+spec+level_range to recommended item IDs
-- ============================================================================
DROP TABLE IF EXISTS `iabot_gear_templates`;
CREATE TABLE `iabot_gear_templates` (
    `id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
    `class` TINYINT UNSIGNED NOT NULL COMMENT 'Player class (1-9)',
    `spec` TINYINT UNSIGNED NOT NULL DEFAULT 0 COMMENT '0=any, 1/2/3=specific spec',
    `role` ENUM('tank','heal','dps') NOT NULL DEFAULT 'dps',
    `min_level` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    `max_level` TINYINT UNSIGNED NOT NULL DEFAULT 80,
    `slot` TINYINT UNSIGNED NOT NULL COMMENT 'Equipment slot (0=head, 1=neck, etc.)',
    `item_ids` VARCHAR(255) NOT NULL COMMENT 'comma-separated item IDs, random pick',
    PRIMARY KEY (`id`),
    INDEX `idx_class_level` (`class`, `min_level`, `max_level`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='mod-iaplayerbot gear templates per class/spec/level';

-- Example: Warrior Tank level 80 basic gear (Tempered Saronite set)
-- Slot reference: 0=Head, 1=Neck, 2=Shoulders, 3=Shirt, 4=Chest, 5=Waist,
-- 6=Legs, 7=Feet, 8=Wrists, 9=Hands, 14=Back, 15=MainHand, 16=OffHand, 17=Ranged
INSERT INTO `iabot_gear_templates` (`class`, `spec`, `role`, `min_level`, `max_level`, `slot`, `item_ids`) VALUES
(1, 0, 'tank', 78, 80, 0,  '41387'),          -- Tempered Saronite Helm
(1, 0, 'tank', 78, 80, 2,  '41389'),          -- Tempered Saronite Shoulders
(1, 0, 'tank', 78, 80, 4,  '41383'),          -- Tempered Saronite Breastplate
(1, 0, 'tank', 78, 80, 6,  '41386'),          -- Tempered Saronite Legplates
(1, 0, 'tank', 78, 80, 7,  '41391'),          -- Tempered Saronite Boots
(1, 0, 'tank', 78, 80, 9,  '41380'),          -- Tempered Saronite Gauntlets
(1, 0, 'tank', 78, 80, 8,  '41388'),          -- Tempered Saronite Bracers
(1, 0, 'tank', 78, 80, 5,  '41390'),          -- Tempered Saronite Belt
(1, 0, 'tank', 78, 80, 15, '41242'),          -- Saronite Defender (1H sword)
(1, 0, 'tank', 78, 80, 16, '41113');          -- Saronite Bulwark (shield)
