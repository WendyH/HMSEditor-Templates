gsUrlBase    = 'http://site.com';
gnTotalItems = 0;

// Создание папки -------------------------------------------------------------
function CreateFolder(sName, sLink, sImg) {
  THmsScriptMediaItem Item = FolderItem.AddFolder(sLink); // Создаём папку с указанной ссылкой
  Item[mpiTitle    ] = sName; // Присваиваем наименование
  Item[mpiThumbnail] = sImg;  // Картинка
  gnTotalItems++;             // Увеличиваем счетчик созданных элементов
  return Item;                // Возвращаем созданный объект
}

// Загрузка страниц и парсинг -------------------------------------------------
function LoadAndParse() {
  sHtml  = ''; // Текст загруженных страниц сайта
  nPages = 2;  // Количество загружаемых страниц

  // Загружаем первые сколько-то страниц (указано в nPages)
  // В зависимости от того, как именно на конкретном сайте выглядят ссылки последующих
  // страниц, возможно потребуедтся изменить формирование ссылки '/page/'+...
  for (i=1; i<nPages; i++) {
    HmsSetProgress(Trunc(i*100/nPages));          // Устанавливаем позицию прогресса загрузки 
    sName = Format('%s: Страница %d из %d', [mpTitle, i, nPages]); // Формируем заголовок прогресса
    HmsShowProgress(sName);                       // Показываем окно прогресса выполнения
    sLink = mpFilePath;
    If (i > 1) sLink += '/page/'+IntToStr(i)+'/'; // Формируем ссылку для загрузки, включающую номер страницы    
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
//                     Г Л А В Н А Я    П Р О Ц Е Д У Р А                    //
{
  FolderItem.DeleteChildItems(); // Удаляем созданные ранее элементы в текущей папке
  LoadAndParse();                // Запускаем загрузку страниц и создание папок видео
}