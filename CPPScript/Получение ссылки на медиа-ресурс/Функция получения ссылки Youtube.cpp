// ------------------------------------------- Получение ссылки на Youtube ----
void GetLink_Youtube3(string sLink) {
  string 
    sData, sVideoID='', sMaxHeight='', sAudio='', 
    sSubtitlesLanguage='ru', sSubtitlesUrl, sFile, sVal, sMsg; 
  TJsonObject JSON; 

  HmsRegExMatch('--maxheight=(\\d+)'    , mpPodcastParameters, sMaxHeight);
  HmsRegExMatch('--sublanguage=(\\w{2})', mpPodcastParameters, sSubtitlesLanguage);
  bool bSubtitles = (Pos('--subtitles'  , mpPodcastParameters)>0);  
  bool bAdaptive  = (Pos('--adaptive'   , mpPodcastParameters)>0);  
  bool bNotDE     = (Pos('notde=1'      , sLink)>0);  

  if (!HmsRegExMatch('[\\?&]v=([^&]+)'       , sLink, sVideoID))
       HmsRegExMatch('/(?:embed|v)/([^\\?]+)', sLink, sVideoID);

  if (sVideoID=='') { HmsLogMessage(1, 'Невозможно получить Video ID в ссылке Youtube'); return; }

  sLink = 'http://hms.lostcut.net/youtube/g.php?v='+sVideoID;
  if (sMaxHeight!=''                  ) sLink += '&max_height='+sMaxHeight;
  if (Trim(mpPodcastMediaFormats )!='') sLink += '&media_formats='+mpPodcastMediaFormats;
//if (Trim(goRoot[mpiAccessToken])!='') sLink += '&auth='+goRoot[mpiAccessToken];
  if (bAdaptive                       ) sLink += '&adaptive=1';
  if (bNotDE                          ) sLink += '&notde=1';
    
  sData = HmsUtf8Decode(HmsDownloadUrl(sLink));
  // if error - exists reason
  if (HmsRegExMatch('"reason":"(.*?)"' , sData, sMsg)) { HmsLogMessage(2 , sMsg); return; }

  JSON  = TJsonObject.Create(); 
  try {
    JSON.LoadFromString(sData);
    MediaResourceLink = JSON.S['url'   ];
    sSubtitlesUrl     = JSON.S['ttsUrl'];
    sAudio            = JSON.S['audio' ];
    if (sAudio!='') MediaResourceLink = Format('-i "%s" -i "%s"', [MediaResourceLink, sAudio]); 
  } finally { JSON.Free(); }
  
  if (Pos('m3u8', MediaResourceLink)>0) MediaResourceLink = ' '+Trim(MediaResourceLink);

  // Если есть субтитры и в дополнительных параметрах еуказано их показывать - загружаем 
  if (bSubtitles && (sSubtitlesUrl!='')) {
    sFile = HmsSubtitlesDirectory+'\\Youtube\\'+PodcastItem.ItemID+'.'+sSubtitlesLanguage+'.srt';
    sLink = sSubtitlesUrl+'&format=srt&lang='; 
    if (!HmsDownloadURLToFile(sLink+sSubtitlesLanguage, sFile, 'Accept-Encoding: gzip, deflate')) {
      HmsDownloadURLToFile(sLink+'en'                 , sFile, 'Accept-Encoding: gzip, deflate');
    }
    PodcastItem[mpiSubtitleLanguage] = sFile;
  }
}