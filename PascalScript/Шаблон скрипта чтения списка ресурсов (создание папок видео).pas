// Глобальные переменные
var
  gsUrlBase: String = 'http://site.com'; // База ссылки, для создания полных ссылок из относительных
  gnTotalItems: Integer = 0;             // Количество созданных элементов
  
///////////////////////////////////////////////////////////////////////////////

// ---- Создание папки --------------------------------------------------------
Function CreateFolder(sName, sLink: String; sImg: String = ''): THmsScriptMediaItem;
Begin
  Result := FolderItem.AddFolder(sLink);    // Создаём папку с указанной ссылкой
  Result.Properties[mpiTitle    ] := sName; // Присваиваем наименование
  Result.Properties[mpiThumbnail] := sImg;  // Картинка
  Inc(gnTotalItems); // Увеличиваем счетчик созданных элементов
End;

// ---- Загрузка страниц и создание ссылок ------------------------------------
Procedure LoadPagesAndCreateLinks();
Var
  sHtml, sData, sName, sLink, sImg, sYear, sVal: String; 
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
  RegEx := TRegExpr.Create('<section>(.*?)<section>', PCRE_SINGLELINE);
  
  // Организовываем цикл
  If RegEx.Search(sHtml) Then Repeat
    sLink:=""; sName:=""; sImg:=""; sYear:=""; // Очищаем значения после последнего цикла
  
    // Получаем данные о видео
    HmsRegExMatch('<a[^>]+href=[''"](.*?)[''"]' , RegEx.Match, sLink); // Ссылка
    HmsRegExMatch('alt="(.*?)"'                 , RegEx.Match, sName); // Наименование
    HmsRegExMatch('<img[^>]+src=[''"](.*?)[''"]', RegEx.Match, sImg ); // Картинка
    HmsRegExMatch('year.*?(\d{4})'              , RegEx.Match, sYear); // Год
    
    sName := HmsHtmlToText(sName);            // Избавляемся от html тегов в названии 
    sLink := HmsExpandLink(sLink, gsUrlBase); // Делаем из относительных ссылок абсолютные
    sImg  := HmsExpandLink(sImg , gsUrlBase);

    // Если в названии нет года, добавляем год выхода 
    If (sYear<>'') AND (Pos(sYear, sName) < 1) Then sName := sName + ' ('+sYear+')';
    
    // Создаём папку видео
    CreateFolder(sName, sLink);

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