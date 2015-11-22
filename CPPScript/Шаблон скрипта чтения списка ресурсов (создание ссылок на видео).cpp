// Глобальные переменные
string gsUrlBase = 'http://site.com'; // База ссылки, для создания полных ссылок из относительных
string gsHtml, gsLink, gsName, gsImg, gsTime, gsVal;
TRegExpr RegExp; THmsScriptMediaItem Item;
  
///////////////////////////////////////////////////////////////////////////////
// ГЛАВНАЯ ПРОЦЕДУРА
{
  FolderItem.DeleteChildItems(); // Очищаем существующие ссылки
  gsTime = '02:00:00.000';       // Длительность по-умолчанию
  gsHtml = HmsUtf8Decode(HmsDownloadUrl(mpFilePath)); // Загрузка страницы
  gsHtml = HmsRemoveLinebreaks(gsHtml);               // Удаляем переносы строк

  // Поиск блоков текста по регулярному выражению
  RegExp = TRegExpr.Create('<section>(.*?)</section>', PCRE_SINGLELINE);
  try {
    // Организуем цикл поиска блоков текста в gsHtml
    if (RegExp.Search(gsHtml)) do {

      // Получаем данные о видео
      HmsRegExMatch('<a[^>]+href=[\'"](.*?)[\'"]' , RegExp.Match, gsLink); // Ссылка
      HmsRegExMatch('(<h4.*</h4>)'                , RegExp.Match, gsName); // Наименование
      HmsRegExMatch('<img[^>]+src=[\'"](.*?)[\'"]', RegExp.Match, gsImg ); // Картинка
      HmsRegExMatch('duration.*?>(.*?)<'          , RegExp.Match, gsTime); // Длительность

      gsName = HmsHtmlToText(gsName);            // Избавляемся от html тегов в названии 
      gsLink = HmsExpandLink(gsLink, gsUrlBase); // Делаем из относительных ссылок абсолютные
      gsImg  = HmsExpandLink(gsImg , gsUrlBase);

      // Создаём элемент медиа-ссылки
      Item = HmsCreateMediaItem(gsLink, FolderItem.ItemID); // Создаём элемент подкаста
      Item[mpiTitle     ] = gsName; // Наименование 
      Item[mpiThumbnail ] = gsImg;  // Картинка 
      Item[mpiTimeLength] = gsTime; // Длительность 

    } while (RegExp.SearchAgain); // Повторять цикл пока SearchAgain возвращает True 

  } finally { RegExp.Free; }      // Что бы ни случилось, освобождаем объект из памяти

}