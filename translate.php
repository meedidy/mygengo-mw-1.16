<?php
require_once(dirname(__FILE__) . '/includes/api.inc');
require_once(dirname(__FILE__) . '/includes/form.inc');

class myGengoTranslate extends SpecialPage 
{
	function __construct() 
	{
		parent::__construct('myGengoTranslate');
		wfLoadExtensionMessages('myGengo');
	}

	function execute($par)
	{
		global $wgRequest,$wgOut,$wgMgApiKey,$wgMgPrivateKey,$wgUser,$wgScriptPath;

		$this->setHeaders();
		if(!$this->userCanExecute($wgUser))
		{
			$this->displayRestrictionError();
			return;
		}

		$langs = array();
		$names = array();
		myGengoApi::language_pairs($names,$langs);

		$this->setHeaders();
		$wgOut->setPagetitle("myGengo Translate");
		$wgOut->addStyle('../extensions/mygengo/css/table.css');
		$wgOut->includeJQuery();
		$wgOut->addScriptFile('../../extensions/mygengo/js/mygengo.js');
		$wgOut->addScript('<script lang="javascript">
			mygengo = new Object();
			mygengo.langs = eval(\'(' . json_encode($langs) . ')\');
			mygengo.default_lang = wgUserLanguage;
		</script>');

		$cnt = array('slug' => '','content' => '');
		if($wgRequest->getText('load','') != '')
		{
			$art = Article::newFromID($wgRequest->getText('load',''));
			$cnt['content'] = preg_replace("/<.*?>/","",$art->getContent());
			$cnt['slug'] = $art->getTitle()->getText();
		}
		$wgOut->addHTML(Html::rawElement('noscript',array(),Html::element('span',array('style' => 'color: #ff0000'),'This page needs JavaScript in order to work correctly')));

		$form = new myGengoTranslateForm($cnt);
		$form->setSubmitCallback('myGengoTranslate::submit');
		if($form->show())
			$wgOut->addWikiText("'''Translation jobs submitted'''");

		$wgOut->addHTML(Html::rawElement('span',array('style' => 'float: right;'),"powered by " . Html::element('img',array('src' => $wgScriptPath . '/extensions/mygengo/img/logo.png'),'myGengo')));
	}

	public static function submit($data)
	{
		global $wgRequest;

		if(strlen($data['slug']) > 0 && strlen($data['content']) > 0)
		{
			$arr = json_decode($wgRequest->getText('mg-hidden','{}'));
			foreach($arr as &$tgt)
			{
				try
				{
					myGengoApi::post_job($data['slug'],$data['content'],$data['src_lang'],$tgt->lc,$tgt->tier,$data['comment'],$data['auto_approve'] ? 1 : 0);
				}
				catch(MWException $e)
				{
					return $e->getText();
				}
			}
			return true;
		}
		else
			return 'Content and summary fields are manatory';
	}

	public function userCanExecute($user)
	{
		global $wgMgUsers;
		return (count($wgMgUsers) == 0 && $user->isLoggedIn()) || isset($wgMgUsers[$user->getName()]);
	}

	public static function links($skin,&$acts)
	{
		$acts['mygengo'] = array(
			'text' => 'translate',
			'href' => Title::makeTitle(NS_SPECIAL,'MyGengoTranslate')->getLinkURL() . 
								'?' .
								http_build_query(array('load' => $skin->mTitle->getArticleID())),
			'class' => '');
		return true;
	}
}

?>
