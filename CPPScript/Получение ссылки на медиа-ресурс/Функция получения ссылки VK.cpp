// ---- Получение ссылки на vk.com --------------------------------------------
bool GetLink_VK(char sLink) {
  char sHtml, sVal, host, uid, vkid, vtag, max_hd, no_flv, res;
  char ResolutionList='0:240, 1:360, 2:480, 3:720', sQAval, sQSel;
  int i, n, iPriority=0, iMinPriority=99; bool bQualityLog;

  bQualityLog = (Pos('--qualitylog',   mpPodcastParameters)>0);
  sHtml = HmsDownloadURL(sLink, sLink, true);
  sHtml = ReplaceStr(sHtml, '\\', '');
  host  = ''; max_hd = '2';

  if (!HmsRegExMatch('vtag["\':=\\s]+([0-9a-z]+)', sHtml, vtag)) {
    if (HmsRegExMatch('(<div[^>]+video_ext_msg.*?</div>)', sHtml, sLink)) {
      sLink = HmsHtmlToText(sLink);
      HmsLogMessage(2, PodcastItem.ItemOrigin.ItemParent[mpiTitle]+': vk.com сообщает - '+sLink);

      char sFileMP3 = HmsTempDirectory+'\\sa.mp3';
      char sFileImg = HmsTempDirectory+'\\vkmsg_';
      sVal  = HmsHttpEncode('Vk.com сообщает:');
      sHtml = HmsHttpEncode(ReplaceStr(sLink, '\n', '|'));
      i = cfgTranscodingScreenHeight;
      n = cfgTranscodingScreenWidth;
      sLink = Format('http://wonky.lostcut.net/videomessage.php?h=%d&w=%d&captfont=AGFriquer_Bold&captsize=%d&fontsize=%d&caption=%s&msg=%s', [i, n, Round(i/6), Round(i/17), sVal, sHtml]);
      HmsDownloadURLToFile(sLink, sFileImg);
      for (i=1; i<=7; i++) CopyFile(sFileImg, sFileImg+Format('%.3d.jpg', [i]), false);
      try {
        if (!FileExists(sFileMP3)) HmsDownloadURLToFile('http://wonky.lostcut.net/mp3/sa.mp3', sFileMP3);
        sFileMP3 = '-i "'+sFileMP3+'" ';
      } except { sFileMP3 = ''; }
      MediaResourceLink = Format('%s-f image2 -r 1 -i "%s" -c:v libx264 -pix_fmt yuv420p ', [sFileMP3, sFileImg+'%03d.jpg']);
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
  if (bQualityLog || (mpPodcastMediaFormats!='')) for (i=StrToIntDef(max_hd, 3); i>=0; i--) {
    HmsRegExMatch(IntToStr(i)+':(\\d+)', ResolutionList, sVal);
    sQAval += sVal + '  ';
    if (sQSel != '') {
      if (StrToIntDef(res, 0)>StrToIntDef(sQSel, 0)) res = sVal;
    } else if (mpPodcastMediaFormats != '') {
      iPriority = HmsMediaFormatPriority(StrToIntDef(sVal, 0), mpPodcastMediaFormats);
      if ((iPriority>=0)&&(iPriority<iMinPriority)) {iMinPriority = iPriority; res=sVal;}
    }
  }
  if (bQualityLog) HmsLogMessage(1, mpTitle+': '+sQAval+'Выбрано: '+res);

  if (LeftCopy(uid, 1)!='u') uid = 'u' + Trim(uid);
  if (Trim(host)=='') HmsRegExMatch('ajax.preload.*?<img[^>]+src="(http://.*?/)', sHtml, host);

  if (uid=='0') MediaResourceLink = host+'assets/videos/'+vtag+''+vkid+'.vk.flv';
  else          MediaResourceLink = host + uid+'/videos/'+vtag+'.'+res+'.mp4';
  return true;
}
