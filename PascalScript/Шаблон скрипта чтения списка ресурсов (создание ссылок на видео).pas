// Объявление глобальных переменных
Var
  gsUrlBase: String = 'http://site.com'; // База ссылки, для создания полных ссылок из относительных
  gStart: TDateTime = Now;     // Время старта скрипта
  gnTotalItems: Integer = 0;   // Счетчик количества созданных элементов
  gnSec : Integer;             // Число секунд длительности видео 
  RegExp: TRegExpr;            // Объект для поиска по регулярному выражению
  Item  : THmsScriptMediaItem; // Объект элемента базы данных программы 
  gsHtml, gsLink, gsName, gsImg, gsTime, gsVal : String;
  
// ГЛАВНАЯ ПРОЦЕДУРА
Begin
  FolderItem.DeleteChildItems;           // Очищаем существующие ссылки

  gsHtml := HmsDownloadUrl(mpFilePath);  // Загрузка страницы по ссылке
  gsHtml := HmsUtf8Decode(gsHtml);       // Перекодируем текст из UTF-8
  gsHtml := HmsRemoveLinebreaks(gsHtml); // Удаляем переносы строк

  // Создаём объект для поиска блоков текста по регулярному выражению,
  // в которых есть информация: ссылка, наименование, ссылка на картинку и проч.
  // Обычно, определяем начало и конец блока и вставляем их вместо <section> и </section>
  RegExp := TRegExpr.Create('<section>(.*?)</section>', PCRE_SINGLELINE);
  
  Try
    // Организуем цикл поиска блоков текста в gsHtml
    If RegExp.Search(gsHtml) Then Repeat // Если Search(...) вернёт True (найдёт) - выполним цикл Repeat

      // Получаем данные о видео
      HmsRegExMatch('<a[^>]+href=[''"](.*?)[''"]' , RegExp.Match, gsLink); // Ссылка
      HmsRegExMatch('(<h4.*</h4>)'                , RegExp.Match, gsName); // Наименование
      HmsRegExMatch('<img[^>]+src=[''"](.*?)[''"]', RegExp.Match, gsImg ); // Картинка
      HmsRegExMatch('duration.*?>(.*?)<'          , RegExp.Match, gsTime); // Длительность
      
      gsName := HmsHtmlToText(gsName);            // Избавляемся от html тегов в названии 
      gsLink := HmsExpandLink(gsLink, gsUrlBase); // Делаем из относительных ссылок абсолютные
      gsImg  := HmsExpandLink(gsImg , gsUrlBase);

      // Вычисляем длительность в секундах из того формата, который на сайте (m:ss)
      gnSec := 0;
      if HmsRegExMatch('(\d+):', gsTime, gsVal) then gnSec := gnSec + StrToInt(gsVal) * 60; // Минуты 
      if HmsRegExMatch(':(\d+)', gsTime, gsVal) then gnSec := gnSec + StrToInt(gsVal);      // Секунды 

      // Создаём элемент медиа-ссылки
      Item := HmsCreateMediaItem(gsLink, FolderItem.ItemID); // Создаём элемент подкаста
      Item.Properties[mpiTitle     ] := gsName; // Наименование 
      Item.Properties[mpiThumbnail ] := gsImg;  // Картинка 
      Item.Properties[mpiTimeLength] := gnSec;  // Длительность 
      Item.Properties[mpiCreateDate] := DateTimeToStr(IncTime(gStart, 0, -gnTotalItems, 0, 0)); // Для обратной сортировки по дате создания

      Inc(gnTotalItems); // Увеличиваем счетчик созданных элементов

    Until Not RegExp.SearchAgain; // Повторять цикл пока SearchAgain возвращает True 
  
  Finally
    RegExp.Free; // Освобождаем созданный объект из памяти

  End;
  HmsLogMessage(1, mpTitle+': создано элементов - '+Str(gnTotalItems)); 
End.