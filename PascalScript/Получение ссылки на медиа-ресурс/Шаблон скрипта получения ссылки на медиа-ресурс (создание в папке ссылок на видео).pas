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
  
  sHtml := HmsDownloadUrl(mpFilePath);  // Загружаем страницу
  sHtml := HmsUtf8Decode(sHtml);        // Перекодируем текст из UTF-8
  sHtml := HmsRemoveLinebreaks(sHtml);  // Удаляем переносы строк

  // Создаём объект для поиска блоков с информацией по регулярному выражению
  RegEx := TRegExpr.Create('<section>(.*?)</section>', PCRE_SINGLELINE);
  
  // Организовываем цикл
  If RegEx.Search(sHtml) Then Repeat
    sLink:=""; sName:=""; sImg:=""; sYear:=""; sTime:=""; // Очищаем значения после последнего цикла
  
    // Получаем данные о видео
    HmsRegExMatch('<a[^>]+href=[''"](.*?)[''"]' , RegEx.Match, sLink); // Ссылка
    HmsRegExMatch('(<h.*</h.>)'                 , RegEx.Match, sName); // Наименование
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
    Item := HmsCreateMediaItem(sLink, PodcastItem.ItemID); // Создаём элемент подкаста
    Item.Properties[mpiTitle     ] := sName; // Наименование 
    Item.Properties[mpiThumbnail ] := sImg;  // Картинка 
    Item.Properties[mpiYear      ] := sYear; // Год 
    Item.Properties[mpiTimeLength] := nSec;  // Длительность 

    Inc(gnTotalItems);         // Увеличиваем счетчик созданных элементов
    
  Until Not RegEx.SearchAgain; // Повторять цикл пока SearchAgain возвращает True 

  RegEx.Free; // Освобождаем созданный объект из памяти

  HmsLogMessage(1, mpTitle+': создано элементов - '+Str(gnTotalItems)); 
End;

// ---- Получение ссылки на медиа-ресурс --------------------------------------
Procedure GetResourceLink();
Var
  sHtml, sLink, sVal: String;
Begin
  
  // ВСТАВЬТЕ КОД ПОЛУЧЕНИЯ ССЫЛКИ ТУТ
  MediaResourceLink := mpFilePath;
  
End;

  
///////////////////////////////////////////////////////////////////////////////
//                     Г Л А В Н А Я    П Р О Ц Е Д У Р А                    //
Begin
  
  If PodcastItem.IsFolder Then Begin
    PodcastItem.DeleteChildItems;          // Очищаем существующие ссылки
    LoadPagesAndCreateLinks();             // Вызов процедуры загрузки страниц и создания ссылок
  
  End Else Begin
  
    GetResourceLink();
  
  End;
    
End.