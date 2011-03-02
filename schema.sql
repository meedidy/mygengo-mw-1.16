DROP TABLE IF EXISTS mg_job;
CREATE TABLE mg_job 
(
	jid 				INT NOT NULL PRIMARY KEY AUTO_INCREMENT,
	body_src 		MEDIUMTEXT NOT NULL,
	body_tgt 		MEDIUMTEXT,
	lc_src 			TINYTEXT,
	lc_tgt 			TINYTEXT,
	unit_count	INT,
	tier				TINYTEXT,
	credits			FLOAT,
	status			TINYTEXT,
	captcha_url	TEXT,
	preview_url	TEXT,
	slug				TINYTEXT,
	ctime				INT,
	atime				INT
);

DROP TABLE IF EXISTS mg_comment;
CREATE TABLE mg_comment 
(
	cid 		INT NOT NULL PRIMARY KEY AUTO_INCREMENT,
	jid			INT NOT NULL,
	body		TEXT,
	author	TINYTEXT,
	ctime		INT,
	new			TINYINT
);

DROP TABLE IF EXISTS mg_language;
CREATE TABLE mg_language 
(
	lc 				VARCHAR(5) NOT NULL PRIMARY KEY,
	language	TEXT NOT NULL,
	localized	TEXT NOT NULL,
	unit_type	TINYTEXT NOT NULL
);
