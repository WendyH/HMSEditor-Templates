string    gsUrlBase    = 'http://site.com'; // База для относительных ссылок
int       gnTotalItems = 0;                 // Счётчик созданных элементов
TDateTime gStart       = Now;               // Время начала запуска скрипта
int gnMaxPages=10, gnMaxInGroup=100; bool gbYearInTitle=false; char gsGroupMode='';

// Регулярные выражения для поиска на странице блоков с информацией о видео
string
  gsPatternBlock  = '<section>(.*?)</section>', // Искомые блоки
  gsCutPage       = '<fromCut>(.*?)<toCut>'   , // Обрезка загруженной страницы
  gsPatternTitle  = '(<a.*?</a>)',                 // Название
  gsPatternLink   = '<a[^>]+href=[\'"](.*?)[\'"]', // Ссылка
  gsPatternImg    = '<img[^>]+src="(.*?)"'       , // Картинка
  gsPatternYear   = 'shortinfo.*?>(\\d{4})<',      // Год
  // Регулярное выражение для поиска максимального номера страницы для дозагрузки
  gsPatternPages  = 'pagination.*page/(\\d+)/">\\d+</a>',
  gsPagesParams   = 'page/<PN>/', // Параметр с номером страницы, который добавляется к ссылке
  ;

///////////////////////////////////////////////////////////////////////////////
// Создание папки или подкаста
THmsScriptMediaItem CreateFolder(THmsScriptMediaItem ParentFolder, string sName, string sLink, string sImg='') {
  THmsScriptMediaItem Item = ParentFolder.AddFolder(sLink); // Создаём папку с указанной ссылкой
  Item[mpiTitle     ] = sName; // Присваиваем наименование
  Item[mpiThumbnail ] = sImg;  // Картинка
  Item[mpiCreateDate] = DateTimeToStr(IncTime(gStart, 0, -gnTotalItems, 0, 0)); // Для обратной сортировки по дате создания

  gnTotalItems++;             // Увеличиваем счетчик созданных элементов
  return Item;                // Возвращаем созданный объект
}

///////////////////////////////////////////////////////////////////////////////
// Получение имени группировки по имени видео (первая буква, "A..Z" или "#")
string GetGroupName(string sName) {
  string sGrp = '#';
  if (HmsRegExMatch('([A-ZА-Я0-9])', sName, sGrp, 1, PCRE_CASELESS)) sGrp = UpperCase(sGrp);
  if (HmsRegExMatch('[A-Z]', sGrp, sGrp)) sGrp = 'A..Z';
  if (HmsRegExMatch('[0-9]', sGrp, sGrp)) sGrp = '#';
  return sGrp;
}

///////////////////////////////////////////////////////////////////////////////
// Загрузка страниц и парсинг 
void LoadAndParse() {
  string sHtml, sData, sName, sLink, sImg, sYear, sPage, sVal, sPost, sServ; // Объявляем переменные
  THmsScriptMediaItem Item, Folder = FolderItem; TRegExpr RegEx;
  int i, n, nPages=0, iCnt=0, nGrp=0; char sGrp=""; bool bGroup=false;

  if (HmsRegExMatch('--maxingroup=(\\d+)', mpPodcastParameters, sVal)) gnMaxInGroup = StrToInt(sVal);
  if (HmsRegExMatch('--maxpages=(\\d+)'  , mpPodcastParameters, sVal)) gnMaxPages   = StrToInt(sVal);
  HmsRegExMatch('--group=(\\w+)', mpPodcastParameters, gsGroupMode);
  gbYearInTitle = (Pos('--yearintitle', mpPodcastParameters)>0); 

  if (LeftCopy(mpFilePath, 4) != "http") {
    // Если нет ссылки - делаем поиск названия
    if (Length(mpTitle)<4) mpTitle += " :::";    // Фишка обхода ограничения на минимальную длину в 4 символа (двоеточие при самом поиске не учитывается)
    HmsRegExMatch('//(.*)', gsUrlBase, sServ); // получаем доменное имя из gsUrlBase в sServ
    sPost = 'do=search&subaction=search&titleonly=3&story='+HmsHttpEncode(mpTitle);
    sHtml = HmsSendRequestEx(sServ, '/index.php?do=search', 'POST',
                             'application/x-www-form-urlencoded; Charset=UTF-8', 
                             gsUrlBase+'/\r\nAccept-Encoding: gzip, deflate\r\nOrigin: '+gsUrlBase, 
                             sPost, 80, 0, '', true);
    gnMaxPages = 1;

  } else {
    // Иначе просто, загружаем страницу по ссылке
    sHtml = HmsDownloadURL(mpFilePath, 'Referer: '+gsUrlBase, true);

  }
  sHtml = HmsUtf8Decode(sHtml);       // Декодируем страницу из UTF-8
  sHtml = HmsRemoveLineBreaks(sHtml); // Удаляем переносы строк, для облегчения работы с регулярными выражениями

  // Если указан шаблон поиска максимального номера страницы - применяем
  if ((gsPatternPages!='') && HmsRegExMatch(gsPatternPages, sHtml, sVal)) nPages = StrToIntDef(sVal, 0);

  // Вырезаем только нужный участок текста HTML, где будем искать блоки.
  // Вместо <fromCut> и <toCut> вставляем начало и конец участка HTML, между которыми
  // будем искать блоки текста с сылкой, наименованием и проч.
  HmsRegExMatch(gsCutPage, sHtml, sHtml); // ищем в sHtml, результат кладём обратно в sHtml

  // =========================================================================
  // Дозагрузка страниц
  if ((gnMaxPages!=0) && (nPages>gnMaxPages)) nPages = gnMaxPages;
  for (i=2; i<=nPages; i++) {
    HmsSetProgress(Trunc(i*100/nPages));
    HmsShowProgress(Format('%s: Загрузка страницы %d из %d', [mpTitle, i, nPages]));
    sLink = mpFilePath + ReplaceStr(gsPagesParams, '<PN>', IntToStr(i));
    sPage = HmsDownloadURL(sLink, 'Referer: '+gsUrlBase, true);
    sPage = HmsUtf8Decode(sPage);
    if (gsCutPage!='') HmsRegExMatch(gsCutPage, sPage, sPage);
    sHtml += sPage;
    if (HmsCancelPressed) break;
  }
  HmsHideProgress();                                                                                                   
  // =========================================================================
      
  // Создаём объект для поиска блоков текста по регулярному выражению,
  // в которых есть информация: ссылка, наименование, ссылка на картинку и проч.
  // Обычно, определяем начало и конец блока и вставляем их вместо <section> и </section>
  RegEx = TRegExpr.Create(gsPatternBlock, PCRE_SINGLELINE);
  try {
    // Определяем, если блоков в загруженном более чем gnMaxInGroup, включаем группировку
    i=0; if (RegEx.Search(sHtml)) do i++; while (RegEx.SearchAgain());
    bGroup = (i > gnMaxInGroup);
    // Главный цикл поиска блоков
    if (RegEx.Search(sHtml)) do {
      sLink=''; sImg=''; sYear=''; sName='';
      HmsRegExMatch(gsPatternTitle, RegEx.Match, sName);
      HmsRegExMatch(gsPatternLink , RegEx.Match, sLink);
      HmsRegExMatch(gsPatternImg  , RegEx.Match, sImg );
      HmsRegExMatch(gsPatternYear , RegEx.Match, sYear);
      if (Trim(sLink)=="") continue;
    
      sName = ReplaceStr(HmsHtmlToText(sName), "/", "-");
      sLink = HmsExpandLink(sLink, gsUrlBase);

      if (sImg!='') sImg = HmsExpandLink(sImg, gsUrlBase);

      // Если в ссылках встречаются русские символы - делаем их безопасными
      if (HmsRegExMatch('^.*?([а-яА-Я].*)', sImg, sVal)) sImg = ReplaceStr(sImg, sVal, HmsPercentEncode(HmsUtf8Encode(sVal)));

      // Если указано добавлять год вназвание и в названии его нет, добавляем
      if (gbYearInTitle && (sYear!='') && (Pos(sYear, sName)<1)) sName += ' ('+sYear+')';

      // Контроль группировки (создаём папку с именем группы)
      if (gsGroupMode=='alph') {
        Folder = FolderItem.AddFolder(GetGroupName(sName)); 
        Folder[mpiFolderSortOrder] = "mpTitle";
      } else if (gsGroupMode=='year') {
        Folder = FolderItem.AddFolder(sYear); 
        Folder[mpiFolderSortOrder] = "mpTitle";
        Folder[mpiYear           ] = sYear;
      } else if (bGroup) {
        iCnt++; if (iCnt>=gnMaxInGroup) { nGrp++; iCnt=0; }
        Folder = FolderItem.AddFolder(Format('%.2d', [nGrp])); 
      }

      CreateFolder(Folder, sName, sLink, sImg); // Создание ссылки (папки с фильмом)

    } while (RegEx.SearchAgain);
  } finally { RegEx.Free(); }
  if      (gsGroupMode=='alph') FolderItem.Sort('mpTitle');
  else if (gsGroupMode=='year') FolderItem.Sort('-mpYear');

  HmsLogMessage(1, mpTitle+': создано элементов - '+IntToStr(gnTotalItems));
}

///////////////////////////////////////////////////////////////////////////////
//                    Г Л А В Н А Я    П Р О Ц Е Д У Р А                     //
{
  FolderItem.DeleteChildItems(); // Удаляем созданные ранее элементы в текущей папке
  LoadAndParse();                // Запускаем загрузку страниц и создание папок видео
}
