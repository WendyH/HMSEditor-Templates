string gsUrlBase = 'http://site.com'; int gnTotalItems = 0;

///////////////////////////////////////////////////////////////////////////////

// Загрузка страниц и парсинг -------------------------------------------------
void LoadAndParse() {
  string sHtml, sData, sName, sLink, sImg, sYear, sTime, sVal; 
  int i, nPages, nSec; TRegExpr RegEx;
  THmsScriptMediaItem Item; // Объект элемента базы данных программы 

  sHtml  = ''; // Текст загруженных страниц сайта
  nPages = 2;  // Количество загружаемых страниц

  // Загружаем первые сколько-то страниц (указано в nPages)
  // В зависимости от того, как именно на конкретном сайте выглядят ссылки последующих
  // страниц, возможно потребуедтся изменить формирование ссылки '/page/'+...
  for (i=1; i<nPages; i++) {
    HmsSetProgress(Trunc(i*100/nPages));          // Устанавливаем позицию прогресса загрузки 
    sName = Format('%s: Страница %d из %d', [mpTitle, i, nPages]); // Формируем заголовок прогресса
    HmsShowProgress(sName);                       // Показываем окно прогресса выполнения
    sLink = mpFilePath+'/page/'+IntToStr(i)+'/';  // Формируем ссылку для загрузки, включающую номер страницы
    sHtml+= HmsUtf8Decode(HmsDownloadUrl(sLink)); // Загружаем страницу
    if (HmsCancelPressed()) break;                // Если в окне прогресса нажали "Отмена" - прерываем цикл
  }
  HmsHideProgress();                    // Убираем окно прогресса с экрана
  sHtml = HmsRemoveLineBreaks(sHtml);   // Удаляем переносы строк, для облегчения работы с регулярными выражениями

  // Создаём объект для поиска блоков текста по регулярному выражению,
  // в которых есть информация: ссылка, наименование, ссылка на картинку и проч.
  // Обычно, определяем начало и конец блока и вставляем их вместо <section> и </section>
  RegEx = TRegExpr.Create('<section>(.*?)</section>'); 
  try {
    if (RegEx.Search(sHtml)) do {       // Если нашли совпадение, запускаем цикл
      sLink=""; sName=""; sImg=""; sYear=""; sTime="";     // Очищаем значения после последнего цикла

      // Получаем значения в переменные по регулярным выражениям
      HmsRegExMatch('<a[^>]+href="(.*?)"'  , RegEx.Match, sLink); // Ссылка
      HmsRegExMatch('(<a[^>]+href=.*?</a>)', RegEx.Match, sName); // Наименование
      HmsRegExMatch('<img[^>]+src="(.*?)"' , RegEx.Match, sImg ); // Картинка
      HmsRegExMatch('year.*?(\\d{4})'      , RegEx.Match, sYear); // Год
      HmsRegExMatch('duration.*?>(.*?)<'   , RegEx.Match, sTime); // Длительность

      if (sLink=='') continue;          // Если нет ссылки, значит что-то не так
       
      sLink = HmsExpandLink(sLink, gsUrlBase);             // Делаем ссылку полной, если она таковой не является
      if (sImg!='') sImg = HmsExpandLink(sImg, gsUrlBase); // Если есть ссылка на картинку, делаем ссылку полной        
      sName = HmsHtmlToText(sName);                        // Преобразуем html в простой текст
      HmsRegExMatch('(.*?)/' , sName, sName);              // Обрезаем слишком длинные названия (на англ. языке)

      // Если в названии нет года, добавляем год выхода 
      if ((sYear!='') && (Pos(sYear, sName)<1)) sName += ' ('+sYear+')';

      // Вычисляем длительность в секундах из того формата, который на сайте (m:ss)
      nSec = 0;
      if (HmsRegExMatch('(\\d+):', sTime, sVal)) nSec += StrToInt(sVal) * 60; // Минуты 
      if (HmsRegExMatch(':(\\d+)', sTime, sVal)) nSec += StrToInt(sVal);      // Секунды 
      if (nSec == 0) nSec = 6000; // Если длительность нет - ставим по-умолчанию 01:40:00 (100 мин)

      // Создаём элемент медиа-ссылки
      Item = HmsCreateMediaItem(sLink, FolderItem.ItemID); // Создаём элемент подкаста
      Item.Properties[mpiTitle     ] = sName; // Наименование 
      Item.Properties[mpiThumbnail ] = sImg;  // Картинка 
      Item.Properties[mpiYear      ] = sYear; // Год 
      Item.Properties[mpiTimeLength] = nSec;  // Длительность 
                                      
    } while (RegEx.SearchAgain);        // Повторяем цикл, если найдено следующее совпадение
  
  } finally { RegEx.Free; }             // Что бы ни случилось, освобождаем объект из памяти

  HmsLogMessage(1, mpTitle+': создано элементов - '+IntToStr(gnTotalItems)); 
} 

///////////////////////////////////////////////////////////////////////////////
//                     Г Л А В Н А Я    П Р О Ц Е Д У Р А                    //
{
  FolderItem.DeleteChildItems(); // Удаляем созданные ранее элементы в текущей папке
  LoadAndParse();                // Запускаем загрузку страниц и создание папок видео
}