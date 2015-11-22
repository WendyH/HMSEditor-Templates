///////////////////////////////////////////////////////////////////////////////
// Параметры:
// 1) Folder - Папка, где будут создаватся влоденные папки и элементы
// 2) sLink  - ссылка на файл плейлиста или сами данные JSON плейлиста
// 3) sName  - Имя подпапки, которую необходимо создать. Необязательный параметр.
// ---- Создание серий из плейлиста -------------------------------------------
Procedure CreateSeriesFromPlaylist(Folder: THmsScriptMediaItem; sLink: String; sName: String='');
Var
  sData, s1, s2, s3: String; i: Integer; JSON, PLITEM: TJsonObject; PLAYLIST: TJsonArray; // Объявляем переменные
Begin
  // Если передано имя плейлиста, то создаём папку, в которой будем создавать элементы
  If Trim(sName)<>'' Then Folder := Folder.AddFolder(sName);          
  
  // Если в переменной sLink сожержится знак '{', то там не ссылка, а сами данные Json
  If Pos('{', sLink)>0 Then
    sData := sLink
  Else Begin
    sData := HmsDownloadURL(sLink, "Referer: "+mpFilePath, true);  // Загружаем плейлист
    sData := DecodeUppodText(sData);                               // Дешифруем
  End;

  JSON := TJsonObject.Create();                 // Создаём объект для работы с Json
  Try
    JSON.LoadFromString(sData);                 // Загружаем json данные в объект
    PLAYLIST := JSON.A['playlist'];              // Пытаемся получить array с именем 'playlist'
    If PLAYLIST= nil Then PLAYLIST := JSON.AsArray; // Если массив 'playlist' получить не получилось, то представляем все наши данные как массив
    If PLAYLIST<>nil Then Begin                     // Если получили массив, то запускаем обход всех элементов в цикл
      For i := 0 To PLAYLIST.Length-1 Do Begin
        PLITEM := PLAYLIST[i];                   // Получаем текущий элемент массива
        sName := PLITEM.S['comment'];            // Название - значение поля comment
        sLink := PLITEM.S['file'   ];            // Получаем значение ссылки на файл
      
        // Форматируем числовое представление серий в названии
        // Если в названии есть число, то будет в s1 - то, что стояло перед ним, s2 - само число, s3 - то, что было после числа
        If HmsRegExMatch3('^(.*?)(\\d+)(.*)$', sName, s1, s2, s3) Then
          sName := Format('%s %.2d %s', [s1, StrToInt(s2), s3]); // Форматируем имя - делаем число двухцифровое (01, 02...)

        // Проверяем, если это вложенный плейлист - запускаем создание элементов из этого плейлиста рекурсивно
        If PLITEM.B['playlist'] Then
          CreateSeriesFromPlaylist(Folder, PLITEM.S['playlist'], sName)
        Else 
          CreateVideoLink(Folder, sName, sLink, true); // Иначе просто создаём ссылки на видео
      End;
    End; // end if (PLAYLIST!=nil) 
  
  Finally
      JSON.Free; // Какие бы ошибки не случились, освобождаем объект из памяти
  End;      
End;
