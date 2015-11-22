string    gsUrlBase    = 'http://site.com'; // База для относительных ссылок
int       gnTotalItems = 0;                 // Счётчик созданных элементов
TDateTime gStart       = Now;               // Время начала запуска скрипта

///////////////////////////////////////////////////////////////////////////////
// Создание папки -------------------------------------------------------------
THmsScriptMediaItem CreateFolder(string sName, string sLink, string sImg='') {
  THmsScriptMediaItem Item = FolderItem.AddFolder(sLink); // Создаём папку с указанной ссылкой
  Item[mpiTitle     ] = sName; // Присваиваем наименование
  Item[mpiThumbnail ] = sImg;  // Картинка
  Item[mpiCreateDate] = DateTimeToStr(IncTime(gStart, 0, -gnTotalItems, 0, 0)); // Для обратной сортировки по дате создания

  gnTotalItems++;             // Увеличиваем счетчик созданных элементов
  return Item;                // Возвращаем созданный объект
}

///////////////////////////////////////////////////////////////////////////////
// Загрузка страниц и парсинг -------------------------------------------------
void LoadAndParse() {
  string sHtml, sData, sName, sLink, sImg, sYear; TRegExpr RegEx; // Объявляем переменные

  sHtml = HmsDownloadUrl(mpFilePath); // Загруженных страницу сайта
  sHtml = HmsUtf8Decode(sHtml);       // Декодируем из UTF-8 кодировки
  sHtml = HmsRemoveLineBreaks(sHtml); // Удаляем переносы строк, для облегчения работы с регулярными выражениями

  // Вырезаем только нужный участок текста HTML, где будем искать блоки.
  // Вместо <Begin> и <End> вставляем начало и конец участка HTML, между которыми
  // будем искать блоки текста с сылкой, наименованием и проч.
  HmsRegExMatch('<Begin>(.*?)<End>', sHtml, sHtml); // ищем в sHtml, результат кладём обратно в sHtml
  
  // Создаём объект для поиска блоков текста по регулярному выражению,
  // в которых есть информация: ссылка, наименование, ссылка на картинку и проч.
  // Обычно, определяем начало и конец блока и вставляем их вместо <section> и </section>
  RegEx = TRegExpr.Create('<section>(.*?)</section>'); 
  try {
    if (RegEx.Search(sHtml)) do {            // Если нашли совпадение, запускаем цикл
      sLink=''; sName=''; sImg=''; sYear=''; // Очищаем переменные от предыдущих значений

      // Получаем значения в переменные по регулярным выражениям
      HmsRegExMatch('<a[^>]+href="(.*?)"'  , RegEx.Match, sLink); // Ссылка
      HmsRegExMatch('(<a[^>]+href=.*?</a>)', RegEx.Match, sName); // Наименование
      HmsRegExMatch('<img[^>]+src="(.*?)"' , RegEx.Match, sImg ); // Картинка
      HmsRegExMatch('(\\d{4})\\)'          , sName      , sYear); // Год

      if (sLink=='') continue;          // Если нет ссылки, значит что-то не так
       
      sLink = HmsExpandLink(sLink, gsUrlBase);             // Делаем ссылку полной, если она таковой не является
      if (sImg!='') sImg = HmsExpandLink(sImg, gsUrlBase); // Если есть ссылка на картинку, делаем ссылку полной        
      sName = HmsHtmlToText(sName);                        // Преобразуем html в простой текст
      HmsRegExMatch('(.*?)/' , sName, sName);              // Обрезаем слишком длинные названия (на англ. языке)

      // Если в названии нет года, добавляем год выхода 
      if ((sYear!='') && (Pos(sYear, sName)<1)) sName += ' ('+sYear+')';

      CreateFolder(sName, sLink, sImg); // Вызываем функцию создания папки видео
                                      
    } while (RegEx.SearchAgain);        // Повторяем цикл, если найдено следующее совпадение
  
  } finally { RegEx.Free; }             // Что бы ни случилось, освобождаем объект из памяти

  HmsLogMessage(1, mpTitle+': создано элементов - '+IntToStr(gnTotalItems)); 
} 

///////////////////////////////////////////////////////////////////////////////
//                    Г Л А В Н А Я    П Р О Ц Е Д У Р А                     //
{
  FolderItem.DeleteChildItems(); // Удаляем созданные ранее элементы в текущей папке
  LoadAndParse();                // Запускаем загрузку страниц и создание папок видео
}