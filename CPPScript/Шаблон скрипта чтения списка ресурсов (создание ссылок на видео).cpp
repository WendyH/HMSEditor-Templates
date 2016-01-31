// Глобальные переменные
string gsUrlBase    = 'http://site.com'; // База ссылки, для создания полных ссылок из относительных
int    gnTotalItems = 0; // Количество созданных ссылок

///////////////////////////////////////////////////////////////////////////////
// Поиск и создание ссылок на видео
void CreateLinks() {
  string sHtml, sLink, sName, sImg, sTime, sVal;
  TRegExpr RegExp; THmsScriptMediaItem Item;

  sTime = '02:00:00.000';       // Длительность по-умолчанию
  sHtml = HmsUtf8Decode(HmsDownloadUrl(mpFilePath)); // Загрузка страницы
  sHtml = HmsRemoveLinebreaks(sHtml);                // Удаляем переносы строк

  // Поиск блоков текста по регулярному выражению
  RegExp = TRegExpr.Create('<section>(.*?)</section>', PCRE_SINGLELINE);
  try {
    // Организуем цикл поиска блоков текста в gsHtml
    if (RegExp.Search(sHtml)) do {

      // Получаем данные о видео
      HmsRegExMatch('<a[^>]+href=[\'"](.*?)[\'"]' , RegExp.Match, sLink); // Ссылка
      HmsRegExMatch('(<h4.*</h4>)'                , RegExp.Match, sName); // Наименование
      HmsRegExMatch('<img[^>]+src=[\'"](.*?)[\'"]', RegExp.Match, sImg ); // Картинка
      HmsRegExMatch('duration.*?>(.*?)<'          , RegExp.Match, sTime); // Длительность

      sName = HmsHtmlToText(sName);            // Избавляемся от html тегов в названии
      sLink = HmsExpandLink(sLink, gsUrlBase); // Делаем из относительных ссылок абсолютные
      sImg  = HmsExpandLink(sImg , gsUrlBase);

      // Создаём элемент медиа-ссылки
      Item = HmsCreateMediaItem(sLink, FolderItem.ItemID); // Создаём элемент подкаста
      Item[mpiTitle     ] = sName; // Наименование
      Item[mpiThumbnail ] = sImg;  // Картинка
      Item[mpiTimeLength] = sTime; // Длительность
      gnTotalItems++;

    } while (RegExp.SearchAgain); // Повторять цикл пока SearchAgain возвращает True

  } finally { RegExp.Free; }      // Что бы ни случилось, освобождаем объект из памяти

  HmsLogMessage(1, mpTitle+": Создано элементов - "+Str(gnTotalItems));
}



///////////////////////////////////////////////////////////////////////////////
//                    Г Л А В Н А Я     П Р О Ц Е Д У Р А                    //
{
  FolderItem.DeleteChildItems(); // Очищаем существующие ссылки

  CreateLinks(); // Создаём ссылки
}
