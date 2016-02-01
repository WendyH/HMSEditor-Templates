// ---- Получение ссылки на Youtube -------------------------------------------
bool GetLink_Youtube31(string sLink) {
  String sData, sVideoID='', sMaxHeight='', sSubtitlesLanguage='ru';
  String hlsUrl, ttsUrl, flp, jsUrl, dashMpdLink, streamMap, playerId, algorithm;
  String sType, itag, sig, alg, s, sFile, sVal, sConfig, sHeaders;
  String UrlBase = ""; bool is3D;
  int  i, n, w, num, height, priority, minPriority = 90, selHeight, maxHeight = 1080;
  TJsonObject JSON; TRegExpr RegEx;

  sHeaders = 'Referer: '+sLink+#13#10+
             'User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/46.0.2490.86 Safari/537.36'+#13#10+
             'Origin: http://www.youtube.com'+#13#10;
  
  HmsRegExMatch('--maxheight=(\\d+)'    , mpPodcastParameters, sMaxHeight);
  HmsRegExMatch('--sublanguage=(\\w{2})', mpPodcastParameters, sSubtitlesLanguage);
  bool bSubtitles = (Pos('--subtitles'  , mpPodcastParameters)>0);  
  bool bAdaptive  = (Pos('--adaptive'   , mpPodcastParameters)>0);  
  TryStrToInt(sMaxHeight, maxHeight);

  if (!HmsRegExMatch('[\\?&]v=([^&]+)'       , sLink, sVideoID))
       HmsRegExMatch('/(?:embed|v)/([^\\?]+)', sLink, sVideoID);
  
  if (sVideoID=='') {
    sData = HmsDownloadURL(sLink, sHeaders, True);
    if (!HmsRegExMatch('youtube.com[^"\'>]+v=([^&]+)', sData, sVideoID))
         HmsRegExMatch('/embed/([^\\?]+)'            , sData, sVideoID);
  }

  if (sVideoID=='') return VideoMessage('Невозможно получить Video ID в ссылке Youtube');

  sLink = 'http://www.youtube.com/watch?v='+sVideoID+'&hl=ru&persist_hl=1&has_verified=1';
  
  sData = HmsUtf8Decode(HmsDownloadURL(sLink));
  sData = HmsRemoveLineBreaks(sData);
  HmsRegExMatch('player.config\\s*?=\\s*?({.*?});', sData, sConfig);
  
  JSON = TJsonObject.Create();
  try {
    JSON.LoadFromString(sConfig);
    hlsUrl      = HmsExpandLink(JSON.S['args\\hlsvp' ], UrlBase);
    ttsUrl      = HmsExpandLink(JSON.S['args\\ttsurl'], UrlBase);
    flp         = HmsExpandLink(JSON.S['url'         ], UrlBase);
    jsUrl       = HmsExpandLink(JSON.S['assets\\js'  ], UrlBase);
    streamMap   = JSON.S['args\\url_encoded_fmt_stream_map'];
    if (bAdaptive && JSON.B['args\\adaptive_fmts']) 
      streamMap = JSON.S['args\\adaptive_fmts'];
    if ((streamMap=='') && (hlsUrl=='')) {
      HmsLogMessage(2, "Can not found stream map in player config");
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
        sLink = ' ' + RegEx.Match(4);
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
        if (sLink!='') {
          if ((Pos('flv', sType)>0) || (Pos('mp4', sType)>0)) {
            height = 0;
            if      (itag in ([13,17,160,36           ])) height = 144;
            else if (itag in ([5,83,133,242           ])) height = 240;
            else if (itag in ([6                      ])) height = 270;
            else if (itag in ([18,34,43,82,100,134,243])) height = 360;
            else if (itag in ([35,44,101,135,244,43   ])) height = 480;
            else if (itag in ([22,45,84,102,136,247   ])) height = 720;
            else if (itag in ([37,46,137,248          ])) height = 1080;
            else if (itag in ([264,271                ])) height = 1440;
            else if (itag in ([266                    ])) height = 2160;
            else if (itag in ([138,272                ])) height = 2304;
            else if (itag in ([38                     ])) height = 3072;
            if (mpPodcastMediaFormats!='') {
              priority = HmsMediaFormatPriority(height, mpPodcastMediaFormats);
              if ((priority>=0) || (priority<minPriority)) {
                MediaResourceLink = sLink; minPriority = priority; selHeight = height;
              }
            } else if ((height>selHeight) && (height<= maxHeight)) {
              MediaResourceLink = sLink; selHeight = height;
            }
          }
        }
      }    
    
  }
  // Если есть субтитры и в дополнительных параметрах указано их показывать - загружаем 
  if (bSubtitles && (ttsUrl!='')) {
    sFile = HmsSubtitlesDirectory+'\\Youtube\\'+PodcastItem.ItemID+'.'+sSubtitlesLanguage+'.srt';
    sLink = ttsUrl+'&fmt=srt&lang='; 
    if (!HmsDownloadURLToFile(sLink+sSubtitlesLanguage, sFile, 'Accept-Encoding: gzip, deflate')) {
      HmsDownloadURLToFile(sLink+'en'                 , sFile, 'Accept-Encoding: gzip, deflate');
    }
    PodcastItem[mpiSubtitleLanguage] = sFile;
  }
}