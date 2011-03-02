<?php
class myGengoDatabase {

static public function job($obj)
{
	$where = array('jid' => mysql_real_escape_string($obj->job_id));
	$res = wfGetDB(DB_SLAVE)->select('mg_job','*',$where);
	$new = get_object_vars($obj);

	$new['jid'] = $new['job_id'];
	unset($new['job_id']);
	unset($new['mt']);
	unset($new['callback_url']);
	unset($new['auto_approve']);
	unset($new['eta']);
	$new['atime'] = gmdate('U');


	if($res->numRows() >= 1)
	{
		$cur = get_object_vars($res->fetchObject());
		$cur = array_merge($cur,$new);

		wfGetDB(DB_MASTER)->update('mg_job',$cur,$where);
	}
	else
		wfGetDB(DB_MASTER)->insert('mg_job',$new);
}

static public function comment($obj,$jid)
{
	// pervent double insert
	$res = wfGetDB(DB_SLAVE)->select('mg_comment','cid',array(
		'ctime' => mysql_real_escape_string($obj->ctime),
		'author' => mysql_real_escape_string($obj->author)));
	
	if($res->numRows() == 0)
	{
		$new = get_object_vars($obj);
		$new['jid'] = $jid;
		$new['new'] = TRUE;
		wfGetDB(DB_MASTER)->insert('mg_comment',$new);
	}
}

static public function language($obj)
{
	$new = get_object_vars($obj);
	$new['localized'] = $new['localized_name'];
	unset($new['localized_name']);

	if(wfGetDB(DB_SLAVE)->select('mg_language','lc',array('lc' => mysql_real_escape_string($obj->lc)))->numRows() == 0)
		wfGetDB(DB_MASTER)->insert('mg_language',$new);
}

static public function languages()
{
	$ret = array();

	$res = wfGetDB(DB_SLAVE)->select('mg_language','*');
	if($res->numRows() >= 1)
		foreach($res as $lang)
			$ret[$lang->lc] = $lang;
	return $ret;
}

static public function read($jid)
{
	wfGetDB(DB_MASTER)->update('mg_comment',array('new' => FALSE),array('jid' => mysql_real_escape_string($jid)));
}

static public function purge()
{
	wfGetDB(DB_MASTER)->delete('mg_job','*');
	wfGetDB(DB_MASTER)->delete('mg_comment','*');
}

static public function jobs()
{
	$ret = array();
	
	foreach(wfGetDB(DB_SLAVE)->select('mg_job','*') as $j_obj)
	{
		$j_obj->{'thread'} = myGengoDatabase::thread($j_obj->jid);
		$ret[] = $j_obj;
	}

	return $ret;
}

static public function fetch($jid)
{
	$res = wfGetDB(DB_SLAVE)->select('mg_job','*',array('jid' => mysql_real_escape_string($jid)));
	if($res->numRows() > 0)
	{
		$job = $res->fetchObject();
		$job->{'thread'} = myGengoDatabase::thread($job->jid);
		return $job;
	}
	else
		throw new MWException('No job #' . $jid);
}

static public function thread($jid)
{
	$ret = array();
	foreach(wfGetDB(DB_SLAVE)->select('mg_comment','*',array('jid' => mysql_real_escape_string($jid))) as $cmnt)
		$ret[] = $cmnt;
	return $ret;
}

static public function captcha($jid,$cap)
{
	wfGetDB(DB_MASTER)->update('mg_job',array('captcha_url' => $cap),array('jid' => mysql_real_escape_string($jid)));
}
}
?>
