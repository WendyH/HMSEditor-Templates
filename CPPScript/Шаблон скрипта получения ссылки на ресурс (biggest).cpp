string    gsUrlBase    = "http://site.com"; // База для относительных ссылок
int       gnTotalItems = 0;                  // Счётчик созданных элементов
TDateTime gStart       = Now;                // Время начала запуска скрипта
string    gsTime       = "01:40:00.000";     // Продолжительность видео
int       mpiCountry   = 10012; // Идентификаторы для хранения дополнительной
int       mpiTranslate = 10013; // информации в свойствах подкаста
int       mpiQuality   = 10014;
int       mpiVideoMessage = 1001001;
string    gsPodcastName   = "site.com";
string    gsPreviewPrefix = 'site'; // Префикс кеша информационных картинок на сервере wonky.lostcut.net

///////////////////////////////////////////////////////////////////////////////
// Раскодирование строки Uppod плеера
string DecodeUppodTextHash(string sData) {
  variant char1, char2, hash, tab_a, tab_b; int i;

  hash = "0123456789WGXMHRUZID=NQVBLihbzaclmepsJxdftioYkngryTwuvihv7ec41D6GpBtXx3QJRiN5WwMf=ihngU08IuldVHosTmZz9kYL2bayE";

  // Проверяем, может не нужно раскодировать (json или ссылка)
  if ((Pos("{", sData)>0) || (LeftCopy(sData, 4)=="http")) return HmsUtf8Decode(sData);

  sData = ReplaceStr(sData, 'tQ3N', ''); // Убираем мусор
  sData = DecodeUppod_tr(sData, "r", "A");
  
  hash = ReplaceStr(hash, 'ih', '\n');
  if (RightCopy(sData, 1)=='!') {
    sData = LeftCopy(sData, Length(sData)-1);
    tab_a = ExtractWord(4, hash, '\n');
    tab_b = ExtractWord(3, hash, '\n');
  } else {
    tab_a = ExtractWord(2, hash, '\n');
    tab_b = ExtractWord(1, hash, '\n');
  }

  sData = ReplaceStr(sData, "\n", "");
  for (i=1; i<=Length(tab_a); i++) {
    char1 = Copy(tab_b, i, 1);
    char2 = Copy(tab_a, i, 1);
    sData = ReplaceStr(sData, char1, "___");
    sData = ReplaceStr(sData, char2, char1);
    sData = ReplaceStr(sData, "___", char2);
  }
  sData = HmsUtf8Decode(HmsBase64Decode(sData));
  sData = ReplaceStr(sData, "hthp:", "http:");
  return sData;
}

string DecodeUppod_tr(string sData, string ch1, string ch2) {
  string s = ""; int i, loc3, nLen;

  if ((Copy(sData, Length(sData)-1, 1)==ch1) && (Copy(sData, 3, 1)==ch2)) {
    nLen = Length(sData);
    for (i=nLen; i>0; i--) s += Copy(sData, i, 1);
    loc3 = Int(StrToIntDef(Copy(s, nLen-1, 2), 0)/2);
    s = Copy(s, 3, nLen-5); i = loc3;
    if (loc3 < Length(s)) {
      while (i < Length(s)) {
        s = LeftCopy(s, i) + Copy(s, i+2, 99999);
        i+= loc3;
      }
    }
    sData = s + "!";
  }
  return sData;
}

///////////////////////////////////////////////////////////////////////////////
// Раскодирование строки Uppod плеера
string DecodeUppodText(string sData) {
  char char1, char2; int i;
  variant Client_codec_a = ["s", "a", "L", "3", "2", "k", "c", "t", "p", "8", "D", "n", "5", "g", "N", "y", "4", "R", "Z", "M", "1", "H", "e", "B", "0", "="];
  variant Client_codec_b = ["Q", "d", "z", "W", "V", "T", "u", "7", "o", "6", "9", "x", "J", "Y", "X", "I", "b", "U", "m", "w", "v", "f", "l", "i", "G", "E"];

  sData = ReplaceStr(sData, 'tQ3N', ''); // Убираем мусор
  sData = ReplaceStr(sData, "\n", "");
  for (i=0; i<Length(Client_codec_a); i++) {
    char1 = Client_codec_b[i];
    char2 = Client_codec_a[i];
    sData = ReplaceStr(sData, char1, "___");
    sData = ReplaceStr(sData, char2, char1);
    sData = ReplaceStr(sData, "___", char2);
  }
  sData = HmsUtf8Decode(HmsBase64Decode(sData));
  return sData;
}

///////////////////////////////////////////////////////////////////////////////
// Создание информационной ссылки
void CreateInfoItem(string sName, string sVal) {
  THmsScriptMediaItem Item; sVal = Trim(sVal);
  if (sVal=="") return;
  Item = HmsCreateMediaItem('Info'+IntToStr(PodcastItem.ChildCount), PodcastItem.ItemID);
  Item[mpiTitle     ] = sName+': '+sVal;
  Item[mpiThumbnail ] = 'http://wonky.lostcut.net/vids/info.jpg';
  Item[mpiTimeLength] = 9;
  Item[mpiCreateDate] = VarToStr(IncTime(gStart,0,-gnTotalItems,0,0));
  gnTotalItems++;
}

///////////////////////////////////////////////////////////////////////////////
// Создание ссылки-ошибки
void CreateErrorItem(string sMsg) {
  THmsScriptMediaItem Item = HmsCreateMediaItem('http://wonky.lostcut.net/vids/podcasterror_hd.mp4', PodcastItem.ItemID);
  Item[mpiTitle     ] = sMsg;
  Item[mpiThumbnail ] = 'http://wonky.lostcut.net/icons/symbol-error.png';
}

///////////////////////////////////////////////////////////////////////////////
// Создание ссылки на видео
THmsScriptMediaItem CreateMediaItem(THmsScriptMediaItem Folder, string sTitle, string sLink='', string sGrp='') {
  THmsScriptMediaItem Item = HmsCreateMediaItem(sLink, Folder.ItemID, sGrp);
  Item[mpiTitle     ] = sTitle;
  Item[mpiCreateDate] = VarToStr(IncTime(gStart,0,-gnTotalItems,0,0));
  Item[mpiTimeLength] = gsTime;
  Item.CopyProperties(PodcastItem, [mpiThumbnail,mpiYear,mpiActor,mpiDirector,mpiProducer,mpiGenre]);
  gnTotalItems++;
  return Item;
}

///////////////////////////////////////////////////////////////////////////////
// Создание папки подкаста
THmsScriptMediaItem CreateFolder(THmsScriptMediaItem ParentFolder, string sName, string sLink, string sImg='') {
  if (sImg=='') sImg = mpThumbnail;
  THmsScriptMediaItem Item = ParentFolder.AddFolder(sLink); // Создаём папку с указанной ссылкой
  Item[mpiTitle     ] = sName; // Присваиваем наименование
  Item[mpiThumbnail ] = sImg;  // Картинка
  Item[mpiCreateDate] = DateTimeToStr(IncTime(gStart, 0, -gnTotalItems, 0, 0)); // Для обратной сортировки по дате создания
  gnTotalItems++;             // Увеличиваем счетчик созданных элементов
  return Item;                // Возвращаем созданный объект
}

///////////////////////////////////////////////////////////////////////////////
// Получение ссылки с vk.com
bool GetLink_VK(char sLink) {
  string sHtml, sVal, host, uid, vkid, vtag, max_hd, no_flv, res;
  string ResolutionList='0:240, 1:360, 2:480, 3:720', sQAval, sQSel;
  int i, n, iPriority=0, iMinPriority=99; bool bQualityLog;

  bQualityLog = (Pos('--qualitylog',   mpPodcastParameters)>0);
  sHtml = HmsDownloadURL(sLink, sLink, true);
  sHtml = ReplaceStr(sHtml, '\\', '');
  host  = ''; max_hd = '2';

  if (!HmsRegExMatch('vtag["\':=\\s]+([0-9a-z]+)', sHtml, vtag)) {
    if (HmsRegExMatch('(<div[^>]+video_ext_msg.*?</div>)', sHtml, sLink)) {
      sLink = HmsHtmlToText(sLink);
      HmsLogMessage(2, PodcastItem.ItemOrigin.ItemParent[mpiTitle]+': vk.com сообщает - '+sLink);

      char sFileMP3 = HmsTempDirectory+'\\sa.mp3';
      char sFileImg = HmsTempDirectory+'\\vkmsg_';
      sVal  = HmsHttpEncode('Vk.com сообщает:');
      sHtml = HmsHttpEncode(ReplaceStr(sLink, '\n', '|'));
      i = cfgTranscodingScreenHeight;
      n = cfgTranscodingScreenWidth;
      sLink = Format('http://wonky.lostcut.net/videomessage.php?h=%d&w=%d&captfont=AGFriquer_Bold&captsize=%d&fontsize=%d&caption=%s&msg=%s', [i, n, Round(i/6), Round(i/17), sVal, sHtml]);
      HmsDownloadURLToFile(sLink, sFileImg);
      for (i=1; i<=7; i++) CopyFile(sFileImg, sFileImg+Format('%.3d.jpg', [i]), false);
      try {
        if (!FileExists(sFileMP3)) HmsDownloadURLToFile('http://wonky.lostcut.net/mp3/sa.mp3', sFileMP3);
        sFileMP3 = '-i "'+sFileMP3+'" ';
      } except { sFileMP3 = ''; }
      MediaResourceLink = Format('%s-f image2 -r 1 -i "%s" -c:v libx264 -pix_fmt yuv420p ', [sFileMP3, sFileImg+'%03d.jpg']);
    } else {
      HmsLogMessage(2, mpTitle+': не удалось обработать ссылку на vk.com');
      MediaResourceLink = 'http://wonky.lostcut.net/vids/error_getlink.avi';
    }
    return true;
  }
  HmsRegExMatch('[^a-z]host[=:"\'\\s]+(.*?)["\'&;,]', sHtml, host  );
  HmsRegExMatch('[^a-z]uid[=:"\'\\s]+([0-9]+)',       sHtml, uid   );
  HmsRegExMatch('no_flv.*?(\\d)'       ,              sHtml, no_flv);
  HmsRegExMatch('(?>hd":"|hd=|video_max_hd.*?)(\\d)', sHtml, max_hd);
  HmsRegExMatch('[^a-z]vkid[=:"\'\\s]+([0-9]+)',      sHtml, vkid  );
  HmsRegExMatch(max_hd+':(\\d+)',            ResolutionList, res   );

  sQAval = 'Доступное качество: '; sQSel = '';
  HmsRegExMatch('--quality=(\\d+)', mpPodcastParameters, sQSel);

  // Если включен приоритет форматов, то ищем ссылку на более приоритетное качество
  if (bQualityLog || (mpPodcastMediaFormats!='')) for (i=StrToIntDef(max_hd, 3); i>=0; i--) {
    HmsRegExMatch(IntToStr(i)+':(\\d+)', ResolutionList, sVal);
    sQAval += sVal + '  ';
    if (sQSel != '') {
      if (StrToIntDef(res, 0)>StrToIntDef(sQSel, 0)) res = sVal;
    } else if (mpPodcastMediaFormats != '') {
      iPriority = HmsMediaFormatPriority(StrToIntDef(sVal, 0), mpPodcastMediaFormats);
      if ((iPriority>=0)&&(iPriority<iMinPriority)) {iMinPriority = iPriority; res=sVal;}
    }
  }
  if (bQualityLog) HmsLogMessage(1, mpTitle+': '+sQAval+'Выбрано: '+res);

  if (LeftCopy(uid, 1)!='u') uid = 'u' + Trim(uid);
  if (Trim(host)=='') HmsRegExMatch('ajax.preload.*?<img[^>]+src="(http://.*?/)', sHtml, host);

  if (uid=='0') MediaResourceLink = host+'assets/videos/'+vtag+''+vkid+'.vk.flv';
  else          MediaResourceLink = host + uid+'/videos/'+vtag+'.'+res+'.mp4';
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// Получение ссылки с Youtube
bool GetLink_Youtube31(string sLink) {
  string sData, sVideoID='', sMaxHeight='', sAudio='', sSubtitlesLanguage='ru',
         sSubtitlesUrl, sFile, sVal, sMsg, sConfig, sHeaders; 
  TJsonObject JSON; TRegExpr RegEx;

  sHeaders = 'Referer: '+sLink+#13#10+
             'User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/46.0.2490.86 Safari/537.36'+#13#10+
             'Origin: http://www.youtube.com'+#13#10;
  
  HmsRegExMatch('--maxheight=(\\d+)'    , mpPodcastParameters, sMaxHeight);
  HmsRegExMatch('--sublanguage=(\\w{2})', mpPodcastParameters, sSubtitlesLanguage);
  bool bSubtitles = (Pos('--subtitles'  , mpPodcastParameters)>0);  
  bool bAdaptive  = (Pos('--adaptive'   , mpPodcastParameters)>0);  
  bool bNotDE     = (Pos('notde=1'      , sLink)>0);  

  if (!HmsRegExMatch('[\\?&]v=([^&]+)'       , sLink, sVideoID))
       HmsRegExMatch('/(?:embed|v)/([^\\?]+)', sLink, sVideoID);

  if (sVideoID=='') return VideoMessage(gsPodcastName, 'Невозможно получить Video ID в ссылке Youtube');

  sLink = 'http://www.youtube.com/watch?v='+sVideoID+'&hl=ru&persist_hl=1&has_verified=1';
  
  sData = HmsUtf8Decode(HmsDownloadURL(sLink));
  sData = HmsRemoveLineBreaks(sData);
  HmsRegExMatch('player.config\\s*?=\\s*?({.*?});', sData, sConfig);
  
  if ((sConfig=='') && HmsRegExMatch('(<div[^>]+class="content".*?</div>)', sData, sVal)) {
    sVal = HmsHtmlToText(sVal);
    VideoMessage('Youtube сообщает:', sVal);
    return false;
  }
  
  String hlsUrl, ttsUrl, flp, jsUrl, dashMpdLink, streamMap, playerId, algorithm;
  String sType, itag, sig, alg, s;
  String UrlBase = "";
  int  i, n, w, num, height, priority, minPriority = 90, selHeight, maxHeight = 1080;
  bool is3D, adaptive;
  
  JSON = TJsonObject.Create();
  try {
    JSON.LoadFromString(sConfig);
    hlsUrl      = HmsExpandLink(JSON.S['args\\hlsvp' ], UrlBase);
    ttsUrl      = HmsExpandLink(JSON.S['args\\ttsurl'], UrlBase);
    flp         = HmsExpandLink(JSON.S['url'         ], UrlBase);
    jsUrl       = HmsExpandLink(JSON.S['assets\\js'  ], UrlBase);
    streamMap   = JSON.S['args\\url_encoded_fmt_stream_map'];
    if (adaptive && JSON.B['args\\adaptive_fmts']) 
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
    sLink = ttsUrl+'&format=srt&lang='; 
    if (!HmsDownloadURLToFile(sLink+sSubtitlesLanguage, sFile, 'Accept-Encoding: gzip, deflate')) {
      HmsDownloadURLToFile(sLink+'en'                 , sFile, 'Accept-Encoding: gzip, deflate');
    }
    PodcastItem[mpiSubtitleLanguage] = sFile;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Вывод видео сообщения с заданным текстом
void ShowVideoMessage(string sMsg, string sTitle='', int nErr=0, string sDescr='') {
  if (nErr==1) sMsg = '<c:#e22>'+Trim(sMsg); 
  TStrings INFO = TStringList.Create();
  INFO.Values['Title' ] = sTitle;
  INFO.Values['Info'  ] = sMsg;
  INFO.Values['Descr' ] = sDescr;
  PodcastItem[mpiVideoMessage] = INFO.Text;
  INFO.Free();
  VideoPreview();
}

///////////////////////////////////////////////////////////////////////////////
// Формирование видео с картинкой с информацией о фильме
bool VideoPreview() {
  string sVal, sFileImage, sPoster, sTitle, sDescr, sCateg, sInfo, sLink, sData;
  int xMargin=7, yMargin=10, nSeconds=10, n; string sCacheDir;
  float nH=cfgTranscodingScreenHeight, nW=cfgTranscodingScreenWidth;
  // Проверяем и, если указаны в параметрах подкаста, выставляем значения смещения от краёв
  if (HmsRegExMatch('--xmargin=(\\d+)', mpPodcastParameters, sVal)) xMargin=StrToInt(sVal);
  if (HmsRegExMatch('--ymargin=(\\d+)', mpPodcastParameters, sVal)) yMargin=StrToInt(sVal);
  sCacheDir = IncludeTrailingBackslash(HmsTempDirectory);

  if (Trim(PodcastItem[mpiVideoMessage])=='') return; // Если нет инфы - выходим быстро!
  TStrings INFO = TStringList.Create();       // Создаём объект TStrings
  INFO.Text  = PodcastItem[1001001];          // И загружаем туда информацию
  sPoster = INFO.Values['Poster'];            // Постер
  sTitle  = INFO.Values['Title' ];            // Самая верхняя надпись - Название
  sCateg  = INFO.Values['Genre' ];            // Жанр
  sInfo   = INFO.Values['Info'  ];            // Блок информации
  sDescr  = INFO.Values['Descr' ];            // Описание
  if (sTitle=='') sTitle = ' ';
  ForceDirectories(sCacheDir);
  sFileImage = ExtractShortPathName(sCacheDir)+'videopreview_'; // Файл-заготовка для сохранения картинки
  sDescr = Copy(sDescr, 1, 3000); // Если блок описания получился слишком большой - обрезаем

  INFO.Text = ""; // Очищаем объект TStrings для формирования параметров запроса
  INFO.Values['prfx' ] = gsPreviewPrefix;  // Префикс кеша сформированных картинок на сервере
  INFO.Values['title'] = sTitle;           // Блок - Название
  INFO.Values['info' ] = sInfo;            // Блок - Информация
  INFO.Values['categ'] = sCateg;           // Блок - Жанр/категории
  INFO.Values['descr'] = sDescr;           // Блок - Описание фильма
  INFO.Values['mlinfo'] = '20';            // Максимальное число срок блока Info
  INFO.Values['w' ] = IntToStr(Round(nW)); // Ширина кадра
  INFO.Values['h' ] = IntToStr(Round(nH)); // Высота кадра
  INFO.Values['xm'] = IntToStr(xMargin);   // Отступ от краёв слева/справа
  INFO.Values['ym'] = IntToStr(yMargin);   // Отступ от краёв сверху/снизу
  INFO.Values['bg'] = 'http://www.pageresource.com/wallpapers/wallpaper/noir-blue-dark_3512158.jpg'; // Катринка фона (кэшируется на сервере) 
  INFO.Values['fx'] = '3'; // Номер эффекта для фона: 0-нет, 1-Blur, 2-more Blur, 3-motion Blur, 4-radial Blur
  INFO.Values['fztitle'] = IntToStr(Round(nH/14)); // Размер шрифта блока названия (тут относительно высоты кадра)
  INFO.Values['fzinfo' ] = IntToStr(Round(nH/22)); // Размер шрифта блока информации
  INFO.Values['fzcateg'] = IntToStr(Round(nH/26)); // Размер шрифта блока жанра/категории
  INFO.Values['fzdescr'] = IntToStr(Round(nH/18)); // Размер шрифта блока описания
  // Если текста описания больше чем нужно - немного уменьшаем шрифт блока
  if (Length(sDescr)>890) INFO.Values['fzdescr'] = IntToStr(Round(nH/20));
  // Если есть постер, задаём его параметры отображения (где, каким размером)
  if (sPoster!='') {
    INFO.Values['wpic'  ] = IntToStr(Round(nW/4)); // Ширина постера (1/4 ширины кадра)
    INFO.Values['xpic'  ] = '10';    // x-координата постера
    INFO.Values['ypic'  ] = '10';    // y-координата постера
    if (mpFilePath=='InfoUpdate') {
      INFO.Values['wpic'  ] = IntToStr(Round(nW/6)); // Ширина постера (1/6 ширины кадра)
      INFO.Values['xpic'  ] = IntToStr(Round(nW/2 - nW/12)); // центрируем
    }
    INFO.Values['urlpic'] = sPoster; // Адрес изображения постера
  }
  sData = '';  // Из установленных параметров формируем строку POST запроса
  for (n=0; n<INFO.Count; n++) sData += '&'+Trim(INFO.Names[n])+'='+HmsHttpEncode(INFO.Values[INFO.Names[n]]);
  INFO.Free(); // Освобождаем объект из памяти, теперь он нам не нужен
  // Делаем POST запрос не сервер формирования картинки с информацией
  sLink = HmsSendRequestEx('wonky.lostcut.net', '/videopreview.php?p='+gsPreviewPrefix, 'POST', 
               'application/x-www-form-urlencoded', '', sData, 80, 0, '', true);
  // В ответе должна быть ссылка на сформированную картинку
  if (LeftCopy(sLink, 4)!='http') {HmsLogMessage(2, 'Ошибка получения файла информации videopreview.'); return;}
  // Сохраняем сформированную картинку с информацией в файл на диске
  HmsDownloadURLToFile(sLink, sFileImage);
  // Копируем и нумеруем файл картики столько раз, сколько секунд мы будем её показывать
  for (n=1; n<=nSeconds; n++) CopyFile(sFileImage, sFileImage+Format('%.3d.jpg', [n]), false);
  // Для некоторых телевизоров (Samsung) видео без звука отказывается проигрывать, поэтому скачиваем звук тишины
  char sFileMP3 = ExtractShortPathName(HmsTempDirectory)+'\\silent.mp3';
  try {
    if (!FileExists(sFileMP3)) HmsDownloadURLToFile('http://wonky.lostcut.net/mp3/silent.mp3', sFileMP3);
    sFileMP3 = '-i "'+sFileMP3+'"';
  } except { sFileMP3=''; }
  // Формируем из файлов пронумерованных картинок и звукового команду для формирования видео
  MediaResourceLink = Format('%s -f image2 -r 1 -i "%s" -c:v libx264 -pix_fmt yuv420p ', [sFileMP3, sFileImage+'%03d.jpg']);
}


///////////////////////////////////////////////////////////////////////////////
// Вывод вместо видео заданного сообщения
bool VideoMessage(char sCaption, char sMessage, int nTime=30) {
  char sFileImage = HmsTempDirectory+'\\videomessage.jpg'; char sCmd;
  sCaption = HmsHttpEncode(ReplaceStr(sCaption, '\n', '|'));
  sMessage = HmsHttpEncode(ReplaceStr(sMessage, '\n', '|'));
  HmsDownloadURLToFile('http://wonky.lostcut.net/videomessage.php?testpic=1&caption='+sCaption+'&msg='+sMessage, sFileImage);
  char sFileMP3 = HmsTempDirectory+'\\silent.mp3';
  try {
    if (!FileExists(sFileMP3)) HmsDownloadURLToFile('http://wonky.lostcut.net/mp3/silent.mp3', sFileMP3);
    sFileMP3 = '-i "'+sFileMP3+'"';
  } except {
    sFileMP3 = '';
  }
  sCmd = Format('%s -loop 1 -f image2 -i "%s" -t %d -r 25 ', [ExtractShortPathName(sFileMP3), ExtractShortPathName(sFileImage), 7]);
  MediaResourceLink = sCmd;
}

///////////////////////////////////////////////////////////////////////////////
// Вывод видео сообщения с информацией о фильме
void ShowVideoInfo() {
  string sInfo; THmsScriptMediaItem Parent = PodcastItem.ItemParent;

  sInfo = '';
  if (Trim(Parent[mpiCountry  ])!='') sInfo += 'Страна: '  +Parent[mpiCountry  ]+"|";
  if (Trim(Parent[mpiTranslate])!='') sInfo += 'Перевод: ' +Parent[mpiTranslate]+"|";
  if (Trim(Parent[mpiQuality  ])!='') sInfo += 'Качество: '+Parent[mpiQuality  ]+"|";
  if (Trim(Parent[mpiDirector ])!='') sInfo += 'Режиссер: '+Parent[mpiDirector ]+"|";
  if (Trim(Parent[mpiActor    ])!='') sInfo += 'В ролях: ' +Parent[mpiActor    ]+"|";
  sInfo = Copy(sInfo, 1, Length(sInfo)-1); // Обрезаем последний символ "|"
  TStrings INFO = TStringList.Create();
  INFO.Values['Poster'] = Parent[mpiThumbnail];
  INFO.Values['Title' ] = Parent[mpiTitle];
  INFO.Values['Genre' ] = Parent[mpiGenre];
  INFO.Values['Info'  ] = sInfo;
  INFO.Values['Descr' ] = ReplaceStr(Parent[mpiComment], "\n", "|");
  PodcastItem[mpiVideoMessage] = INFO.Text;
  INFO.Free();
  VideoPreview();
}

///////////////////////////////////////////////////////////////////////////////
/// Создание ссылок на файл(ы) по переданной ссылке (шаблону) -------------
void CreateVideoLink(THmsScriptMediaItem Folder, string sName, string sLink, bool bSeparateInFolders=false) {
  string sCut, sQualArray, sQual, sFile; int i, nCount; // Объявляем переменные

  // Проверяем, есть ли в переданной ссылке шаблон с массивом существующего качества "[720,480,360]"
  if (HmsRegExMatch('\\[(.*?)\\]', sLink, sQualArray)) {
    sCut   = '['+sQualArray+']';                   // Та часть, которая будет заменятся на индификатор качества
    nCount = WordCount (sQualArray, ',');          // Количество елементов, разделённых запятой
    for (i=1; i<=nCount; i++) {
      sQual = ExtractWord(i, sQualArray, ',');     // Получаем очередной индификатор качества
      if (sQual=='') continue;                     // Может быть пропущен, если не указан
      sFile = ReplaceStr(sLink, sCut, sQual);      // Формируем ссылку на файл, заменяя шаблон на индификатор качества
      if (bSeparateInFolders) {                    // Если был передан флаг "Группировать файлы качества по разным папкам",
        CreateMediaItem(Folder, sName, sFile, sQual); // то передаём индификатор качества как имя группы, где будет создана ссылка
      } else {                                     
        if (sName=='') HmsRegExMatch('.*/(.*)', sLink, sName); // Получаем имя файла из ссылки (всё что идёт после последнего слеша)
        sName = ReplaceStr(sName, sCut, '');          // Убираем перечисление качества из имени
        sName = ReplaceStr(sName, '_', '');           // А также подчекривания (лишние)
        CreateMediaItem(Folder, sQual+' '+sName, sFile); // Добавляем индификатор качества к началу имени и создаём ссылку
      }
    }

  } else {
    // Если шаблона выбора качества в ссылке нет, то просто создаём ссылку
    if (sName=='') HmsRegExMatch('.*/(.*)', sLink, sName); // Если имя пустое, получаем имя файла из ссылки (всё что идёт после последнего слеша)
    CreateMediaItem(Folder, sName, sLink);                    

  }
}

///////////////////////////////////////////////////////////////////////////////
// Создание серий из плейлиста
void CreateSeriesFromPlaylist(THmsScriptMediaItem Folder, string sLink, string sName='') {
  string sData, s1, s2, s3; int i; TJsonObject JSON, PLITEM; TJsonArray PLAYLIST; // Объявляем переменные

  // Если передано имя плейлиста, то создаём папку, в которой будем создавать элементы
  if (Trim(sName)!='') Folder = Folder.AddFolder(sName);          
  
  // Если в переменной sLink сожержится знак '{', то там не ссылка, а сами данные Json
  if (Pos('{', sLink)>0) {
    sData = sLink;
  } else {
    sData = HmsDownloadURL(sLink, "Referer: "+mpFilePath, true);  // Загружаем плейлист
    sData = HmsUtf8Decode(sData);
    sData = DecodeUppodTextHash(sData);                           // Дешифруем
  }  

  JSON  = TJsonObject.Create();                 // Создаём объект для работы с Json
  try {
    JSON.LoadFromString(sData);                 // Загружаем json данные в объект
    PLAYLIST = JSON.A['playlist'];              // Пытаемся получить array с именем 'playlist'
    if (PLAYLIST==nil) PLAYLIST = JSON.AsArray; // Если массив 'playlist' получить не получилось, то представляем все наши данные как массив
    if (PLAYLIST!=nil) {                        // Если получили массив, то запускаем обход всех элементов в цикле
      for (i=0; i<PLAYLIST.Length; i++) {
        PLITEM = PLAYLIST[i];                   // Получаем текущий элемент массива
        sName = PLITEM.S['comment'];            // Название - значение поля comment
        sLink = PLITEM.S['file'   ];            // Получаем значение ссылки на файл
        sName = ReplaceStr(ReplaceStr(HmsHtmlToText(sName), "\n", ' '), "\r", '');
        // Форматируем числовое представление серий в названии
        // Если в названии есть число, то будет в s1 - то, что стояло перед ним, s2 - само число, s3 - то, что было после числа
        if (HmsRegExMatch3('^(.*?)(\\d+)(.*)$', sName, s1, s2, s3)) 
          sName = Trim(Format('%s %.2d %s', [s1, StrToInt(s2), s3])); // Форматируем имя - делаем число двухцифровое (01, 02...)
        if (LeftCopy(sLink, 4)=='oid=') sLink = 'http://vk.com/video_ext.php?'+Trim(sLink);

        // Проверяем, если это вложенный плейлист - запускаем создание элементов из этого плейлиста рекурсивно
        if (PLITEM.B['playlist']) 
          CreateSeriesFromPlaylist(Folder, PLITEM.S['playlist'], sName);
        else 
          CreateVideoLink(Folder, sName, sLink, true); // Иначе просто создаём ссылки на видео
      }
    } // end if (PLAYLIST!=nil) 
  
  } finally { JSON.Free; }                      // Какие бы ошибки не случились, освобождаем объект из памяти
}

///////////////////////////////////////////////////////////////////////////////
// Конвертация продолжительности из формата youtube (ISO8601) в формат HMS
string ConvertYoutubeTime(string sTime) {
  string sVal; int nSeconds = 0;
  if (HmsRegExMatch('(\\d+)H', sTime, sVal)) nSeconds += StrToInt(sVal)*3600;  
  if (HmsRegExMatch('(\\d+)M', sTime, sVal)) nSeconds += StrToInt(sVal)*60;  
  if (HmsRegExMatch('(\\d+)S', sTime, sVal)) nSeconds += StrToInt(sVal);
  if (nSeconds==0) nSeconds = 600;
  return HmsTimeFormat(nSeconds)+'.000';
}

///////////////////////////////////////////////////////////////////////////////
// Создание папок сезонов
void CreateSeasons(string sHtml) {
  string sData, sLink, sName, sTime, sImg, sVal, s1, s2, s3, sSeason;
  THmsScriptMediaItem Item; TRegExpr RegEx; int n;
  
  HmsRegExMatch('<seasons>(.*?)</seasons>', sHtml, sData);
  RegEx = TRegExpr.Create('(<a.*?</a>)', PCRE_SINGLELINE);
  n = 0;
  try {
    if (RegEx.Search(sHtml)) do {
      sLink=''; sName='';
      HmsRegExMatch('(<a.*?</a>)'        , RegEx.Match, sName);
      HmsRegExMatch('<a[^>]+href="(.*?)"', RegEx.Match, sLink);
      if (Trim(sLink)=="") continue;
      n++;
      
      sName = ReplaceStr(HmsHtmlToText(sName), "/", "-");
      sLink = HmsExpandLink(sLink, gsUrlBase);

      // Форматируем номера сезонов в двуцифровой формат
      if (HmsRegExMatch3('^(.*?)(\\d+)(.*)', sName, s1, s2, s3))
        sName = Trim(Format('%s %.2d %s', [s1, StrToInt(s2), s3]));
      
      sLink += '&season='+IntToStr(n);
      CreateFolder(PodcastItem, sName, sLink); // Создание папки сезона

    } while (RegEx.SearchAgain);
  } finally { RegEx.Free(); }
}

///////////////////////////////////////////////////////////////////////////////
// Создание ссылок видео
void CreateLinks() {
  string sHtml, sData, sLink, sName, sTime, sImg, sVal, sSeason, sID;
  THmsScriptMediaItem Item; TRegExpr RegEx;
  
  sSeason = '';
  HmsRegExMatch('season=(\\d+)', mpFilePath, sSeason);
  
  sHtml = HmsDownloadURL(mpFilePath, 'Referer: '+mpFilePath, true);
  sHtml = HmsUtf8Decode(sHtml);
  sHtml = HmsRemoveLineBreaks(sHtml);
  
  // =========================================================================
  // Сбор информации о фильме
  if (HmsRegExMatch('Год выпуска.*?(\\d{4})', sHtml, sVal)) PodcastItem[mpiYear] = sVal;
  if (HmsRegExMatch('Продолжительность.*?(\\d{2}:\\d{2}:\\d{2})', sHtml, sVal)) gsTime = sVal+'.000';
  if (HmsRegExMatch('Жанр:(.*?)<br'    , sHtml, sVal)) PodcastItem[mpiGenre    ] = HmsHtmlToText(sVal);
  if (HmsRegExMatch('Страна:(.*?)<br'  , sHtml, sVal)) PodcastItem[mpiCountry  ] = HmsHtmlToText(sVal);
  if (HmsRegExMatch('Перевод:(.*?)<br' , sHtml, sVal)) PodcastItem[mpiTranslate] = HmsHtmlToText(sVal);
  if (HmsRegExMatch('Качество:(.*?)<br', sHtml, sVal)) PodcastItem[mpiQuality  ] = HmsHtmlToText(sVal);
  if (HmsRegExMatch('Режиссер:(.*?)<br', sHtml, sVal)) PodcastItem[mpiDirector ] = HmsHtmlToText(sVal);
  if (HmsRegExMatch('В ролях:...(.*?)<', sHtml, sVal)) PodcastItem[mpiActor    ] = HmsHtmlToText(sVal);
  if (HmsRegExMatch('(<div id="news-id-.*?)<br', sHtml, sVal)) PodcastItem[mpiComment] = HmsHtmlToText(sVal);
  // =========================================================================
  

  HmsRegExMatch("Base64.decode\\('(.*?)'", sHtml, sVal);
  sData = HmsRemoveLineBreaks(HmsUtf8Decode(HmsBase64Decode(sVal)));

  // Если на странице есть перечисление сезонов и в данный момент мы не в конкретном
  if ((sSeason=='') && HmsRegExMatch('<Все сезоны сериала>', sHtml, sLink)) {
    // Создаём список сезонов
    CreateSeasons(sHtml);
  
  } else if (HmsRegExMatch('flashvars[^>]+file=(.*?)[&"]', sData, sLink)) {
    // Создаём ссылку на конкретный фильм
    sLink = DecodeUppodTextHash(sLink);
    CreateMediaItem(PodcastItem, mpTitle, sLink);
    
  } else if (HmsRegExMatch('flashvars[^>]+;pl=(.*?)[&"]', sData, sLink)) {
    // Создаём ссылки на серии из плейлиста
    sLink = DecodeUppodTextHash(sLink);
    sData = HmsUtf8Decode(HmsDownloadURL(sLink, 'Referer: '+mpFilePath, true));
    sData = DecodeUppodTextHash(sData);
    HmsRegExMatch('(\\{.*\\})', sData, sData);
    CreateSeriesFromPlaylist(PodcastItem, sData);
    
  } else if (HmsRegExMatch('vkArr=(\\[.*?\\]);', sHtml, sVal)) {
    // Создаём ссылки на серии из плейлиста в переменной vkArr
    CreateSeriesFromPlaylist(PodcastItem, sVal);
    
  } else {
    
    CreateErrorItem('Не удалось найти ссылку на фильм на странице сайта.');
    
  }
  
  
  // Если на странице есть ссылка на трейлер - создаём такую ссылку
  if (HmsRegExMatch('youtube.com/watch\\?v=(.*?)[&"]', sHtml, sID)) {
    Item  = CreateMediaItem(PodcastItem, 'Трейлер', 'http://www.youtube.com/watch?v='+sID);
    Item[mpiThumbnail] = 'http://img.youtube.com/vi/'+sID+'/0.jpg';
    // Получение реальной длительности youtube ролика
    sData = HmsDownloadURL('https://www.googleapis.com/youtube/v3/videos?key=AIzaSyDY7NostuASg47evFG1OZvwbZiysnYsAwc&part=contentDetails&id='+sID);
    HmsRegExMatch('"duration":\\s*?"(.*?)"', sData, sVal);
    Item[mpiTimeLength] = ConvertYoutubeTime(sVal);
  }
  
  // Если установлен ключ отображения информационных ссылок - добавляем их
  if (Pos('--addinfoitems', mpPodcastParameters)>0) {
    if (Trim(PodcastItem[mpiGenre    ])!='') CreateInfoItem('Жанр'    , PodcastItem[mpiGenre    ]);
    if (Trim(PodcastItem[mpiCountry  ])!='') CreateInfoItem('Страна'  , PodcastItem[mpiCountry  ]);
    if (Trim(PodcastItem[mpiTranslate])!='') CreateInfoItem('Перевод' , PodcastItem[mpiTranslate]);
    if (Trim(PodcastItem[mpiQuality  ])!='') CreateInfoItem('Качество', PodcastItem[mpiQuality  ]);
    if (Trim(PodcastItem[mpiDirector ])!='') CreateInfoItem('Режиссер', PodcastItem[mpiDirector ]);
  }
  
}

///////////////////////////////////////////////////////////////////////////////
// Получение ссылки на медиаресурс в переменную MediaResourceLink
void GetLink() {
  if      (LeftCopy(mpFilePath, 4)=='Info') ShowVideoInfo();
  else if (LeftCopy(mpFilePath, 3)=='Err' ) VideoMessage(gsPodcastName, mpTitle);
  else if (HmsRegExMatch('youtube', mpFilePath, '')) GetLink_YouTube31(mpFilePath);
  else if (HmsRegExMatch('vk.com' , mpFilePath, '')) GetLink_VK(mpFilePath);
  else MediaResourceLink = mpFilePath;
}


///////////////////////////////////////////////////////////////////////////////
//                     Г Л А В Н А Я   П Р О Ц Е Д У Р А                     //
{
  // Проверяем, при каком событии было вызвано выполнение скрипта (не папка ли это)
  if (PodcastItem.IsFolder) {
    
    PodcastItem.DeleteChildItems();

    CreateLinks(); // Это зашли в папку подкаста (фильма) - создаём ссылки

  } else {
    
    GetLink(); // Это запустили фильм - получаем ссылку на медиа-поток
    
    // Если в MediaResourceLink пусто, значит получить ссылку не получилось - выводим видео об ошибке
    if (MediaResourceLink=='') MediaResourceLink = 'http://wonky.lostcut.net/vids/podcasterror_hd.mp4';
  }

}