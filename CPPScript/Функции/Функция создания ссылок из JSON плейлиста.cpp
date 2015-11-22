///////////////////////////////////////////////////////////////////////////////
// Параметры:
// 1) Folder - Папка, где будут создаватся влоденные папки и элементы
// 2) sLink  - ссылка на файл плейлиста или сами данные JSON плейлиста
// 3) sName  - Имя подпапки, которую необходимо создать. Необязательный параметр.
// ---- Создание серий из плейлиста -------------------------------------------
void CreateSeriesFromPlaylist(THmsScriptMediaItem Folder, string sLink, string sName='') {
  string sData, s1, s2, s3; int i; TJsonObject JSON, PLITEM; TJsonArray PLAYLIST; // Объявляем переменные

  // Если передано имя плейлиста, то создаём папку, в которой будем создавать элементы
  if (Trim(sName)!='') Folder = Folder.AddFolder(sName);          
  
  // Если в переменной sLink сожержится знак '{', то там не ссылка, а сами данные Json
  if (Pos('{', sLink)>0) {
    sData = sLink;
  } else {
    sData = HmsDownloadURL(sLink, "Referer: "+mpFilePath, true);  // Загружаем плейлист
    sData = DecodeUppodText(sData);                               // Дешифруем
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
      
        // Форматируем числовое представление серий в названии
        // Если в названии есть число, то будет в s1 - то, что стояло перед ним, s2 - само число, s3 - то, что было после числа
        if (HmsRegExMatch3('^(.*?)(\\d+)(.*)$', sName, s1, s2, s3)) 
          sName = Format('%s %.2d %s', [s1, StrToInt(s2), s3]); // Форматируем имя - делаем число двухцифровое (01, 02...)

        // Проверяем, если это вложенный плейлист - запускаем создание элементов из этого плейлиста рекурсивно
        if (PLITEM.B['playlist']) 
          CreateSeriesFromPlaylist(Folder, PLITEM.S['playlist'], sName);
        else 
          CreateVideoLink(Folder, sName, sLink, true); // Иначе просто создаём ссылки на видео
      }
    } // end if (PLAYLIST!=nil) 
  
  } finally { JSON.Free; }                      // Какие бы ошибки не случились, освобождаем объект из памяти
}
