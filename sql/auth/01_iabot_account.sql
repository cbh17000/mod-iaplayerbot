-- ============================================================================
-- mod-iaplayerbot — Phase 1b: Auth database setup
-- Run this ONCE against your `acore_auth` database
-- ============================================================================

-- Create a dedicated account for bots
-- The account ID will need to be set in iaplayerbot.conf as IABot.Account
-- Password: iabot (sha1 hash for the account system)

INSERT INTO `account` (`id`, `username`, `sha_pass_hash`, `expansion`, `locked`)
VALUES (90000, 'IABOTACCOUNT', SHA1(CONCAT(UPPER('IABOTACCOUNT'), ':', UPPER('iabot'))), 2, 1)
ON DUPLICATE KEY UPDATE `username` = 'IABOTACCOUNT';

-- Lock the account to prevent real logins
UPDATE `account` SET `locked` = 1 WHERE `id` = 90000;

-- Grant GM level 0 (player) — bots don't need GM powers
-- The security is handled by our module, not by account permissions
DELETE FROM `account_access` WHERE `id` = 90000;

-- ============================================================================
-- IMPORTANT: After running this script, set in iaplayerbot.conf:
--   IABot.Account = 90000
--
-- Then run the characters setup script to create bot characters.
-- ============================================================================

SELECT CONCAT('IABot account created with ID: ', id) AS result
FROM `account` WHERE `username` = 'IABOTACCOUNT';
