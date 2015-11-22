///////////////////////////////////////////////////////////////////////////////
// Создание ссылки(ок) на файл(ы) по ссылке, в которой может содержаться перечень качества [720,480,360]
Procedure CreateVideoLink(aFolder: THmsScriptMediaItem; aName, aLink: String; bSeparateInFolders: Boolean=false);
Var
  sCut, sQualArray, sQual, sFile: String; i, nCount: Integer; // Объявляем переменные
Begin
  // Проверяем, есть ли в переданной ссылке шаблон с массивом существующего качества "[720,480,360]"
  If HmsRegExMatch('\\[(.*?)\\]', aLink, sQualArray) Then Begin
    sCut   := '['+sQualArray+']';                  // Та часть, которая будет заменятся на индификатор качества
    nCount := WordCount(sQualArray, ',');          // Количество елементов, разделённых запятой
    For i := 1 To nCount Do Begin
      sQual := ExtractWord(i, sQualArray, ',');    // Получаем очередной индификатор качества
      If sQual='' Then Continue;                   // Может быть пропущен, если не указан
      aFile := ReplaceStr(aLink, sCut, sQual);     // Формируем ссылку на файл, заменяя шаблон на индификатор качества
      If bSeparateInFolders Then                   // Если был передан флаг "Группировать файлы качества по разным папкам",
        AddMediaItem(aFolder, aName, sFile, sQual) // то передаём индификатор качества как имя группы, где будет создана ссылка
      Else Begin                                     
        If aName='' Then HmsRegExMatch('.*/(.*)', aLink, sName); // Получаем имя файла из ссылки (всё что идёт после последнего слеша)
        aName := ReplaceStr(aName, sCut, '');          // Убираем перечисление качества из имени
        aName := ReplaceStr(aName, '_', '');           // А также подчекривания (лишние)
        AddMediaItem(aFolder, sQual+' '+aName, sFile); // Добавляем индификатор качества к началу имени и создаём ссылку
      End;
    End;

  End Else Begin
    // Если шаблона выбора качества в ссылке нет, то просто создаём ссылку
    If aName='' Then HmsRegExMatch('.*/(.*)', aLink, aName); // Если имя пустое, получаем имя файла из ссылки (всё что идёт после последнего слеша)
    AddMediaItem(aFolder, aName, aLink);                    

  End;
End;
