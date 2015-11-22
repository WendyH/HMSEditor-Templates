///////////////////////////////////////////////////////////////////////////////
// ---- Создание ссылок на файл(ы) по переданной ссылке -----------------------
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
        AddMediaItem(Folder, sName, sFile, sQual); // то передаём индификатор качества как имя группы, где будет создана ссылка
      } else {                                     
        if (sName=='') HmsRegExMatch('.*/(.*)', sLink, sName); // Получаем имя файла из ссылки (всё что идёт после последнего слеша)
        sName = ReplaceStr(sName, sCut, '');          // Убираем перечисление качества из имени
        sName = ReplaceStr(sName, '_', '');           // А также подчекривания (лишние)
        AddMediaItem(Folder, sQual+' '+sName, sFile); // Добавляем индификатор качества к началу имени и создаём ссылку
      }
    }

  } else {
    // Если шаблона выбора качества в ссылке нет, то просто создаём ссылку
    if (sName=='') HmsRegExMatch('.*/(.*)', sLink, sName); // Если имя пустое, получаем имя файла из ссылки (всё что идёт после последнего слеша)
    AddMediaItem(Folder, sName, sLink);                    

  }
}
