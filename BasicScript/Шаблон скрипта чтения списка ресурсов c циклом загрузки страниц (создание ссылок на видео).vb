' Глобальные переменные
DIM gsUrlBase    = "http://site.com" ' База ссылки, для создания полных ссылок из относительных
DIM gnTotalItems = 0                 ' Количество созданных элементов
  
'//////////////////////////////////////////////////////////////////////////////

' ---- Загрузка страниц и создание ссылок ------------------------------------
SUB LoadPagesAndCreateLinks()
  DIM sHtml, sData, sName, sLink, sImg, sYear, sTime, sVal
  DIM i, nPages, nSec 
  DIM RegEx
  DIM Item ' Объект элемента базы данных программы 
  
  sHtml  = ""
  nPages = 2 ' Количество загружаемых страниц

  ' Загружаем первые сколько-то страниц (указано в nPages)
  ' В зависимости от того, как именно на конкретном сайте выглядят ссылки последующих
  ' страниц, возможно потребуедтся изменить формирование ссылки '/page/'+...
  FOR i = 1 TO nPages
    HmsSetProgress(Trunc(i*100/nPages)) ' Устанавливаем позицию прогресса загрузки 
    sName = Format("%s: Страница %d из %d", [mpTitle, i, nPages]) ' Формируем заголовок прогресса
    HmsShowProgress(sName)                                ' Показываем окно прогресса выполнения
    sLink = mpFilePath;
    IF i > 1 THEN sLink = sLink+"/page/"+IntToStr(i)+"/"; ' Формируем ссылку для загрузки, включающую номер страницы    
    sHtml = sHtml + HmsUtf8Decode(HmsDownloadUrl(sLink))  ' Загружаем страницу
    IF HmsCancelPressed THEN BREAK                        ' Если в окне прогресса нажали "Отмена" - прерываем цикл
  NEXT
  HmsHideProgress                      ' Убираем окно прогресса с экрана

  sHtml = HmsUtf8Decode(sHtml)         ' Перекодируем текст из UTF-8
  sHtml = HmsRemoveLinebreaks(sHtml)   ' Удаляем переносы строк

  ' Создаём объект для поиска блоков текста по регулярному выражению,
  ' в которых есть информация: ссылка, наименование, ссылка на картинку и проч.
  ' Обычно, определяем начало и конец блока и вставляем их вместо <section> и </section>
  RegEx = TRegExpr.Create("<section>(.*?)</section>", PCRE_SINGLELINE)
  
  ' Организуем цикл поиска блоков текста в gsHtml
  IF RegEx.Search(sHtml) THEN
    DO
    sLink = ""
    sName = "" 
    sImg  = "" 
    sYear = "" 
    sTime = "" ' Очищаем значения после последнего цикла
  
    ' Получаем данные о видео
    HmsRegExMatch("<a[^>]+href=['""](.*?)['""]" , RegEx.Match, sLink) ' Ссылка
    HmsRegExMatch("alt=""(.*?)"""               , RegEx.Match, sName) ' Наименование
    HmsRegExMatch("<img[^>]+src=['""](.*?)['""]", RegEx.Match, sImg ) ' Картинка
    HmsRegExMatch("year.*?(\d{4})"              , RegEx.Match, sYear) ' Год
    HmsRegExMatch("duration.*?>(.*?)<"          , RegEx.Match, sTime) ' Длительность
    
    sName = HmsHtmlToText(sName)            ' Избавляемся от html тегов в названии 
    sLink = HmsExpandLink(sLink, gsUrlBase) ' Делаем из относительных ссылок абсолютные
    sImg  = HmsExpandLink(sImg , gsUrlBase)

    ' Если в названии нет года, добавляем год выхода 
    IF (sYear<>"") AND (Pos(sYear, sName) < 1) THEN sName = sName + " ("+sYear+")"
    
    ' Вычисляем длительность в секундах из того формата, который на сайте (m:ss)
    nSec = 0
    IF HmsRegExMatch("(\d+):", sTime, sVal) THEN nSec = nSec + StrToInt(sVal) * 60 ' Минуты 
    IF HmsRegExMatch(":(\d+)", sTime, sVal) THEN nSec = nSec + StrToInt(sVal)      ' Секунды 
    IF nSec = 0 THEN nSec = 6000 ' Если длительность нет - ставим по-умолчанию 01:40:00 (100 мин)
    
    ' Создаём элемент медиа-ссылки
    Item = HmsCreateMediaItem(sLink, FolderItem.ItemID) ' Создаём элемент подкаста
    Item.Properties[mpiTitle     ] = sName ' Наименование 
    Item.Properties[mpiThumbnail ] = sImg  ' Картинка 
    Item.Properties[mpiYear      ] = sYear ' Год 
    Item.Properties[mpiTimeLength] = nSec  ' Длительность 

    Inc(gnTotalItems)         ' Увеличиваем счетчик созданных элементов
    
    LOOP WHILE RegEx.SearchAgain ' Повторять цикл пока SearchAgain возвращает True 
  END IF
  RegEx.Free ' Освобождаем созданный объект из памяти

  HmsLogMessage(1, mpTitle+": создано элементов - "+Str(gnTotalItems))
END SUB
  
'//////////////////////////////////////////////////////////////////////////////
'//                    Г Л А В Н А Я    П Р О Ц Е Д У Р А                    //
FolderItem.DeleteChildItems ' Очищаем существующие ссылки
LoadPagesAndCreateLinks()   ' Вызов процедуры загрузки страниц и создания ссылок