///////////////////////////////////////////////////////////////////////////////
// Получение ссылки с moonwalk.cc
void GetLink_Moonwalk(string sLink) {
  string sHtml, sData, sJsData, sPost, sVer, sQual, sVal, sServ, sVar, sUrlBase;
  int i; float f; TRegExpr RE; bool bHdsDump, bQualLog;
  TJsonObject JSON, OPTIONS, POSTDATA;
  
  string sHeaders = sLink+'\r\n'+
                    'Accept-Encoding: identity\r\n'+
                    'User-Agent: Mozilla/5.0 (Windows NT 6.1; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/63.0.3239.84 Safari/537.36\r\n';
  
  // Проверка установленных дополнительных параметров
  HmsRegExMatch('--quality=(\\w+)', mpPodcastParameters, sQual);
  bHdsDump = Pos('--hdsdump'      , mpPodcastParameters) > 0;
  bQualLog = Pos('--qualitylog'   , mpPodcastParameters) > 0;
  
  // Замена домена moonwalk.co и moonwalk.pw на moonwalk.cc
  HmsRegExReplace('(.*?moonwalk.)(co|pw)(.*)', sLink, '$1cc$3', sLink);
  sHtml = HmsDownloadURL(sLink, 'Referer: '+sHeaders);
  
  if (HmsRegExMatch('<iframe[^>]+src="(http.*?)"', sHtml, sLink)) {
    // Если внутри есть ссылка на iframe - загружаем его
    HmsRegExReplace('(.*?moonwalk.)(co|pw)(.*)', sLink, '$1cc$3', sLink);
    sHtml = HmsDownloadURL(sLink, 'Referer: '+sHeaders);
  }
  HmsRegExMatch2('(.*?//([^/]+))', sLink, sUrlBase, sServ); // Получаем UrlBase и домен
  sHeaders += 'Origin: '+sUrlBase+'\r\nX-Requested-With: XMLHttpRequest\r\n';

  JSON     = TJsonObject.Create();
  OPTIONS  = TJsonObject.Create();
  POSTDATA = TJsonObject.Create();
  try {

    // Ищем значения параметров (this.options)
    if (!HmsRegExMatch('VideoBalancer\\((.*?)\\);', sHtml, sData)) {
      HmsLogMessage(2, mpTitle+": Не найдены данные VideoBalancer в iframe."); 
      return;    
    }

    OPTIONS.LoadFromString(sData);

    // Если есть субтитры и их ещё локально не сохраняли - загружаем
    if ((OPTIONS.S["subtitles\\master_vtt"]!="") && (!FileExists(PodcastItem[mpiSubtitleLanguage]))) {
      sData = HmsUtf8Decode(HmsDownloadURL(OPTIONS.S["subtitles\\master_vtt"], "Referer: "+sHeaders));
      RE = TRegExpr.Create('(\\d{2}:\\d{2}:[\\d:,.\\s->]+.*?)(?=\n\\d{2}:d{2}:|\n\\d+\n|$)', PCRE_SINGLELINE);
      i = 1; sVal = ""; // Форматируем
      if (RE.Search(sData)) do { sVal += Format("%d\r\n%s\r\n\r\n", [i, RE.Match]); i++; } while (RE.SearchAgain);
      sLink = HmsSubtitlesDirectory+'\\'+PodcastItem.ItemID+'.srt';
      HmsStringToFile(sVal, sLink);             // Сохраняем субтитры локально
      PodcastItem[mpiSubtitleLanguage] = sLink; // Указываем новое расположение
    }

    // Получение ссылки на js-скрипт, где есть список параметров POST запроса
    if (!HmsRegExMatch('src="([^"]+/video-[^"]+.js)', sHtml, sVal)) {
      HmsLogMessage(2, mpTitle+": Не найдена ссылка на js-скрипт в iframe."); 
      return; 
    }      
    sJsData = HmsDownloadURL(HmsExpandLink(sVal, sUrlBase), 'Referer: '+sLink);
    // Устанавливаем дополнительные заголовки и их значения
    if (HmsRegExMatch('headers:({.*?})', sJsData, sVal)) {
      JSON.LoadFromString(sVal);
      for (i=0; i < JSON.Count; i++) {
        sVal = JSON.Values[i].AsString;
        if (HmsRegExMatch('this.options.(\\w+)', sVal, sVar)) sVal = OPTIONS.S[sVar];
        sHeaders += JSON.Names[i] + ": " + sVal + "\r\n";
      }
    }
    // Получаем параметры POST запроса
    if (!HmsRegExMatch('var\\s+\\w+=(\\{mw_key.*?\\})', sJsData, sData)) {
      HmsLogMessage(2, mpTitle+": Не найдены параметры для POST запроса."); 
      return; 
    }
    POSTDATA.LoadFromString(sData);
    // Формируем данные для POST
    sPost = "";
    for (i=0; i < POSTDATA.Count; i++) {
      sVal = POSTDATA.Values[i].AsString;
      if (HmsRegExMatch2('(this.options.(\\w+))', sVal, sVer, sVar)) sVal = ReplaceStr(sVal, sVer, OPTIONS.S[sVar]);
      if (HmsRegExMatch('\\w+\\.(\\w+)', sVal, sVar)) HmsRegExMatch('window\\.'+sVar+'\\s*=\\s*[\'"](.*?)[\'"]', sHtml, sVal);
      sPost += POSTDATA.Names[i] + "=" + sVal + "&";
    }
    // Get global variable
    if (HmsRegExMatch2("window\\['(\\w+)'\\]\\s*=\\s*'(\\w+)'", sHtml, sVar, sVal))
      if (HmsRegExMatch("\\w+\\.(\\w+)\\s*=\\s*\\w+\\[[\"']"+sVar, sJsData, sVar)) 
        sPost += sVar + "=" + sVal;

    sLink = "/manifests/video/"+OPTIONS.S["video_token"]+"/all";

    sData = HmsSendRequest(sServ, sLink, 'POST', 'application/x-www-form-urlencoded; Charset=UTF-8', sHeaders, sPost, 80, true);
    sData = ReplaceStr(HmsJsonDecode(sData), "\\r\\n", "");
    
  } finally { JSON.Free; OPTIONS.Free; POSTDATA.Free; }
  
  if (bHdsDump && HmsRegExMatch('"manifest_f4m"\\s*?:\\s*?"(.*?)"', sData, sLink)) {
    
    HDSLink(sLink, sQual);
    
  } else if (HmsRegExMatch('"manifest_m3u8"\\s*?:\\s*?"(.*?)"', sData, sLink)) {
    MediaResourceLink = ' ' + sLink;
    // Получение длительности видео, если она не установлена
    // ------------------------------------------------------------------------
    sData = HmsDownloadUrl(sLink, 'Referer: '+sHeaders, true);
    sVal  = Trim(PodcastItem.ItemOrigin[mpiTimeLength]);
    if ((sVal=='') || (RightCopy(sVal, 6)=='00.000')) {
      if (HmsRegExMatch('(http.*?)[\r\n$]', sData, sLink)) {
        sHtml = HmsDownloadUrl(sLink, 'Referer: '+sHeaders, true);
        RE = TRegExpr.Create('#EXTINF:(\\d+.\\d+)', PCRE_SINGLELINE); f=0;
        if (RE.Search(sHtml)) do f += StrToFloatDef(RE.Match(1), 0); while (RE.SearchAgain());
        RE.Free;
        if (f > 0) PodcastItem.ItemOrigin[mpiTimeLength] = HmsTimeFormat(Round(f))+'.000';
      }
    }
    // ------------------------------------------------------------------------
    
    // Если установлен ключ --quality или в настройках подкаста выставлен приоритет выбора качества
    // ------------------------------------------------------------------------
    string sSelectedQual = '', sMsg, sHeight; int iMinPriority = 99, iPriority; 
    if ((sQual!='') || (mpPodcastMediaFormats!='')) {
      TStringList QLIST = TStringList.Create();
      // Собираем список ссылок разного качества
      RE = TRegExpr.Create('#EXT-X-STREAM-INF:RESOLUTION=\\d+x(\\d+).*?[\r\n](http.+)$', PCRE_MULTILINE);
      if (RE.Search(sData)) do {
        sHeight = Format('%.5d', [StrToInt(RE.Match(1))]);
        sLink   = RE.Match(2);
        QLIST.Values[sHeight] = sLink;
        iPriority = HmsMediaFormatPriority(StrToInt(sHeight), mpPodcastMediaFormats);
        if ((iPriority >= 0) && (iPriority < iMinPriority)) {
          iMinPriority  = iPriority;
          sSelectedQual = sHeight;
        }
      } while (RE.SearchAgain());
      RE.Free;
      QLIST.Sort();
      if (QLIST.Count > 0) {
        if      (sQual=='low'   ) sSelectedQual = QLIST.Names[0];
        else if (sQual=='medium') sSelectedQual = QLIST.Names[Round((QLIST.Count-1) / 2)];
        else if (sQual=='high'  ) sSelectedQual = QLIST.Names[QLIST.Count - 1];
        else if (HmsRegExMatch('(\\d+)', sQual, sQual)) {
          extended minDiff = 999999; // search nearest quality
          for (i=0; i < QLIST.Count; i++) {
            extended diff = StrToInt(QLIST.Names[i]) - StrToInt(sQual);
            if (Abs(diff) < minDiff) {
              minDiff = Abs(diff);
              sSelectedQual = QLIST.Names[i];
            }
          }
        }
      }
      if (sSelectedQual != '') MediaResourceLink = ' ' + QLIST.Values[sSelectedQual];
      if (bQualLog) {
        sMsg = 'Доступное качество: ';
        for (i = 0; i < QLIST.Count; i++) {
          if (i>0) sMsg += ', ';
          sMsg += IntToStr(StrToInt(QLIST.Names[i])); // Обрезаем лидирующие нули
        }
        if (sSelectedQual != '') sSelectedQual = IntToStr(StrToInt(sSelectedQual));
        else sSelectedQual = 'Auto';
        sMsg += '. Выбрано: ' + sSelectedQual;
        HmsLogMessage(1, mpTitle+'. '+sMsg);
      }
      QLIST.Free;
    }
    // ------------------------------------------------------------------------
    
  } else if (HmsRegExMatch('"manifest_mp4"\\s*?:\\s*?"(.*?)"', sData, sLink)) {
    sData = HmsDownloadURL(sLink, 'Referer: '+sHeaders);
    // Поддержка установленных приоритетов качества в настройках подкаста
    JSON = TJsonObject.Create();
    int height, selHeight=0, minPriority=99, priority, maxHeight;
    if (sQual=='medium') sQual='480'; if (sQual=='low') sQual='360';
    maxHeight = StrToIntDef(sQual, 4320);
    try {
      JSON.LoadFromString(sData);
      for (i=0; i<JSON.Count; i++) {
        sVer  = JSON.Names[i];
        sLink = JSON.S[sVer];
        height = StrToIntDef(sVer, 0);
        if ((sQual!='') && (sQual==sVer)) { MediaResourceLink = sLink; selHeight = height; break; }
        if (mpPodcastMediaFormats!='') {
          priority = HmsMediaFormatPriority(height, mpPodcastMediaFormats);
          if ((priority>=0) && (priority<minPriority)) {
            MediaResourceLink = sLink; minPriority = priority;
          }
        } else if ((height > selHeight) && (height <= maxHeight)) {
          MediaResourceLink = sLink; selHeight = height;
        }
      }
    } finally { JSON.Free(); }
    
  } else {
    HmsLogMessage(2, mpTitle+': Ошибка получения данных от new_session.');
    if (DEBUG==1) {
      if (ServiceMode) sVal = SpecialFolderPath(0x37); // Общая папака "Видео"
        else           sVal = SpecialFolderPath(0);    // Рабочий стол
        sVal = IncludeTrailingBackslash(sVal)+'Moonwalk.log';
      HmsStringToFile(sHeaders+'\r\nsHtml:\r\n'+sHtml+'\r\nsPost:\r\n'+sPost+'\r\nsData:\r\n'+sData, sVal);
      HmsLogMessage(1, mpTitle+': Создан лог файл '+sVal);
    }
  }
} // Конец функции поулчения ссылки с moonwalk.cc
