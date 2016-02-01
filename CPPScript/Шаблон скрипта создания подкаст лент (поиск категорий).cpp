char gsUrlBase="http://site.com"; int gnTotalItems=0; TDateTime gTimeStart=Now;

///////////////////////  Создание структуры подкаста  ///////////////////////// 

///////////////////////////////////////////////////////////////////////////////
// Функция создания динамической папки с указанным скриптом
THmsScriptMediaItem CreateDynamicItem(THmsScriptMediaItem prntItem, char sTitle, char sLink, char &sScript='') {                                
  THmsScriptMediaItem Folder = prntItem.AddFolder(sLink, false, 32);
  Folder[mpiTitle     ] = sTitle;
  Folder[mpiCreateDate] = VarToStr(IncTime(Now,0,-prntItem.ChildCount,0,0));
  Folder[200] = 5;           // mpiFolderType
  Folder[500] = sScript;     // mpiDynamicScript
  Folder[501] = 'C++Script'; // mpiDynamicSyntaxType
  Folder[mpiFolderSortOrder] = -mpiCreateDate;
  return Folder;
}

///////////////////////////////////////////////////////////////////////////////
// Замена в тексте загруженного скрипта значения текстовой переменной
void ReplaceVarValue(char &sText, char sVarName, char sNewVal) {
  char sVal, sVal2;
  if (HmsRegExMatch2("("+sVarName+"\\s*?=.*?';)", sText, sVal, sVal2)) {
     HmsRegExMatch(sVarName+"\\s*?=\\s*?'(.*)'", sVal, sVal2); 
     sText = ReplaceStr(sText, sVal, ReplaceStr(sVal , sVal2, sNewVal));
  }
}

///////////////////////////////////////////////////////////////////////////////
// Создание папки ПОИСК (с загрузкой скрипта с форума homemediaserver.ru)
void CreateSearchFolder(THmsScriptMediaItem prntItem, char sTitle) {
  char sScript='', sLink, sHtml, sRE, sVal; THmsScriptMediaItem Folder;
  
  // Да да, загружаем скрипт с сайта форума HMS
  sHtml = HmsUtf8Decode(HmsDownloadURL('http://homemediaserver.ru/forum/viewtopic.php?f=15&t=2793&p=17395#p17395', '', true));
  HmsRegExMatch('BeginDynamicSearchScript\\*/(.*?)/\\*EndDynamicSearchScript', sHtml, sScript, 1, PCRE_SINGLELINE);
  sScript = HmsHtmlToText(sScript, 1251);
  sScript = ReplaceStr(sScript, #160, ' ');

  // И меняем значения переменных на свои
//ReplaceVarValue(sScript, 'gsSuggestQuery'  , 'http://bobfilm1.net/engine/ajax/search.php?query=');
//ReplaceVarValue(sScript, 'gsSuggestRegExpr', '<span class="searchheading">(.*?)</span>');
//ReplaceVarValue(sScript, 'gsSuggestMethod' , 'POST');
  //sScript = ReplaceStr(sScript, 'gnSuggestNoUTFEnc = 0', 'gnSuggestNoUTFEnc = 1');

  Folder = prntItem.AddFolder(sTitle, true);
  Folder[mpiCreateDate     ] = VarToStr(IncTime(gTimeStart,0,-gnTotalItems,0,0));
  Folder[mpiFolderSortOrder] = "-mpCreateDate";
  gnTotalItems++;
  
  CreateDynamicItem(Folder, '"Набрать текст"', '-SearchCommands', sScript);
}

///////////////////////////////////////////////////////////////////////////////
// Создание подкаста или папки
THmsScriptMediaItem CreateItem(THmsScriptMediaItem Parent, char sTitle='', char sLink='') {
  THmsScriptMediaItem Item; bool bForceFolder = false;

  if (sLink=='') { sLink = sTitle; bForceFolder = true; }
  else             sLink = HmsExpandLink(sLink, gsUrlBase);
  
  Item = Parent.AddFolder(sLink, bForceFolder);
  Item[mpiTitle     ] = sTitle;
  Item[mpiCreateDate] = VarToStr(IncTime(gTimeStart,0,-gnTotalItems,0,0));
  Item[mpiFolderSortOrder] = -mpiCreateDate;
  gnTotalItems++;
  return Item;
}

///////////////////////////////////////////////////////////////////////////////
// Поиск и создание ссылок категории
void SearchCategories(THmsScriptMediaItem Folder, string sHtml, string sCutPattern) {
  string sName, sLink; TRegExpr RegExp;
  
  HmsRegExMatch(sCutPattern, sHtml, sHtml); // Вырезаем нужный блок
  // Ищем ссылки на категории и создаём их
  RegExp = TRegExpr.Create('(<a.*?</a>)', PCRE_SINGLELINE);
  try {
    if (RegExp.Search(sHtml)) do {
      sName = ""; sLink = "";
      HmsRegExMatch('<a[^>]+href="(.*?)"', RegExp.Match, sLink);
      HmsRegExMatch('(<a.*?</a>)'        , RegExp.Match, sName);
      sLink = HmsExpandLink(sLink, gsUrlBase);
      sName = HmsHtmlToText(sName);
      CreateItem(Folder, sName, sLink);
    } while (RegExp.SearchAgain);
  } finally { RegExp.Free; }
}

///////////////////////////////////////////////////////////////////////////////
// ---------------------  M A I N  P R O C E D U R E  -------------------------
{
  THmsScriptMediaItem Folder, Item; string sHtml, sName, sLink; TRegExpr RegExp;

  FolderItem.DeleteChildItems();

  CreateSearchFolder (FolderItem, '00. Поиск');
  Folder = CreateItem(FolderItem, '01. Последние поступления', '/');
  Folder[mpiPodcastParameters] = '--maxpages=7';
  
  // Загружаем страницу главную страницу
  sHtml = HmsDownloadURL(gsUrlBase, '', true);
  sHtml = HmsRemoveLineBreaks(HmsUtf8Decode(sHtml));
  
  Folder = CreateItem(FolderItem, '02. Категории');
  SearchCategories(Folder, sHtml, 'Категории(.*?)</div>');
  
  Folder = CreateItem(FolderItem, '03. По годам');
  Folder[mpiPodcastParameters] = '--group=year --maxpages=50';
  SearchCategories(Folder, sHtml, 'По году(.*?)</div>');
  
  Folder = CreateItem(FolderItem, '04. Сериалы');
  Folder[mpiPodcastParameters] = '--group=alph';
  SearchCategories(Folder, sHtml, 'Сериалы(.*?)</div>');
  
  Folder = CreateItem(FolderItem, '05. По странам');
  Folder[mpiPodcastParameters] = '--group=year';
  SearchCategories(Folder, sHtml, 'По странам(.*?)</div>');
  
  HmsLogMessage(1, mpTitle+': Создано ссылок - '+IntToStr(gnTotalItems));
}