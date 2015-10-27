// ---- Получение ссылки на поток moonwalk.cc --------------------------------------
Procedure GetLink_Moonwalk(sLink: String);
Var
  sHtml, sData, sPost, sManifest, sVer, sSec, sQual, sVal, sServ: String;
  f: Extended = 0; RegExp: TRegExpr; bHdsDump: Boolean;
  sHeaders: String = gsUrlBase+'/'#13#10+
              'Accept-Encoding: gzip, deflate'#13#10+
              'User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; rv:13.0) Gecko/20100101 Firefox/13.0'#13#10+
              'Origin: http://moonwalk.cc'#13#10+
              'X-Requested-With: XMLHttpRequest'#13#10+
              'Accept: */*'#13#10+
              'Cookie: clcktest=1;'#13#10;
Begin
  HmsRegExMatch('--quality=(\w+)', mpPodcastParameters, sQual);
  bHdsDump := Pos('--hdsprofile', mpPodcastParameters) > 0;

  HmsRegExMatch2('//(.*?)/(.*)', sLink, sServ, sLink);
  sHtml := HmsSendRequestEx(sServ, sLink, 'GET',
         'application/x-www-form-urlencoded; Charset=UTF-8', sHeaders, '', 80, 0, sVal, true);
  If HmsRegExMatch('Set-Cookie: (.*?);', sVal, sVal) Then 
    sHeaders := sHeaders+'Cookie: '+sVal+';'#13#10; 

  If HmsRegExMatch('X-MOON-EXPIRED.*?"(.*?)"', sHtml, sVal) Then sHeaders := sHeaders+'X-MOON-EXPIRED: '+sVal+#13#10; 
  If HmsRegExMatch('X-MOON-TOKEN.*?"(.*?)"'  , sHtml, sVal) Then sHeaders := sHeaders+'X-MOON-TOKEN: '  +sVal+#13#10; 
  
  If Not HmsRegExMatch('/create_session.*?\{(.*?)\}', sHtml, sPost) Then Begin
    sVal := 'Не найдены параметры create_session на странице.';
    HmsLogMessage(2, mpTitle+': '+sVal);
  End;
  sPost := ReplaceStr(sPost, ':', '=');
  sPost := ReplaceStr(sPost, ' ', '');
  sPost := ReplaceStr(sPost, "'", '');
  sPost := ReplaceStr(sPost, ',', '&');
  sPost := ReplaceStr(sPost, 'condition_detected?1=', '');
  sPost := HmsRemoveLineBreaks(sPost);

  sData := HmsSendRequest('moonwalk.cc', '/sessions/create_session', 'POST',
         'application/x-www-form-urlencoded; Charset=UTF-8', sHeaders, sPost, 80, true);

  If HmsRegExMatch('"manifest_m3u8"\s*?:\s*?"(.*?)"', sData, sLink) Then Begin
    MediaResourceLink := ' ' + HmsJsonDecode(sLink);

    // Получение длительности видео, если она не установлена
    sData := HmsDownloadUrl(sLink);
    If (Trim(PodcastItem[mpiTimeLength])='') OR (PodcastItem[mpiTimeLength]='01:40:00.000') Then
      If HmsRegExMatch('(http.*?)[\r\n$]', sData, sLink) Then Begin
        sHtml := HmsDownloadUrl(sLink, 'Referer: '+mpFilePath, true);
        RegExp := TRegExpr.Create('#EXTINF:(\d+.\d+)', PCRE_SINGLELINE);
        If RegExp.Search(sHtml) Then Repeat
          f := f + StrToFloatDef(RegExp.Match(1), 0);
        Until Not RegExp.SearchAgain();
        RegExp.Free;
        PodcastItem.Properties[mpiTimeLength] := Round(f);
      End;

    If sQual<>'' Then Begin
      If      sQual='low'    Then sQual := '360'
      Else If sQual='medium' Then sQual := '640' 
      Else If sQual='high'   Then sQual := '720'; 
      If HmsRegExMatch('RESOLUTION=\d+x'+sQual+'.*?(http.*?)[\r\n$]', sData, sLink) Then
        MediaResourceLink := ' '+sLink
      Else If HmsRegExMatch('RESOLUTION=\\d+x480.*?(http.*?)[\r\n$]', sData, sLink) Then Begin
        sQual := '480'; 
        MediaResourceLink := ' '+sLink;
      End;
      
    End;

  End Else
    HmsLogMessage(2, mpTitle+': Ошибка получения manifest_m3u8 на странице по ссылке.');

End;