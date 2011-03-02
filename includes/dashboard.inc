<?php
require_once(dirname(__FILE__) . '/database.inc');
require_once(dirname(__FILE__) . '/api.inc');

class myGengoDashboard {

static public function render($input,$args,$parser,$frame)
{
	global $wgOut;
	$ret = '';
	$langs = myGengoDatabase::languages();
	$wgOut->addStyle('../extensions/mygengo/css/dashboard.css');
	$jobs = myGengoDatabase::jobs();
	$parser->disableCache();

	$ret .= Html::rawElement('span',array('style' => 'float: right; font-size: large;'),myGengoApi::balance() . ' credits ' . 
		Html::element('a',array('href' => 'https://mygengo.com/account/top_up'),'Top up'));

	if(count($jobs))
		foreach($jobs as $job)
			$ret .= myGengoDashboard::block($job,$langs);
	else
		$ret .= Html::element('div',array('style' => 'font-size: large;'),'No pending jobs');

	$ret .= Html::element('a',array(
		'href' => Title::makeTitle(NS_SPECIAL,'myGengo')->getLinkURL() . '?' . http_build_query(array('page' => 'poll','back' => htmlentities($wgOut->getTitle()))),
		'style' => 'float: right'),'Rebuild database');

	
	return $ret;
}

static public function render_single($jid)
{
	global $wgOut;
	$langs = myGengoDatabase::languages();
	$wgOut->addStyle('../extensions/mygengo/css/dashboard.css');
	
	return myGengoDashboard::block(myGengoDatabase::fetch($jid),$langs);
}

static public function block($job,$langs)
{
	global $wgOut;
	$tbl = '';
	$ret = '';
	$unread = 0;
	$opt = '';

	foreach($job->thread as &$c)
		if($c->new)
			$unread += 1;

	switch($job->status)
	{
		case "available": 	$opt = 'cancel'; 	break;
		case "reviewable": 	$opt = 'review'; 	break;
		case "approved":		$opt = 'view';		break;
	}

	$tbl .= Html::element('td',array('class' => 'dashboard-job-0'),sprintf('Job #%s',$job->jid));
	$tbl .= Html::element('td',array('class' => 'dashboard-job-1'),ucwords($job->status));
	$tbl .= Html::rawElement('td',array('class' => 'dashboard-job-2'),Html::element('a',array(
		'href' => Title::makeTitle(NS_SPECIAL,'myGengo')->getLinkURL() . '?' . http_build_query(array('page' => 'thread','jid' => $job->jid,'back' => htmlentities($wgOut->getTitle())))),
		count($job->thread) . ' comment' . (count($job->thread) == 1 ? '' : 's') . ($unread > 0 ? ' (' . $unread . ' new)' : '')));
	$tbl .= Html::element('td',array('class' => 'dashboard-job-3'),sprintf('%.2f credits',$job->credits));
	$tbl .= Html::element('td',array('class' => 'dashboard-job-4'),sprintf('Updated %s',strftime('%c',$job->atime)));
	$tbl .= Html::element('td',array('class' => 'dashboard-job-5'),'');

	$ret .= Html::rawElement('tr',array(),$tbl);
	
	$tbl = '';
	$tbl .= Html::element('td',array('class' => 'dashboard-job-0'),$job->slug);
	$tbl .= Html::element('td',array('class' => 'dashboard-job-1'),($job->tier == 'ultra_pro' ? 'Ultra' : ucwords($job->tier)));
	$tbl .= Html::element('td',array('class' => 'dashboard-job-2'),sprintf('%s > %s',$langs[$job->lc_src]->language,$langs[$job->lc_tgt]->language));
	$tbl .= Html::element('td',array('class' => 'dashboard-job-3'),$job->unit_count . ' ' . $langs[$job->lc_src]->unit_type . ($job->unit_count > 1 ? 's' : ''));
	$tbl .= Html::element('td',array('class' => 'dashboard-job-4'),sprintf('Ordered %s',strftime('%c',$job->ctime)));
	$tbl .= Html::rawElement('td',array('class' => 'dashboard-job-5'),$opt == '' ? '' : Html::element('a',array(
		'href' => Title::makeTitle(NS_SPECIAL,'myGengo')->getLinkURL() . '?' . http_build_query(array('page' => $opt,'jid' => $job->jid,'back' => htmlentities($wgOut->getTitle())))),
		ucwords($opt)));

	$ret .= Html::rawElement('tr',array(),$tbl);

	return Html::rawElement('table',array('class' => 'dashboard-job'),Html::rawElement('tbody',array(),$ret));
}
}
?>
