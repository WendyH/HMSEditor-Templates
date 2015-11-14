// Объявление глобальных переменных
Var
  gsUrlBase: String = 'http://site.com'; // База ссылки, для создания полных ссылок из относительных
  gnTotalItems: Integer = 0;             // Количество созданных элементов
  gStart: TDateTime     = Now;           // Время старта скрипта

///////////////////////////////////////////////////////////////////////////////

// ---- Создание папки --------------------------------------------------------
Function CreateFolder(sName, sLink: String; sImg: String = ''): THmsScriptMediaItem;
Begin
  Result := FolderItem.AddFolder(sLink);    // Создаём папку с указанной ссылкой
  Result.Properties[mpiTitle     ] := sName; // Присваиваем наименование
  Result.Properties[mpiThumbnail ] := sImg;  // Картинка
  Result.Properties[mpiCreateDate] := DateTimeToStr(IncTime(gStart, 0, -gnTotalItems, 0, 0)); // Для обратной сортировки по дате создания
  Inc(gnTotalItems); // Увеличиваем счетчик созданных элементов
End;

// ---- Загрузка страниц и создание ссылок ------------------------------------
Procedure LoadPagesAndCreateLinks();
Var
  sHtml, sData, sName, sLink, sImg, sYear, sVal: String; 
  i, nPages, nSec: Integer; RegEx: TRegExpr; bPagesGotten: Boolean;
  Item: THmsScriptMediaItem;  // Объект элемента базы данных программы 
Begin
  
  sHtml     := "";
  nPages    :=  2; // Количество загружаемых страниц
  nMaxPages := 50; // Максимальное ограничение количества загружаемых страниц
  bPagesGotten := False; // Если не хотим искать количество страниц для загрузки из загруженной первой страницы, то установить в True.

  // Если в параметрах подкаста указаны значения - устанавливаем оттуда
  If HmsRegExMatch('-pages=(\d+)'   , mpPodcastParameters, sVal) Then nPages    := StrToInt(sVal);
  If HmsRegExMatch('-maxpages=(\d+)', mpPodcastParameters, sVal) Then nMaxPages := StrToInt(sVal);

  // Загружаем первые сколько-то страниц (указано в nPages)
  // В зависимости от того, как именно на конкретном сайте выглядят ссылки последующих
  // страниц, возможно потребуедтся изменить формирование ссылки 'page/'+...
  // ------------- Цикл загрузки страниц ------------------
  For i := 1 To nPages Do Begin
    HmsSetProgress(Trunc(i*100/nPages));                   // Устанавливаем позицию прогресса загрузки 
    sName := Format('%s: Страница %d из %d', [mpTitle, i, nPages]); // Формируем заголовок прогресса
    HmsShowProgress(sName);                                // Показываем окно прогресса выполнения
    sLink := mpFilePath;                                   // По-умолчнанию ссылка для загрузки равна ссылке подкаста
    If i > 1 Then sLink := sLink+'page/'+IntToStr(i)+'/';  // Если это следующая страница, добавляем часть ссылки включающую номер страницы
    sHtml := sHtml + HmsUtf8Decode(HmsDownloadUrl(sLink)); // Загружаем страницу
    If HmsCancelPressed Then Break;                        // Если в окне прогресса нажали "Отмена" - прерываем цикл
    If Not bPagesGotten Then Begin                         // Если мы ещё не определяли сколько страниц загружать, пробуем найти в загруженной странице
      If HmsRegExMatch('.*/page/(\d+)/">\d+</a>', sHtml, sVal) Then nPages := StrToInt(sVal);
      nPages := Min(nPages, nMaxPages);                    // Ограничиваем количество страниц значением nMaxPages
      bPagesGotten := True;                                // Устанавливаем флаг, что мы уже пробовали определять количество страниц
    End;
    If i >= nPages Then Break;
  End;
  HmsHideProgress; // Убираем окно прогресса с экрана
  // ------------------------------------------------------

  sHtml := HmsUtf8Decode(sHtml);         // Перекодируем текст из UTF-8
  sHtml := HmsRemoveLinebreaks(sHtml);   // Удаляем переносы строк

  // Создаём объект для поиска блоков текста по регулярному выражению,
  // в которых есть информация: ссылка, наименование, ссылка на картинку и проч.
  // Обычно, определяем начало и конец блока и вставляем их вместо <section> и </section>
  RegEx := TRegExpr.Create('<section>(.*?)</section>', PCRE_SINGLELINE);
  
  Try
    // Организуем цикл поиска блоков текста в gsHtml
    If RegEx.Search(sHtml) Then Repeat   // Если Search(...) вернёт True (найдёт) - выполним цикл Repeat
      sLink:=""; sName:=""; sImg:=""; sYear:=""; // Очищаем значения после последнего цикла

      // Получаем данные о видео
      HmsRegExMatch('<a[^>]+href=[''"](.*?)[''"]' , RegEx.Match, sLink); // Ссылка
      HmsRegExMatch('(<h\d.*?</h\d>)'             , RegEx.Match, sName); // Наименование
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

  Finally
    RegEx.Free; // Освобождаем созданный объект из памяти

  End;

  HmsLogMessage(1, mpTitle+': создано элементов - '+Str(gnTotalItems)); 
End;
  
///////////////////////////////////////////////////////////////////////////////
//                     Г Л А В Н А Я    П Р О Ц Е Д У Р А                    //
begin
  FolderItem.DeleteChildItems;           // Очищаем существующие ссылки
  LoadPagesAndCreateLinks();             // Вызов процедуры загрузки страниц и создания ссылок
end.