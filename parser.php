<?php
require_once(dirname(__FILE__) . '/includes/dashboard.inc');

class myGengoParser
{
	static public function setup(&$parser)
	{
		$parser->setHook('dashboard','myGengoDashboard::render');
		return true;
	}
}
?>
