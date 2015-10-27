// Глобальные переменные
  string gsUrlBase = 'http://site.com'; // База ссылки, для создания полных ссылок из относительных

  string gsHtml; // Содержимое страницы сайта
  string gsLink; // Ссылка на видео
  string gsName; // Наименование видео
  string gsImg ; // Картинка превью (эскиз)
  string gsTime; // Длительность видео

  string gsVal ; // Переменная для хранения временного значения строки
  int    gnSec ; // Число секунд длительности видео 

  TRegExpr RegExp;          // Объект для поиска по регулярному выражению
  THmsScriptMediaItem Item; // Объект элемента базы данных программы 
  
// ГЛАВНАЯ ПРОЦЕДУРА
{
  FolderItem.DeleteChildItems();        // Очищаем существующие ссылки

  gsHtml = HmsDownloadUrl(mpFilePath);  // Загрузка страницы по ссылке
  gsHtml = HmsUtf8Decode(gsHtml);       // Перекодируем текст из UTF-8
  gsHtml = HmsRemoveLinebreaks(gsHtml); // Удаляем переносы строк

  // Создаём объект для поиска по регулярному выражению
  RegExp = TRegExpr.Create('media--sm-v(.*?)meta__item', PCRE_SINGLELINE);
  
  // Организовываем цикл
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

  RegExp.Free(); // Освобождаем созданный объект из памяти
}