///////////////////////////////////////////////////////////////////////////////
// --- Создание папки видео ---------------------------------------------------
Function CreateFolder(aName, aLink: String; aImg: String=''): THmsScriptMediaItem;
Begin
  Result := PodcastItem.AddFolder(sLink); // Создаём папку с указанной ссылкой
  Result[mpiTitle     ] := sName; // Присваиваем наименование
  Result[mpiThumbnail ] := sImg;  // Картинка
  Result[mpiCreateDate] := DateTimeToStr(IncTime(gStart, 0, 0, -gnTotalItems, 0));
  Inc(gnTotalItems);              // Увеличиваем счетчик созданных элементов
End;
