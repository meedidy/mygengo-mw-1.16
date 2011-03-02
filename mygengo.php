<?php

if (!defined('MEDIAWIKI')) 
{
	echo "This is the myGengo extension for MediaWiki.";
	echo "Create a 'mygengo' directory in <MW>/extensions/ and copy all files there";
	echo "that came with this dirstibution.";
	echo "To install, put the following line in LocalSettings.php:";
	echo "require_once(\"\$IP/extensions/myGengo/myGengo.php\");";
	exit(1);
}

// files
$dir = dirname(__FILE__) . '/';

$wgAutoloadClasses['myGengoTranslate'] = $dir . 'translate.php';
$wgAutoloadClasses['myGengoSpecial'] = $dir . 'special.php';
$wgAutoloadClasses['myGengoParser'] = $dir . 'parser.php';

$wgExtensionMessagesFiles['myGengo'] = $dir . 'i18n.php';
//$wgExtensionAliasesFiles['myGengo'] = $dir . 'alias.php';

// special pages
$wgSpecialPages['mygengotranslate'] = 'myGengoTranslate';
$wgSpecialPages['mygengo'] = 'myGengoSpecial';

// default config parameters
$wgMgApiKey = "";
$wgMgPrivateKey = "";
$wgMgUsers = array();

$wgExtensionCredits['mygengo'][] = array(
	'name' => 'myGengo Dashboard',
	'description' => 'Administer your translation jobs.',
	'version' => 1,
	'author' => 'meedidy',
	'url' => 'http://mygengo.com');

$wgHooks['ParserFirstCallInit'][] = 'myGengoParser::setup';
$wgHooks['SkinTemplateTabs'][] = 'myGengoTranslate::links';
?>
