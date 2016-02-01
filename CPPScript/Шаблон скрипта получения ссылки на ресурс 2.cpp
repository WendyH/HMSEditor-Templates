string    gsUrlBase    = "http://site.com";  // База для относительных ссылок
int       gnTotalItems = 0;                  // Счётчик созданных элементов
TDateTime gStart       = Now;                // Время начала запуска скрипта
string    gsTime       = "01:40:00.000";     // Продолжительность видео
int       mpiCountry   = 10012; // Идентификаторы для хранения дополнительной
int       mpiTranslate = 10013; // информации в свойствах подкаста
int       mpiQuality   = 10014;
int       mpiVideoMessage = 1001001;
string    gsPodcastName   = "New Podcast";
string    gsPreviewPrefix = ''; // Префикс кеша информационных картинок на сервере wonky.lostcut.net

///////////////////////////////////////////////////////////////////////////////
// Создание информационной ссылки
void CreateInfoItem(string sName, string sVal) {
  THmsScriptMediaItem Item; sVal = Trim(sVal);
  if (sVal=="") return;
  Item = HmsCreateMediaItem('Info'+IntToStr(PodcastItem.ChildCount), PodcastItem.ItemID);
  Item[mpiTitle     ] = sName+': '+sVal;
  Item[mpiThumbnail ] = 'http://wonky.lostcut.net/vids/info.jpg';
  Item[mpiTimeLength] = 7;
  Item[mpiCreateDate] = VarToStr(IncTime(gStart,0,-gnTotalItems,0,0));
  gnTotalItems++;
}

///////////////////////////////////////////////////////////////////////////////
// Создание ссылки-ошибки
void CreateErrorItem(string sMsg) {
  THmsScriptMediaItem Item = HmsCreateMediaItem('Err'+IntToStr(PodcastItem.ChildCount), PodcastItem.ItemID);
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
  if (Trim(Parent[mpiCountry  ])!='') sInfo += 'Страна: '  +Parent[mpiCountry  ]+"\r\n";
  if (Trim(Parent[mpiTranslate])!='') sInfo += 'Перевод: ' +Parent[mpiTranslate]+"\r\n";
  if (Trim(Parent[mpiQuality  ])!='') sInfo += 'Качество: '+Parent[mpiQuality  ]+"\r\n";
  if (Trim(Parent[mpiDirector ])!='') sInfo += 'Режиссер: '+Parent[mpiDirector ]+"\r\n";
  if (Trim(Parent[mpiActor    ])!='') sInfo += 'В ролях: ' +Parent[mpiActor    ]+"\r\n";
  
  TStrings INFO = TStringList.Create();
  INFO.Values['Poster'] = Parent[mpiThumbnail];
  INFO.Values['Title' ] = Parent[mpiTitle];
  INFO.Values['Genre' ] = Parent[mpiGenre];
  INFO.Values['Info'  ] = sInfo;
  INFO.Values['Descr' ] = Parent[mpiComment];
  PodcastItem[mpiVideoMessage] = INFO.Text;
  INFO.Free();
  VideoPreview();
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
  string sHtml, sData, sLink, sName, sTime, sImg, sVal, sSeason;
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
  if (HmsRegExMatch('В ролях:..(.*?)<' , sHtml, sVal)) PodcastItem[mpiActor    ] = HmsHtmlToText(sVal);
  if (HmsRegExMatch('(<div id="news-id-.*?)<br', sHtml, sVal)) PodcastItem[mpiComment] = HmsHtmlToText(sVal);
  // =========================================================================
  

  // Если на странице есть перечисление сезонов и в данный момент мы не в конкретном
  if ((sSeason=='') && HmsRegExMatch('Все сезоны сериала', sHtml, sLink)) {
    // Создаём список сезонов
    CreateSeasons(sHtml);
  
  } else {
    // Создаём ссылку на конкретный фильм
    HmsRegExMatch("Base64.decode\\('(.*?)'", sHtml, sVal);
    sData = HmsUtf8Decode(HmsBase64Decode(sVal));
    HmsRegExMatch('flashvars[^>]+file=(.*?)[&"]', sData, sLink);
    CreateMediaItem(PodcastItem, mpTitle, sLink);
    
  }
  
  
  // Если на странице есть ссылка на трейлер - создаём такую ссылку
  if (HmsRegExMatch('flashvars[^>]+Трейлер[^>]+file=(.*?)[&"]', sHtml, sLink)) {
    CreateMediaItem(PodcastItem, 'Трейлер', sLink);
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