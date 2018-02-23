// ------------------------------------------- Получение ссылки на Youtube ----
bool GetLink_Youtube31(string sLink) {
  string sData, sVideoID='', sMaxHeight='', sAudio='', sSubtitlesLanguage='ru';
  string sSubtitlesUrl, sFile, sVal, sMsg, sConfig, sHeaders, ttsDef; 
  TJsonObject JSON; TRegExpr RegEx;
  
  sHeaders = 'Referer: '+sLink+#13#10+
             'User-Agent: Mozilla/5.0 (Windows NT 6.1; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/61.0.3163.100 Safari/537.36'+#13#10+
             'Origin: http://www.youtube.com'+#13#10;
  
  HmsRegExMatch('--maxheight=(\\d+)'    , mpPodcastParameters, sMaxHeight);
  HmsRegExMatch('--sublanguage=(\\w{2})', mpPodcastParameters, sSubtitlesLanguage);
  bool bSubtitles = (Pos('--subtitles'  , mpPodcastParameters)>0);  
  bool bAdaptive  = (Pos('--adaptive'   , mpPodcastParameters)>0);  
  bool bNotDE     = (Pos('notde=1'      , sLink)>0);  

  if (!HmsRegExMatch('[\\?&]v=([^&]+)'       , sLink, sVideoID))
  if (!HmsRegExMatch('youtu.be/([^&]+)'      , sLink, sVideoID))
       HmsRegExMatch('/(?:embed|v)/([^\\?]+)', sLink, sVideoID);

  if (sVideoID=='') return VideoMessage('Невозможно получить Video ID в ссылке Youtube');

  sLink = 'http://www.youtube.com/watch?v='+sVideoID+'&hl=ru&persist_hl=1&has_verified=1';
  
  sData = HmsDownloadURL(sLink, sHeaders, true);
  //sData = HmsUtf8Decode(sData);
  sData = HmsRemoveLineBreaks(sData);
  // Если еще не установлена реальная длительность видео - устанавливаем
  if ((Trim(mpTimeLength)=='') || (RightCopy(mpTimeLength, 6)=='00.000')) {
    if (HmsRegExMatch2('itemprop="duration"[^>]+content="..(\\d+)M(\\d+)S', sData, sVal, sMsg)) {
      PodcastItem[mpiTimeLength] = StrToInt(sVal)*60+StrToInt(sMsg);
    } else {
      sVal = HmsDownloadURL('http://www.youtube.com/get_video_info?html5=1&c=WEB&cver=html5&cplayer=UNIPLAYER&hl=ru&video_id='+sVideoID, sHeaders, true);
      if (HmsRegExMatch('length_seconds=(\\d+)', sData, sMsg))
        PodcastItem[mpiTimeLength] = StrToInt(sMsg);
    }
  }
  if (!HmsRegExMatch('player.config\\s*?=\\s*?({.*?});', sData, sConfig)) {
    // Если в загруженной странице нет нужной информации, пробуем немного по-другому
    sLink = 'http://hms.lostcut.net/youtube/g.php?v='+sVideoID;
    if (sMaxHeight!=''                  ) sLink += '&max_height='+sMaxHeight;
    if (Trim(mpPodcastMediaFormats )!='') sLink += '&media_formats='+mpPodcastMediaFormats;
    if (bAdaptive                       ) sLink += '&adaptive=1';
    sData = HmsUtf8Decode(HmsDownloadUrl(sLink));
    if (HmsRegExMatch('"reason":"(.*?)"' , sData, sMsg)) { 
      HmsLogMessage(2 , sMsg); 
      VideoMessage(sMsg); 
      return; 
    } else {
      sData = HmsJsonDecode(sData);
      HmsRegExMatch('"url":"(.*?)"', sData, MediaResourceLink);
      return true;
    }
  }
  
  String hlsUrl, ttsUrl, flp, jsUrl, dashMpdLink, streamMap, playerId, algorithm;
  String sType, itag, sig, alg, s;
  String UrlBase = "";
  int  i, n, w, num, height, priority, minPriority = 90, selHeight, maxHeight = 1080;
  bool is3D; 
  TryStrToInt(sMaxHeight, maxHeight);
  JSON = TJsonObject.Create();
  try {
    JSON.LoadFromString(sConfig);
    hlsUrl      = HmsExpandLink(JSON.S['args\\hlsvp' ], UrlBase);
    ttsUrl      = HmsExpandLink(JSON.S['args\\caption_tracks'], UrlBase);
    flp         = HmsExpandLink(JSON.S['url'         ], UrlBase);
    jsUrl       = HmsExpandLink(JSON.S['assets\\js'  ], UrlBase);
    streamMap   = JSON.S['args\\url_encoded_fmt_stream_map'];
    if (bAdaptive && JSON.B['args\\adaptive_fmts']) 
      streamMap = JSON.S['args\\adaptive_fmts'];
    if ((streamMap=='') && (hlsUrl=='')) {
      sMsg = "Невозможно найти данные для воспроизведения на странице видео.";
      if (HmsRegExMatch('(<h\\d[^>]+class="message".*?</h\\d>)', sData, sMsg)) sMsg = HmsUtf8Decode(HmsHtmlToText(sMsg));
      HmsLogMessage(2, sMsg);
      VideoMessage(sMsg); 
      return;
    }
  } finally { JSON.Free; }
  if (Copy(jsUrl, 1, 2)=='//') jsUrl = 'http:'+Trim(jsUrl);
  HmsRegExMatch('/player-([\\w_-]+)/', jsUrl, playerId);
  algorithm = HmsDownloadURL('https://hms.lostcut.net/youtube/getalgo.php?jsurl='+HmsHttpEncode(jsUrl));
  
  if (hlsUrl!='') {
    MediaResourceLink = ' '+hlsUrl;

    sData = HmsDownloadUrl(sLink, sHeaders, true);
    RegEx = TRegExpr.Create('BANDWIDTH=(\\d+).*?RESOLUTION=(\\d+)x(\\d+).*?(http[^#]*)', PCRE_SINGLELINE);
    try {
      if (RegEx.Search(sData)) do {
        sLink = '' + RegEx.Match(4);
        height = StrToIntDef(RegEx.Match(3), 0);
        if (mpPodcastMediaFormats!='') {
          priority = HmsMediaFormatPriority(height, mpPodcastMediaFormats);
          if ((priority>=0) && (priority>minPriority)) {
            MediaResourceLink = sLink; minPriority = priority;
          }
        } else if ((height > selHeight) && (height <= maxHeight)) {
          MediaResourceLink = sLink; selHeight = height;
        }
      } while (RegEx.SearchAgain());
    } finally { RegEx.Free(); }
  
  } else if (streamMap!='') {
      i=1; while (i<=Length(streamMap)) {
        sData = Trim(ExtractStr(streamMap, ',', i));
        sType = HmsHttpDecode(ExtractParam(sData, 'type', '', '&'));
        itag  = ExtractParam(sData, 'itag'    , '', '&');
        is3D  = ExtractParam(sData, 'stereo3d', '', '&') == '1';
        sLink = '';
        if (Pos('url=', sData)>0) {
          sLink = ' ' + HmsHttpDecode(ExtractParam(sData, 'url', '', '&'));
          if (Pos('&signature=', sLink)==0) {
            sig = HmsHttpDecode(ExtractParam(sData, 'sig', '', '&'));    
            if (sig=='') {
              sig = HmsHttpDecode(ExtractParam(sData, 's', '', '&'));
              for (w=1; w<=WordCount(algorithm, ' '); w++) {
                alg = ExtractWord(w, algorithm, ' ');
                if (Length(alg)<1) continue;
                if (Length(alg)>1) TryStrToInt(Copy(alg, 2, 4), num);
                if (alg[1]=='r') {s=''; for(n=Length(sig); n>0; n--) s+=sig[n]; sig = s;   } // Reverse
                if (alg[1]=='s') {sig = Copy(sig, num+1, Length(sig));                     } // Clone
                if (alg[1]=='w') {n = (num-Trunc(num/Length(sig)))+1; Swap(sig[1], sig[n]);} // Swap
              }
            }
            if (sig!='') sLink += '&signature=' + sig;
          }
        }
        if (itag in ([139,140,141,171,172])) { sAudio = sLink; continue; }
        if (sLink!='') {
          height = 0; //http://www.genyoutube.net/formats-resolution-youtube-videos.html
            if      (itag in ([13,17,160                  ])) height = 144;
            else if (itag in ([5,36,92,132,133,242        ])) height = 240;
            else if (itag in ([6                          ])) height = 270;
            else if (itag in ([18,34,43,82,100,93,134,243 ])) height = 360;
            else if (itag in ([35,44,83,101,94,135,244,43 ])) height = 480;
            else if (itag in ([22,45,84,102,95,136,298,247])) height = 720;
            else if (itag in ([37,46,85,96,137,248,299    ])) height = 1080;
            else if (itag in ([264,271                    ])) height = 1440;
            else if (itag in ([266,138                    ])) height = 2160;
            else if (itag in ([272                        ])) height = 2304;
            else if (itag in ([38                         ])) height = 3072;
            else continue;
            if (mpPodcastMediaFormats!='') {
              priority = HmsMediaFormatPriority(height, mpPodcastMediaFormats);
              if ((priority>=0) || (priority<minPriority)) {
                MediaResourceLink = sLink; minPriority = priority; selHeight = height;
              }
            } else if ((height>selHeight) && (height<= maxHeight)) {
              MediaResourceLink = sLink; selHeight = height;
            
            } else if ((height>=selHeight) && (height<= maxHeight) && (itag in ([18,22,37,38,82,83,84,85]))) {
              // Если выоста такая же, но формат MP4 - то выбираем именно его (делаем приоритет MP4)
              MediaResourceLink = sLink; selHeight = height;
            }
        }
      }
      if (bAdaptive && (sAudio!='')) MediaResourceLink = '-i "'+Trim(MediaResourceLink)+'" -i "'+Trim(sAudio)+'"';
    
  }
  // Если есть субтитры и в дополнительных параметрах указано их показывать - загружаем 
  if (bSubtitles && (ttsUrl!='')) {
    string sTime1, sTime2; float nStart, nDur;
    sLink = ''; n = WordCount(ttsUrl, ',');
    for (i=1; i <= n; i++) {
      sData = ExtractWord(i, ttsUrl, ',');
      sType = HmsPercentDecode(ExtractParam(sData, 'lc', '', '&'));
      sVal  = HmsPercentDecode(ExtractParam(sData, 'u' , '', '&'));
      if (sType == 'en') sLink = sVal;
      if (sType == sSubtitlesLanguage) { sLink = sVal; break; }
    }
    if (sLink != '') {
      sData = HmsDownloadURL(sLink, sHeaders, true);
      sMsg  = ''; i = 0;
      RegEx = TRegExpr.Create('(<(text|p).*?</(text|p)>)', PCRE_SINGLELINE); // Convert to srt format
      try {
        if (RegEx.Search(sData)) do {
          if      (HmsRegExMatch('start="([\\d\\.]+)', RegEx.Match, sVal)) nStart = StrToFloat(ReplaceStr(sVal, '.', ','))*1000;
          else if (HmsRegExMatch('t="(\\d+)'         , RegEx.Match, sVal)) nStart = StrToFloat(sVal);
          if      (HmsRegExMatch('dur="([\\d\\.]+)'  , RegEx.Match, sVal)) nDur   = StrToFloat(ReplaceStr(sVal, '.', ','))*1000;
          else if (HmsRegExMatch('d="(\\d+)'         , RegEx.Match, sVal)) nDur   = StrToFloat(sVal);
          sTime1 = HmsTimeFormat(Int(nStart/1000))+','+RightCopy(Str(nStart), 3);
          sTime2 = HmsTimeFormat(Int((nStart+nDur)/1000))+','+RightCopy(Str(nStart+nDur), 3);
          sMsg += Format("%d\n%s --> %s\n%s\n\n", [i, sTime1, sTime2, HmsHtmlToText(HmsHtmlToText(RegEx.Match(0), 65001))]);
          i++;
        } while (RegEx.SearchAgain());
      } finally { RegEx.Free(); }
      sFile = HmsSubtitlesDirectory+'\\Youtube\\'+PodcastItem.ItemID+'.'+sSubtitlesLanguage+'.srt';
      HmsStringToFile(sMsg, sFile);
      PodcastItem[mpiSubtitleLanguage] = sFile;
    }
  }
}
