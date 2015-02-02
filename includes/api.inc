<?php

require_once(dirname(__FILE__) . '/database.inc');

define('MYGENGO_URL_BASE','http://api.mygengo.com/v1/');

class myGengoApi {

static public function get($url,$opts)
{
	global $wgMgApiKey, $wgMgPrivateKey;

	$opts['ts'] = gmdate('U');
	$opts['format'] = 'json';
	$opts['api_key'] = $wgMgApiKey;
	ksort($opts);

	$ret = http_build_query($opts);
	$opts['api_sig'] = hash_hmac('sha1',$ret,$wgMgPrivateKey);

	$req = HttpRequest::factory($url . '?' . http_build_query($opts),array('method' => 'GET'));
	$req->setHeader('Accept','application/json');
	$req->setHeader('User-Agent','myGengo MediaWiki Plugin; Version 1.16; https://github.com/meedidy/mygengo-mw-1.16;');
	if($req->execute()->isOK())
		return $req->getContent();
	else
		return false;
}

static public function post($url,$opts)
{
	global $wgMgApiKey, $wgMgPrivateKey;

	$opts['ts'] = gmdate('U');
	$opts['api_key'] = $wgMgApiKey;
	ksort($opts);

	$opts['api_sig'] = hash_hmac('sha1',json_encode($opts),$wgMgPrivateKey);

	$req = HttpRequest::factory($url,array(
		'method' => 'POST',
		'postData' => http_build_query($opts)));
	$req->setHeader('Accept','application/json');
	$req->setHeader('User-Agent','myGengo MediaWiki Plugin; Version 1.16; https://github.com/meedidy/mygengo-mw-1.16;');
	$req->execute()->isOK();
	return $req->getContent();
}

static public function put($url,$opts)
{
	global $wgMgApiKey, $wgMgPrivateKey;

	$opts['_method'] = 'put';
	return myGengoApi::post($url,$opts);
}	

static public function delete($url,$opts)
{
	$opts['_method'] = 'DELETE';
	return myGengoApi::get($url,$opts);
}

static public function perform($m,$cmd,$opts)
{
	$resp = FALSE;

	if($m == 'GET')
		$resp = myGengoApi::get(MYGENGO_URL_BASE . $cmd,$opts);
	else if($m == 'POST')
		$resp = myGengoApi::post(MYGENGO_URL_BASE . $cmd,$opts);
	else if($m == 'PUT')
		$resp = myGengoApi::put(MYGENGO_URL_BASE . $cmd,$opts);
	else if($m == 'DELETE')
		$resp = delete(MYGENGO_URL_BASE . $cmd,$opts);
	else
		throw new MWException('Unknown HTTP method "' . $m . '"');

	if($resp != FALSE)
	{
		$resp = json_decode($resp);
		if($resp == NULL)
			throw new MWException('Invalid response');
		if($resp->opstat == 'error')
			if(is_string($resp->err->msg))
				throw new MWException($resp->err->msg);
	}
	else
		throw new MWException('Response empty');
	
	return $resp;
}

static public function balance()
{
	$json = myGengoApi::perform('GET','account/balance',array());

	if($json->opstat == 'ok')
		return $json->response->credits;
	else
		throw MWException($json->err->msg);
}

static public function language_pairs(&$names,&$langs)
{
	$json_l = myGengoApi::perform('GET','translate/service/languages',array());
	$json_p = myGengoApi::perform('GET','translate/service/language_pairs',array());

	if($json_l->opstat == 'ok' and $json_p->opstat == 'ok')
	{
		foreach($json_l->response as &$i)
		{
			myGengoDatabase::language($i);
			$langs[$i->lc] = $i;
			$names[$i->lc] = $i->language;
		}

		foreach($json_p->response as &$l)
			$langs[$l->lc_src]->lc_tgt[$l->lc_tgt][] = $l;
	}
	else
		throw new MWException($json_l->err->msg . ' ' . $json_p->err->msg);
}

static public function post_job($slug,$body,$lc_src,$lc_tgt,$tier,$comment,$auto_appr)
{
	$json = myGengoApi::perform('POST','translate/job',array('data' => json_encode(array(
		'job' => array(
			'body_src' => $body,
			'lc_src' => $lc_src,
			'lc_tgt' => $lc_tgt,
			'tier' => $tier,
			'slug' => $slug,
			'comment' => $comment,
			'auto_approve' => $auto_appr)))));
	
	if($json->opstat == 'ok')
	{
		myGengoDatabase::job($json->response->job);
		$c = myGengoApi::get_comments($json->response->job->job_id);
		myGengoDatabase::comment($c[0],$json->response->job->job_id);
	}
	else
		throw new MWException($json->err->msg);
}

static public function jobs()
{
	$json = myGengoApi::perform('GET','translate/jobs',array());
	
	if($json->opstat == 'ok')
		return $json->response;
	else
		throw new MWException($json->err->msg);
}

static public function get_job($jid)
{
	$json = myGengoApi::perform('GET',sprintf('translate/job/%s',$jid),array());
	
	if($json->opstat == 'ok')
		return $json->response->job;
	else
		throw new MWException($json->err->msg);
}

static public function get_comments($jid)
{
	$json = myGengoApi::perform('GET',sprintf('translate/job/%s/comments',$jid),array());
	
	if($json->opstat == 'ok')
		return $json->response->thread;
	else
		throw new MWException($json->err->msg);
}

static public function cancel($jid)
{
	$json = myGengoApi::perform('DELETE',sprintf('translate/job/%s',$jid),array());
	
	if($json->opstat == 'ok')
		return;
	else
		throw new MWException($json->err->msg);
}

static public function feedback($jid)
{
	$json = myGengoApi::perform('GET',sprintf('translate/job/%s/feedback',$jid),array());
		
	if($json->opstat == 'ok')
		return $json->response->feedback;
	else
		throw new MWException($json->err->msg);
}

static public function approve($jid,$rate,$trans,$mygengo,$public)
{
	$json = myGengoApi::perform('PUT',sprintf('translate/job/%s',$jid),array(
		'data' => json_encode(array(
			'action' => 'approve', 
			'rating' => $rate,
			'for_translator' => $trans,
			'for_mygengo' => $mygengo,
			'public' => $public))));
	
	if($json->opstat == 'ok')
	{
		myGengoDatabase::job(myGengoApi::get_job($jid));
		return;
	}
	else
		throw new MWException($json->err->msg);
}

static public function preview($jid)
{
	global $wgMgApiKey, $wgMgPrivateKey;

	$opts = array();
	$opts['ts'] = gmdate('U');
	$opts['api_key'] = $wgMgApiKey;
	ksort($opts);

	$ret = http_build_query($opts);
	$opts['api_sig'] = hash_hmac('sha1',$ret,$wgMgPrivateKey);

	return MYGENGO_URL_BASE . 'translate/job/' . $jid . '/preview' . '?' . http_build_query($opts);
}

static public function comment($jid,$body)
{
	$json = myGengoApi::perform('POST',sprintf('translate/job/%s/comment',$jid),array(
		'data' => json_encode(array(
			'body' => $body))));
		
	if($json->opstat == 'ok')
	{
		foreach(myGengoApi::get_comments($jid) as $c)
			myGengoDatabase::comment($c,$jid);
		return;
	}
	else
		throw new MWException($json->err->msg);
}

static public function reject($jid,$follow_up,$reason,$comment,$captcha)
{
	$json = myGengoApi::perform('PUT',sprintf('translate/job/%s',$jid),array(
		'data' => json_encode(array(
			'action' => 'reject', 
			'reason' => $reason,
			'comment' => $comment,
			'follow_up' => $follow_up,
			'captcha' => $captcha))));
	
	if($json->opstat == 'ok')
	{
		myGengoDatabase::job(myGengoApi::get_job($jid));
		return true;
	}
	else
	{
		myGengoDatabase::captcha($jid,$json->err->msg[1]);
		return false;
	}
}

static public function correct($jid,$comment)
{
	$json = myGengoApi::perform('PUT',sprintf('translate/job/%s',$jid),array(
		'data' => json_encode(array(
			'action' => 'revise', 
			'comment' => $comment))));
	
	if($json->opstat == 'ok')
	{
		myGengoDatabase::job(myGengoApi::get_job($jid));
		return true;
	}
	else
		throw new MWException($json->err->msg);
}
}
?>
