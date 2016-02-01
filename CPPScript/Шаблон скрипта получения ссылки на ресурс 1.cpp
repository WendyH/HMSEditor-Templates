string    gsUrlBase    = "http://site.com";  // База для относительных ссылок
int       gnTotalItems = 0;                  // Счётчик созданных элементов
TDateTime gStart       = Now;                // Время начала запуска скрипта
string    gsTime       = "01:40:00.000";     // Продолжительность видео
int       mpiCountry   = 10012; // Идентификаторы для хранения дополнительной
int       mpiTranslate = 10013; // информации в свойствах подкаста
int       mpiQuality   = 10014;

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

  MediaResourceLink = mpFilePath;
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