// ---- Получение ссылки Moonwalk.cc ------------------------------------------
void GetLink_Moonwalk(string sLink) {
  string sHtml, sData, sPost, sManifest, sVer, sSec, sQual, sVal, sServ;
  float f; TRegExpr RegExp; Boolean bHdsDump;
  string sHeaders = gsUrlBase+'/'#13#10+
              'Accept-Encoding: gzip, deflate'#13#10+
              'User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; rv:13.0) Gecko/20100101 Firefox/13.0'#13#10+
              'Origin: http://moonwalk.cc'#13#10+
              'X-Requested-With: XMLHttpRequest'#13#10+
              'Accept: */*'#13#10+
              'Cookie: clcktest=1;'#13#10;

  HmsRegExMatch('--quality=(\w+)', mpPodcastParameters, sQual);
  bHdsDump = Pos('--hdsprofile', mpPodcastParameters) > 0;

  HmsRegExMatch2('//(.*?)/(.*)', sLink, sServ, sLink);
  sHtml = HmsSendRequestEx(sServ, sLink, 'GET',
         'application/x-www-form-urlencoded; Charset=UTF-8', sHeaders, '', 80, 0, sVal, true);
  if (HmsRegExMatch('Set-Cookie: (.*?);', sVal, sVal))
    sHeaders = sHeaders+'Cookie: '+sVal+';'#13#10; 

  if (HmsRegExMatch('X-MOON-EXPIRED.*?"(.*?)"', sHtml, sVal)) sHeaders += 'X-MOON-EXPIRED: '+sVal+#13#10; 
  if (HmsRegExMatch('X-MOON-TOKEN.*?"(.*?)"'  , sHtml, sVal)) sHeaders += 'X-MOON-TOKEN: '  +sVal+#13#10; 
  
  if (!HmsRegExMatch('/create_session.*?\{(.*?)\}', sHtml, sPost)) {
    HmsLogMessage(2, mpTitle+': Не найдены параметры create_session на странице.');
  }
  sPost = ReplaceStr(sPost, ':', '=');
  sPost = ReplaceStr(sPost, ' ', '');
  sPost = ReplaceStr(sPost, "'", '');
  sPost = ReplaceStr(sPost, ',', '&');
  sPost = ReplaceStr(sPost, 'condition_detected?1=', '');
  sPost = HmsRemoveLineBreaks(sPost);

  sData = HmsSendRequest('moonwalk.cc', '/sessions/create_session', 'POST',
         'application/x-www-form-urlencoded; Charset=UTF-8', sHeaders, sPost, 80, true);

  if (HmsRegExMatch('"manifest_m3u8"\s*?:\s*?"(.*?)"', sData, sLink))
    MediaResourceLink = ' ' + HmsJsonDecode(sLink);

    // Получение длительности видео, если она не установлена
    sData = HmsDownloadUrl(sLink);
    if ((Trim(PodcastItem[mpiTimeLength])=='') || (PodcastItem[mpiTimeLength]=='01:40:00.000')) {
      if (HmsRegExMatch('(http.*?)[\r\n$]', sData, sLink)) {
        sHtml = HmsDownloadUrl(sLink, 'Referer: '+mpFilePath, true);
        RegExp = TRegExpr.Create('#EXTINF:(\d+.\d+)', PCRE_SINGLELINE);
        if (RegExp.Search(sHtml)) do {
          f += StrToFloatDef(RegExp.Match(1), 0);
        } while (RegExp.SearchAgain());
        RegExp.Free;
        PodcastItem.Properties[mpiTimeLength] = Round(f);
      }

    if (sQual!='') {
      if      (sQual=='low'   ) sQual = '360';
      else if (sQual=='medium') sQual = '640'; 
      else if (sQual=='high'  ) sQual = '720'; 
      if (HmsRegExMatch('RESOLUTION=\d+x'+sQual+'.*?(http.*?)[\r\n$]', sData, sLink))
        MediaResourceLink = ' '+sLink;
      else if (HmsRegExMatch('RESOLUTION=\\d+x480.*?(http.*?)[\r\n$]', sData, sLink)) {
        sQual = '480'; 
        MediaResourceLink = ' '+sLink;
      }
      
    }

  } else
    HmsLogMessage(2, mpTitle+': Ошибка получения manifest_m3u8 на странице по ссылке.');

}
