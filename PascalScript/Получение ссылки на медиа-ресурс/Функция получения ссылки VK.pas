///////////////////////////////////////////////////////////////////////////////
// Получение ссылки на vk.com -------------------------------------------------
Procedure GetLink_VK(sLink: String);
Var
  sHtml, sVal, host, uid, vkid, vtag, max_hd, no_flv, res, sFileMP3, sFileImg: String;
  ResolutionList: String ='0:240, 1:360, 2:480, 3:720'; sQAval, sQSel, extra, cat: String;
  i, n, iPriority: Integer = 0; iMinPriority: Integer = 99;
  bQualityLog: Boolean;
Begin
  sHtml := HmsDownloadURL(sLink, sLink, true);
  sHtml := ReplaceStr(sHtml, '\', '');
  host := ''; max_hd := '2';

  sLink := '';
  HmsRegExMatch('--quality=(\\d+)', mpPodcastParameters, sQSel);
  if sQSel<>'' Then HmsRegExMatch('"url'+sQSel+'":"(.*?)"', sHtml, sLink);
  if sLink ='' Then HmsRegExMatch('"url720":"(.*?)"', sHtml, sLink);
  if sLink ='' Then HmsRegExMatch('"url480":"(.*?)"', sHtml, sLink);
  if sLink ='' Then HmsRegExMatch('"url360":"(.*?)"', sHtml, sLink);
  if sLink ='' Then HmsRegExMatch('"url240":"(.*?)"', sHtml, sLink);
  if sLink<>'' Then Begin
    MediaResourceLink := HmsJsonDecode(sLink);
    Exit;
  End;
  
  If Not HmsRegExMatch('vtag["'':=\s]+([0-9a-z]+)', sHtml, vtag) Then Begin
    If HmsRegExMatch('(<div[^>]+video_ext_msg.*?</div>)', sHtml, sLink) Then Begin
      sLink := HmsHtmlToText(sLink);
      HmsLogMessage(2, PodcastItem.ItemOrigin.ItemParent[mpiTitle]+': vk.com сообщает - '+sLink);

      sFileMP3 := HmsTempDirectory+'\\sa.mp3';
      sFileImg := HmsTempDirectory+'\\vkmsg_';
      sVal  := HmsHttpEncode('Vk.com сообщает:');
      sHtml := HmsHttpEncode(ReplaceStr(sLink, '\n', '|'));
      i := cfgTranscodingScreenHeight;
      n := cfgTranscodingScreenWidth;
      sLink := Format('http://wonky.lostcut.net/videomessage.php?h=%d&w=%d&captfont=AGFriquer_Bold&captsize=%d&fontsize=%d&caption=%s&msg=%s', [i, n, Round(i/6), Round(i/17), sVal, sHtml]);
      HmsDownloadURLToFile(sLink, sFileImg);
      For i := 1 To 7 Do CopyFile(sFileImg, sFileImg+Format('%.3d.jpg', [i]), false);
      Try
        If Not FileExists(sFileMP3) Then HmsDownloadURLToFile('http://wonky.lostcut.net/mp3/sa.mp3', sFileMP3);
        sFileMP3 := '-i "'+sFileMP3+'" ';
      Except
        sFileMP3 := '';
      End;
      MediaResourceLink := Format('%s-f image2 -r 1 -i "%s" -c:v libx264 -pix_fmt yuv420p ', [sFileMP3, sFileImg+'%03d.jpg']);
    End Else Begin
      HmsLogMessage(2, mpTitle+': не удалось обработать ссылку на vk.com');
      MediaResourceLink := 'http://wonky.lostcut.net/vids/error_getlink.avi';
    End;
    Exit;
  End;
  HmsRegExMatch('[^a-z]host[=:"''\s]+(.*?)["''&;,]', sHtml, host  );
  HmsRegExMatch('[^a-z]uid[=:"''\s]+([0-9]+)'      , sHtml, uid   );
  HmsRegExMatch('no_flv.*?(\d)'                    , sHtml, no_flv);
  HmsRegExMatch('(?>hd":"|hd=|video_max_hd.*?)(\d)', sHtml, max_hd);
  HmsRegExMatch('[^a-z]vkid[=:"''\s]+([0-9]+)'     , sHtml, vkid  );
  HmsRegExMatch('extra=([\w-]+)'                   , sHtml, extra );
  HmsRegExMatch('/(\d+)/u\d+/'                     , sHtml, cat   );
  HmsRegExMatch(max_hd+':(\d+)',            ResolutionList, res   );

  sQAval := 'Доступное качество: '; sQSel := '';
  HmsRegExMatch('--quality=(\\d+)', mpPodcastParameters, sQSel);

  // Если включен приоритет форматов, то ищем ссылку на более приоритетное качество
  If bQualityLog OR (mpPodcastMediaFormats<>'') Then For i := StrToIntDef(max_hd, 3) DownTo 0 Do Begin
    HmsRegExMatch(IntToStr(i)+':(\\d+)', ResolutionList, sVal);
    sQAval := sQAval + sVal + '  ';
    If sQSel <> '' Then
      If StrToIntDef(res, 0) > StrToIntDef(sQSel, 0) Then res := sVal
    Else If mpPodcastMediaFormats <> '' Then Begin
      iPriority := HmsMediaFormatPriority(StrToIntDef(sVal, 0), mpPodcastMediaFormats);
      If (iPriority>=0) AND (iPriority<iMinPriority) Then Begin iMinPriority := iPriority; res:=sVal; End;
    End;
  End;
  If bQualityLog Then HmsLogMessage(1, mpTitle+': '+sQAval+'Выбрано: '+res);

  If LeftCopy(uid, 1)<>'u' Then uid := 'u' + Trim(uid);
  If Trim(host)='' Then HmsRegExMatch('ajax.preload.*?<img[^>]+src="(http://.*?/)', sHtml, host);

  If uid='0' Then MediaResourceLink := host+'assets/videos/'+vtag+''+vkid+'.vk.flv'
  Else            MediaResourceLink := host + uid+'/videos/'+vtag+'.'+res+'.mp4';
  If Trim(extra)<>'' Then MediaResourceLink := MediaResourceLink+'?extra='+extra;
  If Trim(cat  )<>'' Then MediaResourceLink := ReplaceStr(MediaResourceLink, '/'+uid, '/'+cat+'/'+uid);
  
  HmsRegExMatch(";url"+res+"=(.*?)&", sHtml, MediaResourceLink);
End;
