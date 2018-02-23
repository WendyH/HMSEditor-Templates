// -------------------------------------------- Получение ссылки на vk.com ----
bool GetLink_VK(char sLink) {
  string sHtml, sVal, host, uid, vkid, vtag, max_hd, no_flv, res;
  string ResolutionList='0:240, 1:360, 2:480, 3:720', sQAval, sQSel;
  int i, iPriority=0, iMinPriority=99; int nFlag=0; 
  string sRet='', sPost='', sOid, sID, sLnk2='';
  
  sHtml = HmsDownloadUrl(sLink, 'Referer: '+gsHeaders, true);

  if ((sHtml=="") || !HmsRegExMatch('vtag["\':=\\s]+([0-9a-z]+)', sHtml, vtag)) {
    LogInSiteVK();
    if (HmsRegExMatch2("oid=(.*?)&.*?id=(.*?)&", sLink, sVal, sID))
      sLink = "https://vk.com/video"+sVal+"_"+sID;
    sHtml = HmsDownloadUrl(sLink, 'Referer: '+gsHeaders, true);
  }
  
  sHtml = ReplaceStr(sHtml, '\\', '');
  host = ''; max_hd = '2';
  
  sLink = '';
  HmsRegExMatch('--quality=(\\d+)', mpPodcastParameters, sQSel);
  if (sQSel!='') HmsRegExMatch('"url'+sQSel+'":"(.*?)"', sHtml, sLink);
  if (sLink=='') HmsRegExMatch('"url720":"(.*?)"', sHtml, sLink);
  if (sLink=='') HmsRegExMatch('"url480":"(.*?)"', sHtml, sLink);
  if (sLink=='') HmsRegExMatch('"url360":"(.*?)"', sHtml, sLink);
  if (sLink=='') HmsRegExMatch('"url240":"(.*?)"', sHtml, sLink);
  if (sLink!='') {
    MediaResourceLink = HmsJsonDecode(sLink);
    return;
  }
  
  if (!HmsRegExMatch('vtag["\':=\\s]+([0-9a-z]+)', sHtml, vtag)) {
    if (HmsRegExMatch('<div[^>]+video_ext_msg.*?>(.*?)</div>', sHtml, sLink) || 
        HmsRegExMatch('<div style="position:absolute; top:50%; text-align:center; right:0pt; left:0pt;.*?>(.*?)</div>', sHtml, sLink)) {
      HmsLogMessage(2, PodcastItem.ItemOrigin.ItemParent[mpiTitle]+': vk.com сообщает - '+HmsHtmlToText(sLink));
      VideoMessage('', 'VK.COM СООБЩАЕТ:\n\n'+HmsHtmlToText(sLink));
      
    } else if (HmsRegExMatch("ajax.preload\\('al_video.php.*?src=\\\"(.*?)\\\"", sHtml, sLink)) {
      sLink = HmsJsonDecode(sLink);
      if (LeftCopy(sLink, 2)=="//") sLink = "http:" + Trim(sLink);
      return CheckKnownLinks(sLink);
      
    } else {
      HmsLogMessage(2, mpTitle+': не удалось обработать ссылку на vk.com');
      MediaResourceLink = 'http://wonky.lostcut.net/vids/error_getlink.avi';
    }
    return true;
  }
  HmsRegExMatch('[^a-z]host[=:"\'\\s]+(.*?)["\'&;,]', sHtml, host  );
  HmsRegExMatch('[^a-z]uid[=:"\'\\s]+([0-9]+)',       sHtml, uid   );
  HmsRegExMatch('no_flv.*?(\\d)'       ,              sHtml, no_flv);
  HmsRegExMatch('(?>hd":"|hd=|video_max_hd.*?)(\\d)', sHtml, max_hd);
  HmsRegExMatch('[^a-z]vkid[=:"\'\\s]+([0-9]+)',      sHtml, vkid  );
  HmsRegExMatch(max_hd+':(\\d+)',            ResolutionList, res   );

  sQAval = 'Доступное качество: '; sQSel = '';
  HmsRegExMatch('--quality=(\\d+)', mpPodcastParameters, sQSel);

  // Если включен приоритет форматов, то ищем ссылку на более приоритетное качество
  if (gbQualityLog || (mpPodcastMediaFormats!='')) for (i=StrToIntDef(max_hd, 3); i>=0; i--) {
    HmsRegExMatch(IntToStr(i)+':(\\d+)', ResolutionList, sVal);
    sQAval += sVal + '  ';
    if (sQSel != '') {
      if (StrToIntDef(res, 0)>StrToIntDef(sQSel, 0)) res = sVal;
    } else if (mpPodcastMediaFormats != '') {
      iPriority = HmsMediaFormatPriority(StrToIntDef(sVal, 0), mpPodcastMediaFormats);
      if ((iPriority>=0)&&(iPriority<iMinPriority)) {iMinPriority = iPriority; res=sVal;}
    }
  }
  if (gbQualityLog) HmsLogMessage(1, mpTitle+': '+sQAval+'Выбрано: '+res);

  if (LeftCopy(uid, 1)!='u') uid = 'u' + Trim(uid);
  if (Trim(host)=='') HmsRegExMatch('ajax.preload.*?<img[^>]+src="(http://.*?/)', sHtml, host);
  if (LeftCopy(host, 4)!='http') host = "http://cs"+host+".vk.me/";
  if (uid=='0') MediaResourceLink = host+'assets/videos/'+vtag+''+vkid+'.vk.flv';
  else          MediaResourceLink = host + uid+'/videos/'+vtag+'.'+res+'.mp4';
  HmsRegExMatch(";url"+res+"=(.*?)&", sHtml, MediaResourceLink);
  return true;
}
