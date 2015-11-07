// Глобальные переменные
string gsUrlBase = 'http://site.com'; // База ссылки, для создания полных ссылок из относительных
string gsHtml, gsLink, gsName, gsImg, gsTime, gsVal;
int    gnSec; // Число секунд длительности видео 
TRegExpr RegExp;          // Объект для поиска по регулярному выражению
THmsScriptMediaItem Item; // Объект элемента базы данных программы 
  
// ГЛАВНАЯ ПРОЦЕДУРА
{
  FolderItem.DeleteChildItems();        // Очищаем существующие ссылки

  gsHtml = HmsDownloadUrl(mpFilePath);  // Загрузка страницы по ссылке
  gsHtml = HmsUtf8Decode(gsHtml);       // Перекодируем текст из UTF-8
  gsHtml = HmsRemoveLinebreaks(gsHtml); // Удаляем переносы строк

  // Создаём объект для поиска блоков текста по регулярному выражению,
  // в которых есть информация: ссылка, наименование, ссылка на картинку и проч.
  // Обычно, определяем начало и конец блока и вставляем их вместо <section> и </section>
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

      // Вычисляем длительность в секундах из того формата, который на сайте (m:ss)
      gnSec = 0;
      if (HmsRegExMatch('(\\d+):', gsTime, gsVal)) gnSec += StrToInt(gsVal) * 60; // Минуты 
      if (HmsRegExMatch(':(\\d+)', gsTime, gsVal)) gnSec += StrToInt(gsVal);      // Секунды 

      // Создаём элемент медиа-ссылки
      Item = HmsCreateMediaItem(gsLink, FolderItem.ItemID); // Создаём элемент подкаста
      Item.Properties[mpiTitle     ] = gsName; // Наименование 
      Item.Properties[mpiThumbnail ] = gsImg;  // Картинка 
      Item.Properties[mpiTimeLength] = gnSec;  // Длительность 

    } while (RegExp.SearchAgain); // Повторять цикл пока SearchAgain возвращает True 

  } finally { RegExp.Free; }      // Что бы ни случилось, освобождаем объект из памяти

}