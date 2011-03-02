<?php

require_once(dirname(__FILE__) . '/includes/api.inc');
require_once(dirname(__FILE__) . '/includes/form.inc');

class myGengoSpecial extends SpecialPage 
{
	function __construct() 
	{
		parent::__construct('myGengo');
		wfLoadExtensionMessages('myGengo');
	}

	function execute($par)
	{
		global $wgRequest,$wgOut,$wgUser,$wgScriptPath;

		if($wgRequest->getText('page','') == 'callback')
		{
			$wgOut->clearHTML();
			$wgOut->addHTML('go away!');

			$this->callback();
			return;
		}

		$this->setHeaders();
		if(!$this->userCanExecute($wgUser))
		{
			$this->displayRestrictionError();
			return;
		}

		$wgOut->setPagetitle("myGengo");

		switch($wgRequest->getText('page',''))
		{
			case 'poll': 			$this->poll(); 																break;
			case 'thread': 		$this->thread($wgRequest->getText('jid',''));	break;
			case 'cancel':		$this->cancel($wgRequest->getText('jid',''));	break;
			case 'view':			$this->view($wgRequest->getText('jid',''));		break;
			case 'review':		$this->review($wgRequest->getText('jid',''));	break;
			case 'reject':		$this->reject($wgRequest->getText('jid',''));	break;
			case 'correct':		$this->correct($wgRequest->getText('jid',''));break;

			default: $wgOut->addWikiText('<dashboard>');
		}
		$wgOut->addHTML(Html::rawElement('span',array('style' => 'float: right;'),"powered by " . Html::element('img',array('src' => $wgScriptPath . '/extensions/mygengo/img/logo.png'),'myGengo')));		
		$wgOut->addReturnTo(Title::newFromText($wgRequest->getText('back',Title::newMainPage())));
	}

	function poll()
	{
		global $wgOut;

		$wgOut->setPagetitle('Database rebuild');

		// kill db
		myGengoDatabase::purge();

		// fetch jobs and their comment threads
		foreach(myGengoApi::jobs() as $job)
		{
			try
			{
				// job
				$full = myGengoApi::get_job($job->job_id);
				myGengoDatabase::job($full);

				// comments
				$cmnts = myGengoApi::get_comments($job->job_id);
				foreach($cmnts as &$c)
					myGengoDatabase::comment($c,$job->job_id);
				
				$wgOut->addWikiText(sprintf('* Merged job #%s (%s), %d comments',$full->job_id,ucwords($full->status),count($cmnts)));
			}
			catch(Exception $e)
			{
				$wgOut->addWikiText(sprintf('* Failed with job #%s',$job->job_id));
			}
		}

		// fetch language <-> lc pairs
		$arry1 = array();
		$arry2 = array();

		try
		{
			myGengoApi::language_pairs($arry1,$arry2);
		}
		catch(MWException $e)
		{
			$wgOut->addWikiText(sprintf('* Failed to fetch language pairs: ' . $e->getText()));
		}
			
	}

	function thread($jid)
	{
		global $wgOut,$wgRequest;

		$wgOut->addStyle('../extensions/mygengo/css/thread.css');
	
		$form = new myGengoCommentForm($jid,$wgRequest->getText('back',Title::newMainPage()));
		$form->setSubmitCallback('myGengoSpecial::thread_submit');	
		
		if($form->show())
			$wgOut->addWikiText("'''Comment posted'''");
		
		foreach(array_reverse(myGengoDatabase::thread($jid)) as $cmnt)
		{
			$tbl = '';
			
			$tbl .= Html::rawElement('tr',array('class' => 'thread-body'),Html::element('td',array(),$cmnt->body));
			$tbl .= Html::rawElement('tr',array('class' => 'thread-sig'),Html::element('td',array(),ucwords($cmnt->author) . ' | ' . strftime('%c',$cmnt->ctime)));

			$wgOut->addHtml(Html::rawElement('table',array('class' => ($cmnt->author == 'customer' ? 'thread thread-user' : 'thread')),Html::rawElement('tbody',array(),$tbl)));
		}

		myGengoDatabase::read($jid);
	}

	static public function thread_submit($data)
	{
		if(strlen($data['body']) > 0)
		{
			try
			{
				myGengoApi::comment($data['jid'],$data['body']);
				return true;
			}
			catch(MWException $e)
			{
				return 'Failed to post comment. (' . $e->getText() . ')';
			}
		}
		else
			return 'Comment field empty';
	}

	function cancel($jid)
	{
		global $wgOut;

		$wgOut->setPagetitle('Cancel job #' . $jid);
		
		try
		{
			myGengoApi::cancel($jid);
			$wgOut->addWikiText('Canceled job #' . $jid);
			myGengoDatabase::job(myGengoApi::get_job($jid));
		}
		catch(MWException $e)
		{
			$wgOut->addWikiText('Failed to cancel job #' . $jid);
		}
	}

	function view($jid)
	{
		global $wgOut,$wgRequest;

		try
		{
			$fdbck = myGengoApi::feedback($jid);
			$job = myGengoDatabase::fetch($jid);

			$wgOut->setPagetitle('View job #' . $jid);
	
			$wgOut->addHtml(myGengoDashboard::render_single($jid));

			$wgOut->addWikiText('== Translation ==');
			$wgOut->addHtml(Html::element('p',array(),$job->body_tgt));

			$wgOut->addWikiText('== Publish ==');
			$form = new myGengoPublishForm($jid,$wgRequest->getText('back',Title::newMainPage()));
			$form->setSubmitCallback('myGengoSpecial::view_submit');
			if($form->show())
			{
				$wgOut->clearHTML();
				$wgOut->redirect(Title::newFromText($form->mFieldData['title'])->getLinkURL());
				return;
			}

		
			$wgOut->addWikiText('== Feedback ==');
			$wgOut->addWikiText('Rating: ' . $fdbck->rating);
			$wgOut->addHtml(Html::element('p',array(),$fdbck->for_translator));
		}
		catch(MWException $e)
		{
			$wgOut->addWikiText($e->getText());
		}
	}

	static public function view_submit($data)
	{
		if(strlen($data['title']) == 0)
			return 'No title given';

		$a = new Article(Title::newFromText($data['title']));
		$job = myGengoDatabase::fetch($data['jid']);

		if(!$a->doEdit(preg_replace("/<.*?>/","",$job->body_tgt),'created',EDIT_NEW)->isGood())
			return $a->doEdit(preg_replace("/<.*?>/","",$job->body_tgt),'translated',EDIT_UPDATE)->isGood();
		else
			return true;
	}

	function callback()
	{
		if(isset($_POST['job']))
			myGengoDatabase::job(json_decode($_POST['job']));

		if(isset($_POST['comment']))
			// mygengo doesn't send author field
			// WORKAROUND is to receck the whole thread
			foreach(myGengoApi::get_comments(json_decode($_POST['comment'])->job_id) as $c)
				myGengoDatabase::comment($c,json_decode($_POST['comment'])->job_id);
	}

	function review($jid)
	{
		global $wgOut,$wgRequest;

		try
		{
			$fdbck = myGengoApi::feedback($jid);
			$job = myGengoDatabase::fetch($jid);

			$form = new myGengoReviewForm($jid,$wgRequest->getText('back',Title::newMainPage()));
			$form->setSubmitCallback('myGengoSpecial::review_submit');

			$wgOut->addWikiText('== Translation ==');
			$wgOut->addHtml(Html::element('img',array('src' => myGengoApi::preview($job->jid)),''));
			$wgOut->addWikiText('== Original text ==');
			$wgOut->addHtml(Html::element('p',array(),$job->body_src));

			if($form->show())
			{
				$wgOut->clearHTML();
				$wgOut->addWikiText("'''Feedback submitted'''\n\n");
				return;
			}
				
			$wgOut->addHtml(Html::rawElement('span',array('style' => 'float: right;'),
											Html::element('a',array('href' => Title::makeTitle(NS_SPECIAL,'myGengo')->getLocalURL() . '?' . http_build_query(array(
												'page' => 'reject',
												'jid' => $jid,
												'back' => htmlentities($wgOut->getTitle())))),'Reject') . 
											'&nbsp;|&nbsp;' .
											Html::element('a',array('href' => Title::makeTitle(NS_SPECIAL,'myGengo')->getLocalURL() . '?' . http_build_query(array(
												'page' => 'correct',
												'jid' => $jid,
												'back' => htmlentities($wgOut->getTitle())))),'Request corrections')));
			$wgOut->setPagetitle('Review job #' . $jid);

		}
		catch(MWException $e)
		{
			$wgOut->addWikiText($e->getText());
		}
	}

	static public function review_submit($data)
	{
		try
		{
			myGengoApi::approve($data['jid'],$data['rating'],$data['for_trans'],$data['for_mygengo'],$data['public']);
			return true;
		}
		catch(MWException $e)
		{
			return $e->getText();
		}
	}

	function reject($jid)
	{
		global $wgOut,$wgRequest;

		$job = myGengoDatabase::fetch($jid);

		try
		{
			$wgOut->addWikiText("Please use rejections sparingly, and only as a last resort.\n\nIf you're not happy with a translation, you can reject and cancel the job. However, before you receive your full refund, the myGengo Quality Control team will review your request and determine whether or not it was a fair rejection. You also have the option to pass the translation along to another translator if you don't want to cancel the job.\n\nWe're people too (and so are our translators). So please try to explain things as calmly as possible if things go wrong - as the Beatles say 'We can work it out' :)\n\nPlease read the [http://mygengo.com/help/faqs FAQ] for more informations.");

			$form = new myGengoRejectForm($jid,$job->captcha_url,$wgRequest->getText('back',Title::newMainPage()));
			$form->setSubmitCallback('myGengoSpecial::reject_submit');
			if($form->show())
				$wgOut->addWikiText("'''Rejection submitted'''");
			$wgOut->setPagetitle('Reject job #' . $jid);
		}
		catch(MWException $e)
		{
			$wgOut->addWikiText($e->getText());
		}
	}

	public static function reject_submit($data)
	{
		if(strlen($data['follow_up']) > 0 && strlen($data['reason']) > 0 && strlen($data['comment']) > 0 && strlen($data['captcha']) > 0)
		{
			if(myGengoApi::reject($data['jid'],$data['follow_up'],$data['reason'],$data['comment'],$data['captcha']))
			{
				return true;
			}
			else
				return "Captcha wrong";
		}
		else
			return "All fields are mandatory";
	}

	function correct($jid)
	{
		global $wgOut,$wgRequest;

		$wgOut->addWikiText("If you find a few small mistakes in the translation - or the translator has not fully responded to your comment requests, please select the request corrections option and explain what changes need to be done.\n\nIf you think that corrections will not be enough, then please reject the translation and either choose to pass the job onto another translator, or request a refund. If you do choose to reject a job, please give a detailed explanation of the reason. This information will be helpful for us in improving our services, as well as good feedback for the translator.\n\nPlease read the [http://mygengo.com/help/faqs FAQ] for more informations.\n");

		$form = new myGengoCorrectForm($jid,$wgRequest->getText('back',Title::newMainPage()));
		$form->setSubmitCallback('myGengoSpecial::correct_submit');
		if($form->show())
			$wgOut->addWikiText("'''Correction request submitted'''");
			
		$wgOut->setPagetitle('Request corrections for job #' . $jid);
	}

	static public function correct_submit($data)
	{
		if(strlen($data['comment']) > 0)
		{
			try
			{
				myGengoApi::correct($data['jid'],$data['comment']);
				return true;
			}
			catch(MWException $e)
			{
				return $e->getText();
			}
		}
		else
			return "Please enter a request";
	}

	public function userCanExecute($user)
	{
		global $wgMgUsers;
		return (count($wgMgUsers) == 0 && $user->isLoggedIn()) || isset($wgMgUsers[$user->getName()]);
	}
}

?>
