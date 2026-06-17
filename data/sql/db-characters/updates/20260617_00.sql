DROP TABLE IF EXISTS `mod_altexpboost_character_settings`;
CREATE TABLE `mod_altexpboost_character_settings` (
	`guid` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`enabled` TINYINT(3) UNSIGNED NOT NULL DEFAULT '1',
	PRIMARY KEY (`guid`) USING BTREE
);
