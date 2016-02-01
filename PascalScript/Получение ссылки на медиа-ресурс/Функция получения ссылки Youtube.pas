// ---- Получение ссылки на Youtube -------------------------------------------
Function GetLink_Youtube31(sLink: String): Boolean;
Var
  sData, sVideoID, sMaxHeight, sSubtitlesLanguage:String;
  hlsUrl, ttsUrl, flp, jsUrl, dashMpdLink, streamMap, playerId, algorithm: String;
  sType, itag, sig, alg, s, sFile, sVal, sConfig, sHeaders: String ;
  UrlBase:String = ""; is3D, bSubtitles, bAdaptive: Boolean;
  i, n, w, num, height, priority, minPriority, selHeight, maxHeight: Integer;
  JSON: TJsonObject; RegEx: TRegExpr;
Begin 
  sSubtitlesLanguage :='ru'; minPriority := 90; maxHeight := 1080;
  sHeaders := 'Referer: '+sLink+#13#10+
              'User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/46.0.2490.86 Safari/537.36'+#13#10+
              'Origin: http://www.youtube.com'+#13#10;
  
  HmsRegExMatch('--maxheight=(\d+)'    , mpPodcastParameters, sMaxHeight);
  HmsRegExMatch('--sublanguage=(\w{2})', mpPodcastParameters, sSubtitlesLanguage);
  bSubtitles := (Pos('--subtitles'  , mpPodcastParameters)>0);  
  bAdaptive  := (Pos('--adaptive'   , mpPodcastParameters)>0);  
  TryStrToInt(sMaxHeight, maxHeight);

  If Not HmsRegExMatch('[\?&]v=([^&]+)'       , sLink, sVideoID) Then
         HmsRegExMatch('/(?:embed|v)/([^\?]+)', sLink, sVideoID);
  
  If sVideoID='' Then Begin
    sData := HmsDownloadURL(sLink, sHeaders, True);
    If Not HmsRegExMatch('youtube.com[^"''>]+v=([^&]+)', sData, sVideoID) Then
           HmsRegExMatch('/embed/([^\?]+)'             , sData, sVideoID);
  End;

  If sVideoID='' Then Begin HmsLogMessage(2, 'Невозможно получить Video ID в ссылке Youtube'); Exit; End;

  sLink := 'http://www.youtube.com/watch?v='+sVideoID+'&hl=ru&persist_hl=1&has_verified=1';
  
  sData := HmsUtf8Decode(HmsDownloadURL(sLink));
  sData := HmsRemoveLineBreaks(sData);
  HmsRegExMatch('player.config\s*?=\s*?({.*?});', sData, sConfig);
  
  JSON := TJsonObject.Create();
  Try
    JSON.LoadFromString(sConfig);
    hlsUrl      := HmsExpandLink(JSON.S['args\hlsvp' ], UrlBase);
    ttsUrl      := HmsExpandLink(JSON.S['args\ttsurl'], UrlBase);
    flp         := HmsExpandLink(JSON.S['url'        ], UrlBase);
    jsUrl       := HmsExpandLink(JSON.S['assets\js'  ], UrlBase);
    streamMap   := JSON.S['args\url_encoded_fmt_stream_map'];
    If bAdaptive AND JSON.B['args\adaptive_fmts'] Then 
      streamMap := JSON.S['args\adaptive_fmts'];
    If (streamMap='') AND (hlsUrl='') Then Begin
      HmsLogMessage(2, "Can not found stream map in player config");
      Exit;
    End;
  Finally 
    JSON.Free;
  End;
  If Copy(jsUrl, 1, 2)='//' Then jsUrl := 'http:'+Trim(jsUrl);
  HmsRegExMatch('/player-([\w_-]+)/', jsUrl, playerId);
  algorithm := HmsDownloadURL('https://hms.lostcut.net/youtube/getalgo.php?jsurl='+HmsHttpEncode(jsUrl));
  
  If hlsUrl<>'' Then Begin
    MediaResourceLink := ' '+hlsUrl;
    sData := HmsDownloadUrl(sLink, sHeaders, true);
    RegEx := TRegExpr.Create('BANDWIDTH=(\d+).*?RESOLUTION=(\d+)x(\d+).*?(http[^#]*)', PCRE_SINGLELINE);
    try 
      If RegEx.Search(sData) Then Repeat
        sLink  := ' ' + RegEx.Match(4);
        height := StrToIntDef(RegEx.Match(3), 0);
        If mpPodcastMediaFormats<>'' Then Begin
          priority := HmsMediaFormatPriority(height, mpPodcastMediaFormats);
          If (priority>=0) AND (priority>minPriority) Then Begin
            MediaResourceLink := sLink; minPriority := priority;
          End;
        End Else If ((height > selHeight) AND (height <= maxHeight)) Then Begin
          MediaResourceLink := sLink; selHeight := height;
        End;
      Until Not RegEx.SearchAgain();
    Finally 
      RegEx.Free();
    End;
  
  End Else If streamMap<>'' Then Begin
    i:=1; While (i<=Length(streamMap)) Do 
    Begin
      sData := Trim(ExtractStr(streamMap, ',', i));
      sType := HmsHttpDecode(ExtractParam(sData, 'type', '', '&'));
      itag  := ExtractParam(sData, 'itag'    , '', '&');
      is3D  := ExtractParam(sData, 'stereo3d', '', '&') = '1';
      sLink := '';
      If (Pos('url=', sData)>0) Then Begin
        sLink := ' ' + HmsHttpDecode(ExtractParam(sData, 'url', '', '&'));
        If (Pos('&signature=', sLink)=0) Then Begin
          sig := HmsHttpDecode(ExtractParam(sData, 'sig', '', '&'));    
          If (sig='') Then Begin
            sig := HmsHttpDecode(ExtractParam(sData, 's', '', '&'));
            For w:=1 To WordCount(algorithm, ' ') Do Begin
              alg := ExtractWord(w, algorithm, ' ');
              If (Length(alg)<1) Then continue;
              If (Length(alg)>1) Then TryStrToInt(Copy(alg, 2, 4), num);
              If (alg[1]='r') Then Begin s:=''; for n:=Length(sig) DownTo 1 Do s:=s+sig[n]; sig := s; End; // Reverse
              If (alg[1]='s') Then Begin sig := Copy(sig, num+1, Length(sig));                        End; // Clone
              If (alg[1]='w') Then Begin n := (num-Trunc(num/Length(sig)))+1; Swap(sig[1], sig[n]);   End; // Swap
            End;
          End;
          If sig<>'' Then sLink := sLink + '&signature=' + sig;
        End;
      End;
      If (sLink<>'') Then Begin
        If ((Pos('flv', sType)>0) Or (Pos('mp4', sType)>0)) Then Begin
          height := 0;
          If      (itag in ([13,17,160,36           ])) Then height := 144
          Else if (itag in ([5,83,133,242           ])) Then height := 240
          Else if (itag in ([6                      ])) Then height := 270
          Else if (itag in ([18,34,43,82,100,134,243])) Then height := 360
          Else if (itag in ([35,44,101,135,244,43   ])) Then height := 480
          Else if (itag in ([22,45,84,102,136,247   ])) Then height := 720
          Else if (itag in ([37,46,137,248          ])) Then height := 1080
          Else if (itag in ([264,271                ])) Then height := 1440
          Else if (itag in ([266                    ])) Then height := 2160
          Else if (itag in ([138,272                ])) Then height := 2304
          Else if (itag in ([38                     ])) Then height := 3072;
          If (mpPodcastMediaFormats<>'') Then Begin
            priority := HmsMediaFormatPriority(height, mpPodcastMediaFormats);
            If ((priority>=0) Or (priority<minPriority)) Then Begin
              MediaResourceLink := sLink; minPriority := priority; selHeight := height;
            End;
          End Else If ((height>selHeight) And (height<= maxHeight)) Then Begin
            MediaResourceLink := sLink; selHeight := height;
          End;
        End;
      End;
    End;
    
  End;
  // Если есть субтитры и в дополнительных параметрах указано их показывать - загружаем 
  If (bSubtitles AND (ttsUrl<>'')) Then Begin
    sFile := HmsSubtitlesDirectory+'\Youtube\'+PodcastItem.ItemID+'.'+sSubtitlesLanguage+'.srt';
    sLink := ttsUrl+'&fmt=srt&lang='; 
    If Not HmsDownloadURLToFile(sLink+sSubtitlesLanguage, sFile, 'Accept-Encoding: gzip, deflate') Then
           HmsDownloadURLToFile(sLink+'en'              , sFile, 'Accept-Encoding: gzip, deflate');
    PodcastItem[mpiSubtitleLanguage] := sFile;
  End;
End;