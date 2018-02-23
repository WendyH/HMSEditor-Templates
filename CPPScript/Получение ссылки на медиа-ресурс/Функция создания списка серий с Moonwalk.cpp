///////////////////////////////////////////////////////////////////////////////
// Создание списка серий сериала с Moonwalk.cc
void CreateMoonwallkLinks(string sLink) {
  String sHtml, sData, sServ, sRef, sSerie, sVal, sHeaders, sID, sSub, sFile;
  int n, nEpisode, nSeason; THmsScriptMediaItem Item, Folder = PodcastItem;
  TJsonObject JSON; TJsonArray JARRAY, EPISODE; bool bOneSeason;
  
  sHeaders = sLink+'/\r\n'+
             'Accept-Encoding: gzip, deflate\r\n'+
             'User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; rv:13.0) Gecko/20100101 Firefox/13.0\r\n'+
             'X-Requested-With: XMLHttpRequest\r\n';
  
  gbUseSerialKPInfo = Pos("--usekpinfo", mpPodcastParameters) > 0;
  
  sHtml = HmsDownloadURL(sLink, 'Referer: '+sHeaders, true);
  if (HmsRegExMatch('<body>\\s*?</body>', sHtml, '')) {
    HmsRegExReplace('(.*?moonwalk.)(co|pw)(.*)', sLink, '$1cc$3', sLink);
    sHtml = HmsDownloadURL(sLink, 'Referer: '+sHeaders, true);
  }
  if (HmsRegExMatch('<iframe[^>]+src="(.*?)"', sHtml, sLink)) {
    if (LeftCopy(sLink, 2)=="//") sLink = "http:" + Trim(sLink);
    sHtml = HmsDownloadURL(sLink, 'Referer: '+sHeaders, true);
  }
  sHtml = HmsRemoveLineBreaks(HmsUtf8Decode(sHtml));
  
  if (Trim(mpSeriesTitle)=='') { PodcastItem[mpiSeriesTitle] = mpTitle; HmsRegExMatch('^(.*?)[\\(\\[]', mpTitle, PodcastItem[mpiSeriesTitle]); }
  
  if (!HmsRegExMatch('VideoBalancer\\((.*?)\\);', sHtml, sData)) {
    if (HmsRegExMatch('>([^<]+Error.*?)<', sHtml, sVal))
      CreateErrorItem(sVal);
    HmsLogMessage(2, mpTitle+': Не найдены данные VideoBalancer в iframe.');
    return;    
  }
  JSON = TJsonObject.Create();
  try {
    JSON.LoadFromString(sData);
    HmsRegExMatch("'(.*?)'", JSON.S['ref'], sRef);
    nSeason = 0;
    if (HmsRegExMatch2('^(.*?)\\?season=(\\d+)', sLink, sLink, sVal)) nSeason = StrToInt(sVal);
    
    // Подсчитываем количество серий и запоминаем в укромном месте
    n = JSON['episodes'].Count; PodcastItem[100508] = Str(n);
    JARRAY = JSON.A['seasons'];
    // Если в ссылке указан номер сезона - показываем только серии этого сезона
    // Если сезон всего один и номер его = 1, то сразу выкатываем серии
    if (JARRAY != nil) bOneSeason = (JARRAY.Length == 1) && (JARRAY.I[0] == 1);
    if ((nSeason > 0) || bOneSeason) {
      gsTime = '00:45:00.000';
      JARRAY = JSON.A['episodes'];
      for (n=0; n < JARRAY.Length; n++) {
        EPISODE  = JARRAY[n].AsArray;
        nEpisode = JARRAY.I[n];
        sSerie   = Format("%.2d серия", [nEpisode]); // Форматируем номер в два знака
        Item = CreateMediaItem(Folder, sSerie, sLink+'?season='+Str(nSeason)+'&episode='+Str(nEpisode)+'&ref='+sRef, mpThumbnail, gsTime);
        if (JSON.S["subtitles\\master_vtt"]!="") Item[mpiSubtitleLanguage] = HmsSubtitlesDirectory+'\\'+PodcastItem.ItemID+'.srt';
        GetSerialInfo(Item, nSeason, nEpisode);
      }
    } else if (JARRAY != nil) {
      // Создаём список сезонов как папки
      for (n=0; n < JARRAY.Length; n++) {
        nSeason  = JARRAY.I[n];
        sSerie   = Format("%d сезон", [nSeason]); // Форматируем номер в два знака
        Item = CreateFolder(Folder, sSerie, sLink+'?season='+Str(nSeason)+'&ref='+sRef, mpThumbnail);
        Item[mpiSeriesTitle] = PodcastItem[mpiSeriesTitle];
      }
    } else {
      // Просто ссылка на фильм
      Item = CreateMediaItem(Folder, mpTitle, sLink, mpThumbnail, gsTime);
    }

  } finally { JSON.Free; }
}
