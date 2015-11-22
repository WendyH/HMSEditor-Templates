// ---- Получение ссылки на Youtube -------------------------------------------
Procedure GetLink_Youtube3(sLink: String);
Var
  sData, sVideoID, sMaxHeight, sAudio, sSubtitlesLanguage, sSubtitlesUrl, sFile, sVal, sMsg: String; 
  JSON: TJsonObject; bSubtitles, bAdaptive, bNotDE: Boolean;
Begin
  sSubtitlesLanguage := 'ru';
  HmsRegExMatch('--maxheight=(\\d+)'    , mpPodcastParameters, sMaxHeight);
  HmsRegExMatch('--sublanguage=(\\w{2})', mpPodcastParameters, sSubtitlesLanguage);
  bSubtitles := (Pos('--subtitles'  , mpPodcastParameters)>0);  
  bAdaptive  := (Pos('--adaptive'   , mpPodcastParameters)>0);  
  bNotDE     := (Pos('notde=1'      , sLink)>0);  

  If Not HmsRegExMatch('[\\?&]v=([^&]+)'       , sLink, sVideoID) Then
         HmsRegExMatch('/(?:embed|v)/([^\\?]+)', sLink, sVideoID);

  If sVideoID='' Then Begin HmsLogMessage(1, 'Невозможно получить Video ID в ссылке Youtube'); Exit; End;

  sLink := 'http://hms.lostcut.net/youtube/g.php?v='+sVideoID;
  If (sMaxHeight<>''                  ) Then sLink := sLink+'&max_height='+sMaxHeight;
  If (Trim(mpPodcastMediaFormats )<>'') Then sLink := sLink+'&media_formats='+mpPodcastMediaFormats;
//If (Trim(goRoot[mpiAccessToken])<>'') Then sLink := sLink+'&auth='+goRoot[mpiAccessToken];
  If (bAdaptive                       ) Then sLink := sLink+'&adaptive=1';
  If (bNotDE                          ) Then sLink := sLink+'&notde=1';
    
  sData := HmsUtf8Decode(HmsDownloadUrl(sLink));
  // if error - exists reason
  If HmsRegExMatch('"reason":"(.*?)"' , sData, sMsg) Then Begin HmsLogMessage(2 , sMsg); Exit; End;

  JSON := TJsonObject.Create(); 
  Try 
    JSON.LoadFromString(sData);
    MediaResourceLink := JSON.S['url'   ];
    sSubtitlesUrl     := JSON.S['ttsUrl'];
    sAudio            := JSON.S['audio' ];
    If sAudio<>'' Then MediaResourceLink := Format('-i "%s" -i "%s"', [MediaResourceLink, sAudio]); 
  Finally 
    JSON.Free;
  End;
  
  If Pos('m3u8', MediaResourceLink)>0 Then MediaResourceLink := ' '+Trim(MediaResourceLink);

  // Если есть субтитры и в дополнительных параметрах еуказано их показывать - загружаем 
  If bSubtitles AND (sSubtitlesUrl<>'') Then Begin
    sFile := HmsSubtitlesDirectory+'\Youtube\'+PodcastItem.ItemID+'.'+sSubtitlesLanguage+'.srt';
    sLink := sSubtitlesUrl+'&format=srt&lang='; 
    If Not HmsDownloadURLToFile(sLink+sSubtitlesLanguage, sFile, 'Accept-Encoding: gzip, deflate') Then
           HmsDownloadURLToFile(sLink+'en'              , sFile, 'Accept-Encoding: gzip, deflate');
    PodcastItem.Properties[mpiSubtitleLanguage] := sFile;
  End;
End;
