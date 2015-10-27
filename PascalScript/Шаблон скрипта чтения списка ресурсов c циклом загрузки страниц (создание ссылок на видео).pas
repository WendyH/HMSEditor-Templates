// Глобальные переменные
var
  gsUrlBase: String = 'http://site.com'; // База ссылки, для создания полных ссылок из относительных
  gnTotalItems: Integer = 0;             // Количество созданных элементов
  
///////////////////////////////////////////////////////////////////////////////

// ---- Загрузка страниц и создание ссылок ------------------------------------
Procedure LoadPagesAndCreateLinks();
Var
  sHtml, sData, sName, sLink, sImg, sYear, sTime, sVal: String; 
  i, nPages, nSec: Integer; RegEx: TRegExpr;
  Item: THmsScriptMediaItem;  // Объект элемента базы данных программы 
Begin
  
  sHtml  := "";
  nPages := 2;  // Количество загружаемых страниц

  // Загружаем первые сколько-то страниц
  For i := 1 To nPages Do Begin
    HmsSetProgress(Trunc(i*100/nPages));                   // Устанавливаем позицию прогресса загрузки 
    sName := Format('%s: Страница %d из %d', [mpTitle, i, nPages]); // Формируем заголовок прогресса
    HmsShowProgress(sName);                                // Показываем окно прогресса выполнения
    sLink := mpFilePath+'/page/'+IntToStr(i)+'/';          // Формируем ссылку для загрузки, включающую номер страницы
    sHtml := sHtml + HmsUtf8Decode(HmsDownloadUrl(sLink)); // Загружаем страницу
    If HmsCancelPressed Then Break;                        // Если в окне прогресса нажали "Отмена" - прерываем цикл
  End;
  HmsHideProgress;                       // Убираем окно прогресса с экрана

  sHtml := HmsUtf8Decode(sHtml);         // Перекодируем текст из UTF-8
  sHtml := HmsRemoveLinebreaks(sHtml);   // Удаляем переносы строк

  // Создаём объект для поиска по регулярному выражению
  RegEx := TRegExpr.Create('<section>(.*?)</section>', PCRE_SINGLELINE);
  
  // Организовываем цикл
  If RegEx.Search(sHtml) Then Repeat
    sLink:=""; sName:=""; sImg:=""; sYear:=""; sTime:=""; // Очищаем значения после последнего цикла
  
    // Получаем данные о видео
    HmsRegExMatch('<a[^>]+href=[''"](.*?)[''"]' , RegEx.Match, sLink); // Ссылка
    HmsRegExMatch('(<h4.*</h4>)'                , RegEx.Match, sName); // Наименование
    HmsRegExMatch('<img[^>]+src=[''"](.*?)[''"]', RegEx.Match, sImg ); // Картинка
    HmsRegExMatch('year.*?(\d{4})'              , RegEx.Match, sYear); // Год
    HmsRegExMatch('duration.*?>(.*?)<'          , RegEx.Match, sTime); // Длительность
    
    sName := HmsHtmlToText(sName);            // Избавляемся от html тегов в названии 
    sLink := HmsExpandLink(sLink, gsUrlBase); // Делаем из относительных ссылок абсолютные
    sImg  := HmsExpandLink(sImg , gsUrlBase);

    // Если в названии нет года, добавляем год выхода 
    If (sYear<>'') AND (Pos(sYear, sName) < 1) Then sName := sName + ' ('+sYear+')';
    
    // Вычисляем длительность в секундах из того формата, который на сайте (m:ss)
    nSec := 0;
    If HmsRegExMatch('(\d+):', sTime, sVal) Then nSec := nSec + StrToInt(sVal) * 60; // Минуты 
    If HmsRegExMatch(':(\d+)', sTime, sVal) Then nSec := nSec + StrToInt(sVal);      // Секунды 
    If nSec = 0 Then nSec := 6000; // Если длительность нет - ставим по-умолчанию 01:40:00 (100 мин)
    
    // Создаём элемент медиа-ссылки
    Item := HmsCreateMediaItem(sLink, FolderItem.ItemID); // Создаём элемент подкаста
    Item.Properties[mpiTitle     ] := sName; // Наименование 
    Item.Properties[mpiThumbnail ] := sImg;  // Картинка 
    Item.Properties[mpiYear      ] := sYear; // Год 
    Item.Properties[mpiTimeLength] := nSec;  // Длительность 

    Inc(gnTotalItems);         // Увеличиваем счетчик созданных элементов
    
  Until Not RegEx.SearchAgain; // Повторять цикл пока SearchAgain возвращает True 

  RegEx.Free; // Освобождаем созданный объект из памяти

  HmsLogMessage(1, mpTitle+': создано элементов - '+Str(gnTotalItems)); 
End;
  
///////////////////////////////////////////////////////////////////////////////
//                     Г Л А В Н А Я    П Р О Ц Е Д У Р А                    //
begin
  FolderItem.DeleteChildItems;           // Очищаем существующие ссылки
  LoadPagesAndCreateLinks();             // Вызов процедуры загрузки страниц и создания ссылок
end.