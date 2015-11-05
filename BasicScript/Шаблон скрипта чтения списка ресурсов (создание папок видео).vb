' Глобальные переменные
DIM gsUrlBase    = "http://site.com" ' База ссылки, для создания полных ссылок из относительных
DIM gnTotalItems = 0                 ' Количество созданных элементов
  
'//////////////////////////////////////////////////////////////////////////////

' ---- Создание папки --------------------------------------------------------
FUNCTION CreateFolder(sName, sLink, sImg)
  Result = FolderItem.AddFolder(sLink)    ' Создаём папку с указанной ссылкой
  Result.Properties[mpiTitle    ] = sName ' Присваиваем наименование
  Result.Properties[mpiThumbnail] = sImg  ' Картинка
  Inc(gnTotalItems) ' Увеличиваем счетчик созданных элементов
END FUNCTION

' ---- Загрузка страниц и создание ссылок ------------------------------------
SUB LoadPagesAndCreateLinks
  DIM sHtml, sData, sName, sLink, sImg, sYear, sVal 
  DIM i, nPages, nSec, RegEx
  DIM Item ' Объект элемента базы данных программы 
  
  sHtml  = ""
  nPages = 2  ' Количество загружаемых страниц

  ' Загружаем первые сколько-то страниц
  FOR i = 1 TO nPages
    HmsSetProgress(Trunc(i*100/nPages))                   ' Устанавливаем позицию прогресса загрузки 
    sName = Format("%s: Страница %d из %d", [mpTitle, i, nPages]) ' Формируем заголовок прогресса
    HmsShowProgress(sName)                                ' Показываем окно прогресса выполнения
    sLink = mpFilePath+"/page/"+IntToStr(i)+"/"           ' Формируем ссылку для загрузки, включающую номер страницы
    sHtml = sHtml + HmsUtf8Decode(HmsDownloadUrl(sLink))  ' Загружаем страницу
    IF HmsCancelPressed THEN BREAK                        ' Если в окне прогресса нажали "Отмена" - прерываем цикл
  NEXT
  HmsHideProgress                     ' Убираем окно прогресса с экрана

  sHtml = HmsUtf8Decode(sHtml)        ' Перекодируем текст из UTF-8
  sHtml = HmsRemoveLinebreaks(sHtml)  ' Удаляем переносы строк

  ' Создаём объект для поиска по регулярному выражению
  RegEx = TRegExpr.Create("<section>(.*?)<section>", PCRE_SINGLELINE)
  
  ' Организовываем цикл
  IF RegEx.Search(sHtml) THEN
    DO
    sLink = ""
    sName = ""
    sImg  = ""
    sYear = "" ' Очищаем значения после последнего цикла
  
    ' Получаем данные о видео
    HmsRegExMatch("<a[^>]+href=['""](.*?)['""]" , RegEx.Match, sLink) ' Ссылка
    HmsRegExMatch("alt=""(.*?)"""               , RegEx.Match, sName) ' Наименование
    HmsRegExMatch("<img[^>]+src=['""](.*?)['""]", RegEx.Match, sImg ) ' Картинка
    HmsRegExMatch("year.*?(\d{4})"              , RegEx.Match, sYear) ' Год
    
    sName = HmsHtmlToText(sName)             ' Избавляемся от html тегов в названии 
    sLink = HmsExpandLink(sLink, gsUrlBase)  ' Делаем из относительных ссылок абсолютные
    sImg  = HmsExpandLink(sImg , gsUrlBase)

    ' Если в названии нет года, добавляем год выхода 
    IF (sYear<>"") AND (Pos(sYear, sName) < 1) THEN sName = sName + " ("+sYear+")"
    
    ' Создаём папку видео
    CreateFolder(sName, sLink, sImg)

    Inc(gnTotalItems)      ' Увеличиваем счетчик созданных элементов
    
    LOOP WHILE RegEx.SearchAgain  ' Повторять цикл пока SearchAgain возвращает True 
  END IF

  RegEx.Free ' Освобождаем созданный объект из памяти

  HmsLogMessage(1, mpTitle+": создано элементов - "+Str(gnTotalItems))
END SUB
  
'//////////////////////////////////////////////////////////////////////////////
'//                    Г Л А В Н А Я    П Р О Ц Е Д У Р А                    //
FolderItem.DeleteChildItems           ' Очищаем существующие ссылки
LoadPagesAndCreateLinks()             ' Вызов процедуры загрузки страниц и создания ссылок