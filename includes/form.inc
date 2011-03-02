<?php

class myGengoHTMLFormTable extends HTMLFormField
{
	function getInputHTML($value)
	{
		$head = '';
		$body = '';
		
		if(array_key_exists('head',$this->mParams))
			foreach($this->mParams['head'] as &$h)
				$head .= Html::element('td',array(),$h);

		$head = Html::rawElement('thead',array(),$head);


		if(array_key_exists('body',$this->mParams))
			foreach($this->mParams['body'] as &$row)
			{
				$tr = '';
				foreach($row as $c)
					$tr .= Html::rawElement('td',array(),$c);
				$body .= Html::element('tr',array(),$tr);
			}
		$body = Html::rawElement('tbody',array(),$body);

		$attrs = array('class' => 'tgt-lang-table');
		if(array_key_exists('id',$this->mParams))
			$attrs['id'] = $this->mParams['id'];
			
	
		return Html::rawElement('table',$attrs,$head . $body);
	}
}

class myGengoHTMLFormLink extends HTMLFormField
{
	function getInputHTML($value)
	{
		return Html::element('a',array('href' => $this->mParams['href']),$value);
	}
}

class myGengoHTMLFormRaw extends HTMLFormField
{
	function getInputHTML($value)
	{
		return $value;
	}
}

class myGengoTranslateForm extends HTMLForm
{
	function __construct($content)
	{
		parent::__construct(array(
			'slug' => array(
				'type' => 'text',
				'label-message' => 'summary',
				'section' => 'job',
				'default' => $content['slug']),
			'content' => array(
				'type' => 'textarea',
				'label-message' => 'content',
				'section' => 'job',
				'cols' => 150,
				'rows' => 20,
				'default' => $content['content'],
				'id' => 'mg-content'),
			'src_lang' => array(
				'type' => 'select',
				'label-message' => 'src-lang',
				'options' => array(
					'English' => 'en',
					'German' => 'de'),
				'default' => 'en',
				'section' => 'job',
				'id' => 'mg-src-lang'),
			'tgt_tbl' => array(
				'class' => 'myGengoHTMLFormTable',
				'label-message' => 'cur-tgt',
				'section' => 'tgt-langs',
				'head' => array('Target language','Tier','Cost','Command'),
				'body' => array(),
				'id' => 'mg-table'),
			'tgt_lang' => array(
				'type' => 'select',
				'label-message' => 'tgt-lang',
				'options' => array(	
					'English' => 'en',
					'German' => 'de'),
				'default' => 'en',
				'section' => 'tgt-langs',
				'id' => 'mg-tgt-lang'),
			'tgt_tier' => array(
				'type' => 'radio',
				'label-message' => 'tier',
				'options' => array_flip(array(
					"machine" => "Machine", 
					"standard" => "Standard",
					"pro" => "Pro",
					"ultra" => "Ultra")),
				'default' => 'pro',
				'section' => 'tgt-langs',
				'id' => 'mg-tier'),
			'tgt_add' => array(
				'class' => 'myGengoHTMLFormLink',
				'default' => 'Add language',
				'nodata' => 'Add language',
				'href' => 'javascript:add_language();',
				'section' => 'tgt-langs',
				'id' => 'mg-add'),
			'comment' => array(
				'type' => 'textarea',
				'label-message' => 'job-comment',
				'help-message' => 'comment-desc',
				'section' => 'misc',
				'cols' => 150,
				'rows' => 10,
				'id' => 'mg-comment'),
			'auto_approve' => array(
				'type' => 'check',
				'label-message' => 'auto-appr',
				'default' => 1,
				'section' => 'misc',
				'id' => 'mg-auto-appr'),
			'cost_lb' => array(
				'class' => 'myGengoHTMLFormRaw',
				'default' => '<span style="font-size: large;">Price: <span id="mg-cost">0.00</span></span>',
				'nodata' => '<span style="font-size: large;">Price: <span id="mg-cost">0.00</span></span>'),
			'mg-hidden' => array(
				'type' => 'hidden',
				'default' => '',
				'id' => 'mg-hidden')));
	
		$this->setMessagePrefix("mg");
		$this->setTitle(Title::makeTitle(NS_SPECIAL,'myGengoTranslate'));
		$this->setSubmitText('Submit job');
	}
}

class myGengoPublishForm extends HTMLForm
{
	function __construct($jid,$back)
	{
		parent::__construct(array(
			'title' => array(
				'type' => 'text',
				'label-message' => 'publish-as',
				'help-message' => 'publish-help',
				'required' => TRUE),
			'jid' => array(
				'type' => 'hidden',
				'default' => $jid),
			'page' => array(
				'type' => 'hidden',
				'default' => 'view'),
			'back' => array(
				'type' => 'hidden',
				'default' => $back)));

		$this->setMessagePrefix("mg");
		$this->setTitle(Title::makeTitle(NS_SPECIAL,'myGengo'));
		$this->setSubmitText('Publish job');
	}
}

class myGengoReviewForm extends HTMLForm
{
	function __construct($jid,$back)
	{
		parent::__construct(array(
			'rating' => array(
				'type' => 'radio',
				'label-message' => 'rating',
				'options' => array(
					'1 Bad' => '1',
					'2' => '2',
					'3' => '3',
					'4' => '4',
					'5 Great' => '5'),
				'section' => 'feedback',
				'required' => TRUE,
				'default' => 3),
			'for_trans' => array(
				'type' => 'textarea',
				'label-message' => 'for-translator',
				'rows' => 6,
				'section' => 'feedback'),
			'for_mygengo' => array(
				'type' => 'textarea',
				'label-message' => 'for-mygengo',
				'rows' => 6,
				'section' => 'feedback'),
			'public' => array(
				'type' => 'check',
				'label-message' => 'pub-msg',
				'help-message' => 'pub-desc',
				'default' => '',
				'section' => 'feedback'),
			'jid' => array(
				'type' => 'hidden',
				'default' => $jid),
			'page' => array(
				'type' => 'hidden',
				'default' => 'review'),
			'back' => array(
				'type' => 'hidden',
				'default' => $back)));

		$this->setMessagePrefix("mg");
		$this->setTitle(Title::makeTitle(NS_SPECIAL,'myGengo'));
		$this->setSubmitText('Approve job');
	}
}

class myGengoCommentForm extends HTMLForm
{
	function __construct($jid,$back)
	{
		global $wgOut;

		parent::__construct(array(
			'body' => array(
				'type' => 'textarea',
				'label-message' => 'your-comment',
				'rows' => '6',
				'section' => 'new-comment'),
			'jid' => array(
				'type' => 'hidden',
				'default' => $jid),
			'back' => array(
				'type' => 'hidden',
				'default' => $back),
			'page' => array(
				'type' => 'hidden',
				'default' => 'thread')));

		$this->setMessagePrefix("mg");
		$this->setTitle(Title::makeTitle(NS_SPECIAL,'myGengo'));
		$wgOut->setPagetitle('Comment thread for job #' . $jid);
		$this->setSubmitText('Submit comment');
	}
}

class myGengoRejectForm extends HTMLForm
{
	function __construct($jid,$cap,$back)
	{
		parent::__construct(array(
			'follow_up' => array(
				'type' => 'radio',
				'label-message' => 'cancel-q',
				'default' => 'requeue',
				'options' => array_flip(array(
					'cancel' => 'Yes, please cancel and refund me',
					'requeue' => 'No, please have another translator finish the job'))),
			'reason' => array(
				'type' => 'radio',
				'label-message' => 'rejection-reason',
				'default' => 'quality',
				'options' => array_flip(array(
					'quality' => 'Poor quality of the translation',
					'incomplete' => 'Missing or incomplete translation',
					'other' => 'Other (please describe below)'))),
			'comment' => array(
				'type' => 'textarea',
				'label-message' => 'feedback-trans',
				'rows' => '8'),
			'captcha_img' => array(
				'class' => 'myGengoHTMLFormRaw',
				'default' => Html::element('img',array('src' => $cap),''),
				'nodata' => ' '),
			'captcha' => array(
				'type' => 'text',
				'label-message' => 'captcha-msg'),
			'back' => array(
				'type' => 'hidden',
				'default' => $back),
			'jid' => array(
				'type' => 'hidden',
				'default' => $jid),
			'page' => array(
				'type' => 'hidden',
				'default' => 'reject')));

		$this->setMessagePrefix("mg");
		$this->setTitle(Title::makeTitle(NS_SPECIAL,'myGengo'));
		$this->setSubmitText('Reject translation');
	}
}

class myGengoCorrectForm extends HTMLForm
{
	function __construct($jid,$back)
	{
		global $wgOut, $wgRequest;
		parent::__construct(array(
			'comment' => array(
				'type' => 'textarea',
				'label-message' => 'correct-req'),
			'back' => array(
				'type' => 'hidden',
				'default' => $back),
			'jid' => array(
				'type' => 'hidden',
				'default' => $jid),
			'page' => array(
				'type' => 'hidden',
				'default' => 'correct')));

		$this->setMessagePrefix("mg");
		$this->setTitle(Title::makeTitle(NS_SPECIAL,'myGengo'));
		$this->setSubmitText('Request corrections from translator');
	}
}

?>
